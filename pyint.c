/******************** MODIFICATIONS to pyint.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/06/14 TLi  bug fix in function guards_black_flight()
 **
 ** 2007/12/27 TLi  bug fix in function stalemate_immobilise_black()
 **
 **************************** End of List ******************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "py.h"
#include "pyproc.h"
#include "pyhash.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "stipulation/help_play/branch.h"
#include "pybrafrk.h"
#include "pyproof.h"
#include "pypipe.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/filter.h"
#include "optimisations/intelligent/proof.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/maxsolutions/maxsolutions.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/legal_move_finder.h"
#include "platform/maxtime.h"
#include "trace.h"

typedef unsigned int index_type;

#define piece_usageENUMERATORS \
    ENUMERATOR(piece_is_unused), \
    ENUMERATOR(piece_pins), \
    ENUMERATOR(piece_is_fixed_to_diagram_square), \
    ENUMERATOR(piece_intercepts), \
    ENUMERATOR(piece_blocks), \
    ENUMERATOR(piece_guards), \
    ENUMERATOR(piece_gives_check), \
    ENUMERATOR(piece_is_missing), \
    ENUMERATOR(piece_is_king)

#define ENUMERATORS piece_usageENUMERATORS
#define ENUMERATION_TYPENAME piece_usage
#define ENUMERATION_DECLARE
#include "pyenum.h"

#define ENUMERATORS piece_usageENUMERATORS
#define ENUMERATION_TYPENAME piece_usage
#define ENUMERATION_MAKESTRINGS
#include "pyenum.h"


typedef struct
{
    square square;
    Flags flags;
    piece type;
    piece_usage usage;
} PIECE;

static goal_type goal_to_be_reached;

static unsigned int MaxPiece[nr_sides];

static PIECE white[nr_squares_on_board];
static PIECE black[nr_squares_on_board];
static square save_ep_1;
static square save_ep2_1;
static unsigned int moves_to_white_prom[nr_squares_on_board];
static square const *where_to_start_placing_unused_black_pieces;
static unsigned int index_of_designated_piece_delivering_check;
static boolean is_initial_check_uninterceptable;
enum { index_of_king = 0 };

static PIECE target_position[MaxPieceId+1];

slice_index current_start_slice = no_slice;

boolean solutions_found;

#define SetPiece(P, SQ, SP) {e[SQ]= P; spec[SQ]= SP;}

static boolean testcastling;

static unsigned int MovesRequired[nr_sides][maxply+1];
static unsigned int CapturesLeft[maxply+1];


static boolean rider_guards(square to_be_guarded, square guarding_from, int dir)
{
  boolean result = false;

  if (dir!=0)
  {
    square tmp = guarding_from;
    do
    {
      tmp += dir;
      if (tmp==to_be_guarded)
      {
        result = true;
        break;
      }
    } while (e[tmp]==vide);
  }

  return result;
}

static boolean guards(square to_be_guarded, piece guarding, square guarding_from)
{
  boolean result;
  int const diff = to_be_guarded-guarding_from;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_guarded);
  TracePiece(guarding);
  TraceSquare(guarding_from);
  TraceFunctionParamListEnd();

  switch (abs(guarding))
  {
    case Pawn:
      result = (guarding_from>=square_a2
                && (diff==+dir_up+dir_left || diff==+dir_up+dir_right));
      break;

    case Knight:
      result = CheckDirKnight[diff]!=0;
      break;

    case Bishop:
      result = rider_guards(to_be_guarded,guarding_from,CheckDirBishop[diff]);
      break;

    case Rook:
      result = rider_guards(to_be_guarded,guarding_from,CheckDirRook[diff]);
      break;

    case Queen:
      result = (rider_guards(to_be_guarded,guarding_from,CheckDirBishop[diff])
                || rider_guards(to_be_guarded,guarding_from,CheckDirRook[diff]));
      break;

    case King:
      result = move_diff_code[abs(diff)]<3;
      break;

    default:
      assert(0);
      result = false;
      break;
  }


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_kings_too_close(square white_king_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_diff_code[abs(white_king_square-king_square[Black])]<3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean uninterceptably_attacks_king(Side side, square from, piece p)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (king_square[side]==initsquare)
    result = false;
  else
  {
    int const dir = king_square[side]-from;
    switch(p)
    {
      case db:
      case dn:
        result = CheckDirQueen[dir]==dir;
        break;

      case tb:
      case tn:
        result = CheckDirRook[dir]==dir;
        break;

      case fb:
      case fn:
        result = CheckDirBishop[dir]==dir;
        break;

      case cb:
      case cn:
        result = CheckDirKnight[dir]!=0;
        break;

      case pb:
        result = dir==dir_up+dir_right || dir==dir_up+dir_left;
        break;

      case pn:
        result = dir==dir_down+dir_right || dir==dir_down+dir_left;
        break;

      default:
        assert(0);
        result = false;
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_white_king_interceptably_attacked(void)
{
  return ((*checkfunctions[Bishop])(king_square[White],fn,eval_ortho)
          || (*checkfunctions[Rook])(king_square[White],tn,eval_ortho)
          || (*checkfunctions[Queen])(king_square[White],dn,eval_ortho));
}

static boolean is_white_king_uninterceptably_attacked_by_non_king(square s)
{
  return ((*checkfunctions[Pawn])(s,pn,eval_ortho)
          || (*checkfunctions[Knight])(s,cn,eval_ortho)
          || (*checkfunctions[Fers])(s,fn,eval_ortho)
          || (*checkfunctions[Wesir])(s,tn,eval_ortho)
          || (*checkfunctions[ErlKing])(s,dn,eval_ortho));
}

static boolean guards_black_flight(piece as_piece, square from)
{
  int i;
  boolean result;

  TraceFunctionEntry(__func__);
  TracePiece(as_piece);
  TraceSquare(from);
  TraceSquare(king_square[Black]);
  TraceFunctionParamListEnd();

  /* if we initialised result to false, we'd miss some solutions with
   * double-check. Cf. 4_47_to_4_49.reg
   */
  result = guards(king_square[Black],as_piece,from);;

  e[king_square[Black]]= vide;

  for (i = 8; i!=0 && !result; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && guards(king_square[Black]+vec[i],as_piece,from))
      result = true;

  e[king_square[Black]]= roin;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_from_to_no_check(piece from_piece,
                                                       square from_square,
                                                       piece to_piece,
                                                       square to_square);

static unsigned int count_nr_of_moves_from_to_pawn_promotion(square from_square,
                                                             piece to_piece,
                                                             square to_square)
{
  unsigned int result = maxply+1;
  square const start = to_piece<vide ? square_a1 : square_a8;
  piece const pawn = to_piece<vide ? pn : pb;
  square v_sq;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  for (v_sq = start; v_sq<start+nr_files_on_board; ++v_sq)
  {
    unsigned int const curmoves = (count_nr_of_moves_from_to_no_check(pawn,from_square,
                                                                      pawn,v_sq)
                                   + count_nr_of_moves_from_to_no_check(to_piece,v_sq,
                                                                        to_piece,to_square));
    if (curmoves<result)
      result = curmoves;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_from_to_pawn_no_promotion(piece pawn,
                                                                square from_square,
                                                                square to_square)
{
  unsigned int result;
  int const diffcol = abs(from_square%onerow - to_square%onerow);
  int const diffrow = from_square/onerow - to_square/onerow;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (pawn<vide)
  {
    /* black pawn */
    if (diffrow<diffcol)
      /* if diffrow<=0 then this test is true, since diffcol is always
       * non-negative
       */
      result = maxply+1;

    else if (from_square>=square_a7 && diffrow-2 >= diffcol)
      /* double step */
      result = diffrow-1;

    else
      result = diffrow;
  }
  else
  {
    /* white pawn */
    if (-diffrow<diffcol)
      result = maxply+1;

    else  if (from_square<=square_h2 && -diffrow-2 >= diffcol)
      /* double step */
      result = -diffrow-1;

    else
      result = -diffrow;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_from_to_king_no_castling(square from, square to)
{
  unsigned int const diffcol = abs(from%onerow - to%onerow);
  unsigned int const diffrow = abs(from/onerow - to/onerow);

  return diffcol>diffrow ? diffcol : diffrow;
}

static unsigned int count_nr_of_moves_from_to_king(piece piece,
                                                   square from_square,
                                                   square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = count_nr_of_moves_from_to_king_no_castling(from_square,to_square);

  if (testcastling)
  {
    if (piece==roib)
    {
      if (from_square==square_e1)
      {
        if (TSTCASTLINGFLAGMASK(nbply,White,ra_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_c1,to_square);
          if (withcast<result)
            result = withcast;
        }
        if (TSTCASTLINGFLAGMASK(nbply,White,rh_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_g1,to_square);
          if (withcast<result)
            result = withcast;
        }
      }
    }
    else {
      if (from_square==square_e8)
      {
        if (TSTCASTLINGFLAGMASK(nbply,Black,ra_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_c8,to_square);
          if (withcast<result)
            result = withcast;
        }
        if (TSTCASTLINGFLAGMASK(nbply,Black,rh_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_g8,to_square);
          if (withcast<result)
            result = withcast;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_from_to_no_check(piece from_piece,
                                                       square from_square,
                                                       piece to_piece,
                                                       square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(from_piece);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square && from_piece==to_piece)
    result = 0;
  else
  {
    switch (abs(from_piece))
    {
      case Knight:
        result = ProofKnightMoves[abs(from_square-to_square)];
        break;

      case Rook:
        result = CheckDirRook[from_square-to_square]==0 ? 2 : 1;
        break;

      case Queen:
        result = ((CheckDirRook[from_square-to_square]==0
                   && CheckDirBishop[from_square-to_square]==0)
                  ? 2
                  : 1);
        break;

      case Bishop:
        if (SquareCol(from_square)==SquareCol(to_square))
          result = CheckDirBishop[from_square-to_square]==0 ? 2 : 1;
        else
          result = maxply+1;
        break;

      case King:
        result = count_nr_of_moves_from_to_king(from_piece,from_square,
                                                to_square);
        break;

      case Pawn:
        if (from_piece==to_piece)
          result = count_nr_of_moves_from_to_pawn_no_promotion(from_piece,
                                                               from_square,
                                                               to_square);
        else
          result = count_nr_of_moves_from_to_pawn_promotion(from_square,
                                                            to_piece,to_square);
        break;

      default:
        assert(0);
        result = UINT_MAX;
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_from_to_checking(piece from_piece,
                                                       square from_square,
                                                       piece to_piece,
                                                       square to_square)
{
  if (from_square==to_square && from_piece==to_piece)
  {
    if (from_piece==pb)
      return maxply+1;

    else if (from_piece==cb)
      return 2;

    /* it's a rider */
    else if (move_diff_code[abs(king_square[Black]-to_square)]<3)
      return 2;
  }

  return count_nr_of_moves_from_to_no_check(from_piece,from_square,to_piece,to_square);
}

static boolean mate_isGoalReachable(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (pprise[nbply]
      && target_position[GetPieceId(pprispec[nbply])].square!=initsquare)
    /* a piece has been captured that participates in the mate */
    result = false;

  else
  {
    TraceValue("%u",MovesLeft[White]);
    TraceValue("%u\n",MovesLeft[Black]);

    if (nbply==2
        || (testcastling && castling_flag[nbply]!=castling_flag[nbply-1]))
    {
      square const *bnp;
      MovesRequired[White][nbply] = 0;
      MovesRequired[Black][nbply] = 0;
      for (bnp = boardnum; *bnp!=initsquare; bnp++)
      {
        square const from_square = *bnp;
        piece const from_piece = e[from_square];
        if (from_piece!=vide && from_piece!=obs)
        {
          PieceIdType const id = GetPieceId(spec[from_square]);
          if (target_position[id].square!=initsquare)
          {
            Side const from_side = from_piece>vide ? White : Black;
            if (id==GetPieceId(white[index_of_designated_piece_delivering_check].flags)
                && MovesLeft[White]>0)
            {
              square const save_king_square = king_square[Black];
              PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
              king_square[Black] = target_position[id_king].square;
              MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_checking(from_piece,
                                                                                    from_square,
                                                                                    target_position[id].type,
                                                                                    target_position[id].square);
              king_square[Black] = save_king_square;
            }
            else
              MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_no_check(from_piece,
                                                                                    from_square,
                                                                                    target_position[id].type,
                                                                                    target_position[id].square);
          }
        }
      }
    }
    else
    {
      PieceIdType const id = GetPieceId(jouespec[nbply]);
      MovesRequired[White][nbply] = MovesRequired[White][nbply-1];
      MovesRequired[Black][nbply] = MovesRequired[Black][nbply-1];

      if (target_position[id].square!=initsquare)
      {
        unsigned int time_before;
        unsigned int time_now;
        if (id==GetPieceId(white[index_of_designated_piece_delivering_check].flags))
        {
          square const save_king_square = king_square[Black];
          PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
          king_square[Black] = target_position[id_king].square;
          time_before = count_nr_of_moves_from_to_checking(pjoue[nbply],
                                                           move_generation_stack[nbcou].departure,
                                                           target_position[id].type,
                                                           target_position[id].square);
          king_square[Black] = save_king_square;
        }
        else
          time_before = count_nr_of_moves_from_to_no_check(pjoue[nbply],
                                                           move_generation_stack[nbcou].departure,
                                                           target_position[id].type,
                                                           target_position[id].square);

        if (id==GetPieceId(white[index_of_designated_piece_delivering_check].flags)
            && MovesLeft[White]>0)
        {
          square const save_king_square = king_square[Black];
          PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
          king_square[Black] = target_position[id_king].square;
          time_now = count_nr_of_moves_from_to_checking(e[move_generation_stack[nbcou].arrival],
                                                        move_generation_stack[nbcou].arrival,
                                                        target_position[id].type,
                                                        target_position[id].square);
          king_square[Black] = save_king_square;
        }
        else
          time_now = count_nr_of_moves_from_to_no_check(e[move_generation_stack[nbcou].arrival],
                                                        move_generation_stack[nbcou].arrival,
                                                        target_position[id].type,
                                                        target_position[id].square);

        assert(MovesRequired[trait[nbply]][nbply]+time_now>=time_before);
        MovesRequired[trait[nbply]][nbply] += time_now-time_before;
      }
    }

    result = (MovesRequired[White][nbply]<=MovesLeft[White]
              && MovesRequired[Black][nbply]<=MovesLeft[Black]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean stalemate_are_there_sufficient_moves_left_for_required_captures(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  CapturesLeft[nbply] = CapturesLeft[nbply-1];
  if (pprise[nbply]<vide)
    --CapturesLeft[nbply];

  TraceValue("%u\n",CapturesLeft[nbply]);
  result = MovesLeft[White]>=CapturesLeft[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean stalemate_isGoalReachable(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (pprise[nbply]
      && target_position[GetPieceId(pprispec[nbply])].square!=initsquare)
    /* a piece has been captured that participates in the mate */
    result = false;

  else
  {
    TraceValue("%u",MovesLeft[White]);
    TraceValue("%u\n",MovesLeft[Black]);

    if (nbply==2
        || (testcastling && castling_flag[nbply]!=castling_flag[nbply-1]))
    {
      square const *bnp;
      MovesRequired[White][nbply] = 0;
      MovesRequired[Black][nbply] = 0;
      for (bnp = boardnum; *bnp!=initsquare; bnp++)
      {
        square const from_square = *bnp;
        piece const from_piece = e[from_square];
        if (from_piece!=vide && from_piece!=obs)
        {
          PieceIdType const id = GetPieceId(spec[from_square]);
          if (target_position[id].square!=initsquare)
          {
            Side const from_side = from_piece>vide ? White : Black;
            MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_no_check(from_piece,
                                                                                  from_square,
                                                                                  target_position[id].type,
                                                                                  target_position[id].square);
          }
        }
      }
    }
    else
    {
      PieceIdType const id = GetPieceId(jouespec[nbply]);
      MovesRequired[White][nbply] = MovesRequired[White][nbply-1];
      MovesRequired[Black][nbply] = MovesRequired[Black][nbply-1];

      if (target_position[id].square!=initsquare)
      {
        unsigned int const time_before = count_nr_of_moves_from_to_no_check(pjoue[nbply],
                                                                            move_generation_stack[nbcou].departure,
                                                                            target_position[id].type,
                                                                            target_position[id].square);

        unsigned int const time_now = count_nr_of_moves_from_to_no_check(e[move_generation_stack[nbcou].arrival],
                                                                         move_generation_stack[nbcou].arrival,
                                                                         target_position[id].type,
                                                                         target_position[id].square);

        TracePiece(pjoue[nbply]);
        TraceSquare(move_generation_stack[nbcou].departure);
        TracePiece(e[move_generation_stack[nbcou].arrival]);
        TraceSquare(move_generation_stack[nbcou].arrival);
        TracePiece(target_position[id].type);
        TraceSquare(target_position[id].square);
        TraceValue("%u",time_before);
        TraceValue("%u\n",time_now);

        assert(MovesRequired[trait[nbply]][nbply]+time_now>=time_before);
        MovesRequired[trait[nbply]][nbply] += time_now-time_before;
      }
    }

    TraceValue("%u",MovesRequired[White][nbply]);
    TraceValue("%u\n",MovesRequired[Black][nbply]);

    result = (MovesRequired[White][nbply]<=MovesLeft[White]
              && MovesRequired[Black][nbply]<=MovesLeft[Black]
              && stalemate_are_there_sufficient_moves_left_for_required_captures());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* declarations */
static void stalemate_black_block(unsigned int nr_remaining_black_moves,
                                  unsigned int nr_remaining_white_moves,
                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                  stip_length_type n,
                                  square to_be_blocked);
static void stalemate_place_an_unused_black_piece(unsigned int nr_remaining_black_moves,
                                                  unsigned int nr_remaining_white_moves,
                                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                                  stip_length_type n);
static void stalemate_immobilise_black(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n);
static void stalemate_intercept_checks(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       unsigned int nr_checks_to_opponent,
                                       Side side);
static void stalemate_continue_intercepting_checks(unsigned int nr_remaining_black_moves,
                                                   unsigned int nr_remaining_white_moves,
                                                   unsigned int max_nr_allowed_captures_by_black_pieces,
                                                   unsigned int max_nr_allowed_captures_by_white_pieces,
                                                   stip_length_type n,
                                                   int const check_directions[8],
                                                   unsigned int nr_of_check_directions,
                                                   unsigned int const nr_checks_to_opponent,
                                                   Side side);
static unsigned int count_nr_black_moves_to_square(square to_be_blocked,
                                                   unsigned int nr_remaining_black_moves);
static void stalemate_fix_white_king_on_diagram_square(unsigned int nr_remaining_black_moves,
                                                       unsigned int nr_remaining_white_moves,
                                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                                       stip_length_type n);
static void mate_neutralise_guarding_pieces(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n);
static void mate_place_any_black_piece_on(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          square placed_on);
static void mate_place_any_white_piece_on(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          square placed_on);
static void stalemate_test_target_position(unsigned int nr_remaining_black_moves,
                                           unsigned int nr_remaining_white_moves,
                                           unsigned int max_nr_allowed_captures_by_black_pieces,
                                           unsigned int max_nr_allowed_captures_by_white_pieces,
                                           stip_length_type n);
static void mate_intercept_checks(unsigned int nr_remaining_black_moves,
                                  unsigned int nr_remaining_white_moves,
                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                  stip_length_type n);

/*#define DETAILS*/
#if defined(DETAILS)
static void trace_target_position(PIECE const position[MaxPieceId+1],
                                  unsigned int nr_required_captures)
{
  unsigned int moves_per_side[nr_sides] = { 0, 0 };
  square const *bnp;

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
    if (e[*bnp]!=vide)
    {
      Flags const sp = spec[*bnp];
      PieceIdType const id = GetPieceId(sp);
      PIECE const * const target = &position[id];
      if (target->square!=vide)
      {
        unsigned int const time = count_nr_of_moves_from_to_no_check(e[*bnp],
                                                                     *bnp,
                                                                     target->type,
                                                                     target->square);
        moves_per_side[e[*bnp]<vide ? Black : White] += time;
        TracePiece(e[*bnp]);
        TraceSquare(*bnp);
        TracePiece(target->type);
        TraceSquare(target->square);
        TraceEnumerator(piece_usage,target->usage,"");
        TraceValue("%u\n",time);
      }
    }

  TraceValue("%u",nr_required_captures);
  TraceValue("%u",moves_per_side[Black]);
  TraceValue("%u\n",moves_per_side[White]);
}

static piece_usage find_piece_usage(PieceIdType id)
{
  piece_usage result = piece_is_unused;

  unsigned int i;
  for (i = 0; i<MaxPiece[White]; ++i)
    if (id==GetPieceId(white[i].flags))
    {
      result = white[i].usage;
      break;
    }
  for (i = 0; i<MaxPiece[Black]; ++i)
    if (id==GetPieceId(black[i].flags))
    {
      result = black[i].usage;
      break;
    }

  assert(result!=piece_is_unused);

  return result;
}
#endif

static void solve_target_position(stip_length_type n)
{
  square const save_king_square[nr_sides] = { king_square[White],
                                              king_square[Black] };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      target_position[id].square = initsquare;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; bnp++)
    {
      piece const type = e[*bnp];
      if (type!=vide && type!=obs)
      {
        Flags const flags = spec[*bnp];
        PieceIdType const id = GetPieceId(flags);
        target_position[id].type = type;
        target_position[id].flags = flags;
        target_position[id].square = *bnp;
#if defined(DETAILS)
        target_position[id].usage = find_piece_usage(id);
#endif
      }
    }
  }

  /* solve the problem */
  ResetPosition();

  castling_supported = true;

  ep[1] = save_ep_1;
  ep2[1] = save_ep2_1;

#if defined(DETAILS)
  TraceText("target position:\n");
  trace_target_position(target_position,CapturesLeft[1]);
#endif

  reset_nr_solutions_per_target_position();

  closehash();
  inithash(current_start_slice);

  if (help(slices[current_start_slice].u.pipe.next,n)<=n)
    solutions_found = true;

  /* reset the old mating position */
  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; bnp++)
    {
      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      if (target_position[id].square != initsquare)
      {
        e[target_position[id].square] = target_position[id].type;
        spec[target_position[id].square] = target_position[id].flags;
      }
  }

  {
    int p;
    for (p = King; p<=Bishop; ++p)
    {
      nbpiece[-p] = 2;
      nbpiece[p] = 2;
    }
  }

  king_square[White] = save_king_square[White];
  king_square[Black] = save_king_square[Black];

  ep[1] = initsquare;
  ep2[1] = initsquare;

  castling_supported = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_deal_with_unused_pieces(unsigned int nr_remaining_black_moves,
                                              unsigned int nr_remaining_white_moves,
                                              unsigned int max_nr_allowed_captures_by_black_pieces,
                                              unsigned int max_nr_allowed_captures_by_white_pieces,
                                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!hasMaxtimeElapsed())
  {
    if (white[index_of_king].usage==piece_is_unused
        && white[index_of_king].square!=square_e1
        && nr_remaining_white_moves==0)
    {
      if (e[white[index_of_king].square]==vide)
        stalemate_fix_white_king_on_diagram_square(nr_remaining_black_moves,
                                                   nr_remaining_white_moves,
                                                   max_nr_allowed_captures_by_black_pieces,
                                                   max_nr_allowed_captures_by_white_pieces,
                                                   n);
    }
    else
    {
      unsigned int unused = 0;

      {
        unsigned int i;
        for (i = 1; i<MaxPiece[Black]; ++i)
          if (black[i].usage==piece_is_unused)
            ++unused;
      }

      TraceValue("%u\n",piece_is_unused);
      TraceValue("%u\n",MovesLeft[White]);
      if (unused>0)
        stalemate_place_an_unused_black_piece(nr_remaining_black_moves,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n);

      if (unused<=MovesLeft[White])
      {
        CapturesLeft[1] = unused;
        solve_target_position(n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_place_an_unused_black_piece(unsigned int nr_remaining_black_moves,
                                                  unsigned int nr_remaining_white_moves,
                                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                                  stip_length_type n)
{
  square const *bnp;
  square const * const save_start = where_to_start_placing_unused_black_pieces;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = where_to_start_placing_unused_black_pieces; *bnp; ++bnp)
    if (e[*bnp]==vide)
    {
      where_to_start_placing_unused_black_pieces = bnp;
      stalemate_black_block(nr_remaining_black_moves,
                            nr_remaining_white_moves,
                            max_nr_allowed_captures_by_black_pieces,
                            max_nr_allowed_captures_by_white_pieces,
                            n,
                            *bnp);
    }

  where_to_start_placing_unused_black_pieces = save_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean mate_exists_redundant_white_piece(void)
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* check for redundant white pieces */
  for (bnp = boardnum; !result && *bnp!=initsquare; bnp++)
  {
    square const sq = *bnp;
    if (sq!=king_square[White] && e[sq]>obs)
    {
      piece const p = e[sq];
      Flags const sp = spec[sq];

      /* remove piece */
      e[sq] = vide;
      spec[sq] = EmptySpec;

      result = (echecc(nbply,Black)
                && slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution);

      /* restore piece */
      e[sq] = p;
      spec[sq] = sp;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void mate_test_target_position(unsigned int nr_remaining_black_moves,
                                      unsigned int nr_remaining_white_moves,
                                      unsigned int max_nr_allowed_captures_by_black_pieces,
                                      unsigned int max_nr_allowed_captures_by_white_pieces,
                                      stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,White));
  if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
  {
    /* avoid duplicate test of the same target position (modulo redundant pieces
     * and unused white king) */
    if (!mate_exists_redundant_white_piece()
        && !(white[index_of_king].usage==piece_is_unused
             && white[index_of_king].square!=square_e1
             && nr_remaining_white_moves==0))
      solve_target_position(n);
  }
  else
    mate_neutralise_guarding_pieces(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_continue_intercepting_checks(unsigned int nr_remaining_black_moves,
                                              unsigned int nr_remaining_white_moves,
                                              unsigned int max_nr_allowed_captures_by_black_pieces,
                                              unsigned int max_nr_allowed_captures_by_white_pieces,
                                              stip_length_type n,
                                              int const check_directions[8],
                                              unsigned int nr_of_check_directions);

static void mate_intercept_check_with_unpromoted_white_pawn(unsigned int nr_remaining_black_moves,
                                                            unsigned int nr_remaining_white_moves,
                                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                                            stip_length_type n,
                                                            unsigned int placed_index,
                                                            square placed_on,
                                                            int const check_directions[8],
                                                            unsigned int nr_of_check_directions)
{
  square const placed_from = white[placed_index].square;
  unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (diffcol<=max_nr_allowed_captures_by_white_pieces
      && !(is_initial_check_uninterceptable
           && uninterceptably_attacks_king(Black,placed_on,pb)))
  {
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                          placed_from,
                                                                          placed_on);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(pb,placed_on,white[placed_index].flags);
      mate_continue_intercepting_checks(nr_remaining_black_moves,
                                        nr_remaining_white_moves-time,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces-diffcol,
                                        n,
                                        check_directions,
                                        nr_of_check_directions);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_with_promoted_white_pawn(unsigned int nr_remaining_black_moves,
                                                          unsigned int nr_remaining_white_moves,
                                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                                          stip_length_type n,
                                                          unsigned int placed_index,
                                                          square placed_on,
                                                          int const check_directions[8],
                                                          unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = moves_to_white_prom[placed_index];
    if (placed_on<=square_h7)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++time;

    if (time<=nr_remaining_white_moves)
    {
      square const placed_from = white[placed_index].square;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!(is_initial_check_uninterceptable
              && uninterceptably_attacks_king(Black,placed_on,pp)))
        {
          unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                             pp,
                                                                             placed_on);
          unsigned int diffcol;
          if (pp==fb && SquareCol(placed_on)==SquareCol(placed_from%onerow))
            diffcol= 1;
          else
            diffcol= 0;
          TracePiece(pp);
          TraceValue("%u",diffcol);
          TraceValue("%u\n",time);

          if (diffcol<=max_nr_allowed_captures_by_white_pieces
              && time<=nr_remaining_white_moves)
          {
            SetPiece(pp,placed_on,white[placed_index].flags);
            mate_continue_intercepting_checks(nr_remaining_black_moves,
                                              nr_remaining_white_moves-time,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces-diffcol,
                                              n,
                                              check_directions,
                                              nr_of_check_directions);
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_with_white_officer(unsigned int nr_remaining_black_moves,
                                                    unsigned int nr_remaining_white_moves,
                                                    unsigned int max_nr_allowed_captures_by_black_pieces,
                                                    unsigned int max_nr_allowed_captures_by_white_pieces,
                                                    stip_length_type n,
                                                    unsigned int placed_index,
                                                    piece placed_type, square placed_on,
                                                    int const check_directions[8],
                                                    unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!(is_initial_check_uninterceptable
        && uninterceptably_attacks_king(Black,placed_on,placed_type)))
  {
    square const placed_from = white[placed_index].square;
    unsigned int const time= count_nr_of_moves_from_to_no_check(placed_type,
                                                                placed_from,
                                                                placed_type,
                                                                placed_on);
    if (time<=nr_remaining_white_moves)
    {
      Flags const placed_flags = white[placed_index].flags;
      SetPiece(placed_type,placed_on,placed_flags);
      mate_continue_intercepting_checks(nr_remaining_black_moves,
                                        nr_remaining_white_moves-time,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        check_directions,
                                        nr_of_check_directions);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_white(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       square placed_on,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
    if (white[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = white[placed_index].type;

      white[placed_index].usage = piece_intercepts;

      if (placed_type==pb)
      {
        if (placed_on<=square_h7)
          mate_intercept_check_with_unpromoted_white_pawn(nr_remaining_black_moves,
                                                          nr_remaining_white_moves,
                                                          max_nr_allowed_captures_by_black_pieces,
                                                          max_nr_allowed_captures_by_white_pieces,
                                                          n,
                                                          placed_index,placed_on,
                                                          check_directions,
                                                          nr_of_check_directions);
        mate_intercept_check_with_promoted_white_pawn(nr_remaining_black_moves,
                                                      nr_remaining_white_moves,
                                                      max_nr_allowed_captures_by_black_pieces,
                                                      max_nr_allowed_captures_by_white_pieces,
                                                      n,
                                                      placed_index,placed_on,
                                                      check_directions,
                                                      nr_of_check_directions);
      }
      else
        mate_intercept_check_with_white_officer(nr_remaining_black_moves,
                                                nr_remaining_white_moves,
                                                max_nr_allowed_captures_by_black_pieces,
                                                max_nr_allowed_captures_by_white_pieces,
                                                n,
                                                placed_index,placed_type,placed_on,
                                                check_directions,
                                                nr_of_check_directions);

      white[placed_index].usage = piece_is_unused;
    }

  e[placed_on]= vide;
  spec[placed_on]= EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_with_promoted_black_pawn(unsigned int nr_remaining_black_moves,
                                                          unsigned int nr_remaining_white_moves,
                                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                                          stip_length_type n,
                                                          unsigned int placed_index,
                                                          square placed_on,
                                                          int const check_directions[8],
                                                          unsigned int nr_of_check_directions)
{
  square const placed_from = black[placed_index].square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = (placed_from>=square_a7
                         ? 5
                         : placed_from/onerow - nr_of_slack_rows_below_board);
    assert(time<=5);

    if (placed_on>=square_a2)
      /* square is not on 1st rank -- 1 move necessary to get there */
      ++time;

    if (time<=nr_remaining_black_moves)
    {
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
        if (!guards(king_square[White],pp,placed_on))
        {
          unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                             pp,
                                                                             placed_on);
          unsigned int diffcol = 0;
          if (pp==fn)
          {
            unsigned int const placed_from_file = placed_from%nr_files_on_board;
            square const promotion_square_on_same_file = square_a1+placed_from_file;
            if (SquareCol(placed_on)!=SquareCol(promotion_square_on_same_file))
              diffcol = 1;
          }

          if (diffcol<=max_nr_allowed_captures_by_black_pieces
              && time<=nr_remaining_black_moves)
          {
            SetPiece(pp,placed_on,black[placed_index].flags);
            mate_continue_intercepting_checks(nr_remaining_black_moves-time,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces-diffcol,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n,
                                              check_directions,
                                              nr_of_check_directions);
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_with_unpromoted_black_pawn(unsigned int nr_remaining_black_moves,
                                                            unsigned int nr_remaining_white_moves,
                                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                                            stip_length_type n,
                                                            unsigned int placed_index,
                                                            square placed_on,
                                                            int const check_directions[8],
                                                            unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,placed_on,pn))
  {
    square const placed_from = black[placed_index].square;
    unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);
    if (diffcol<=max_nr_allowed_captures_by_black_pieces)
    {
      unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                            placed_from,
                                                                            placed_on);
      if (time<=nr_remaining_black_moves)
      {
        SetPiece(pn,placed_on,black[placed_index].flags);
        mate_continue_intercepting_checks(nr_remaining_black_moves-time,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces-diffcol,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          check_directions,
                                          nr_of_check_directions);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_with_black_officer(unsigned int nr_remaining_black_moves,
                                                    unsigned int nr_remaining_white_moves,
                                                    unsigned int max_nr_allowed_captures_by_black_pieces,
                                                    unsigned int max_nr_allowed_captures_by_white_pieces,
                                                    stip_length_type n,
                                                    unsigned int placed_index,
                                                    piece placed_type, square placed_on,
                                                    int const check_directions[8],
                                                    unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!guards(king_square[White],placed_type,placed_on))
  {
    square const placed_from = black[placed_index].square;
    unsigned int const time = count_nr_of_moves_from_to_no_check(placed_type,
                                                                 placed_from,
                                                                 placed_type,
                                                                 placed_on);
    if (time<=nr_remaining_black_moves)
    {
      SetPiece(placed_type,placed_on,black[placed_index].flags);
      mate_continue_intercepting_checks(nr_remaining_black_moves-time,
                                        nr_remaining_white_moves,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        check_directions,
                                        nr_of_check_directions);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_check_black(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       square placed_on,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
    if (black[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = black[placed_index].type;

      black[placed_index].usage = piece_intercepts;

      if (placed_type==pn)
      {
        if (placed_on>=square_a2)
          mate_intercept_check_with_unpromoted_black_pawn(nr_remaining_black_moves,
                                                          nr_remaining_white_moves,
                                                          max_nr_allowed_captures_by_black_pieces,
                                                          max_nr_allowed_captures_by_white_pieces,
                                                          n,
                                                          placed_index,placed_on,
                                                          check_directions,
                                                          nr_of_check_directions);
        mate_intercept_check_with_promoted_black_pawn(nr_remaining_black_moves,
                                                      nr_remaining_white_moves,
                                                      max_nr_allowed_captures_by_black_pieces,
                                                      max_nr_allowed_captures_by_white_pieces,
                                                      n,
                                                      placed_index,placed_on,
                                                      check_directions,
                                                      nr_of_check_directions);
      }
      else
        mate_intercept_check_with_black_officer(nr_remaining_black_moves,
                                                nr_remaining_white_moves,
                                                max_nr_allowed_captures_by_black_pieces,
                                                max_nr_allowed_captures_by_white_pieces,
                                                n,
                                                placed_index,placed_type,placed_on,
                                                check_directions,
                                                nr_of_check_directions);

      black[placed_index].usage = piece_is_unused;
    }

  e[placed_on] = vide;
  spec[placed_on] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_intercept_next_check(unsigned int nr_remaining_black_moves,
                                      unsigned int nr_remaining_white_moves,
                                      unsigned int max_nr_allowed_captures_by_black_pieces,
                                      unsigned int max_nr_allowed_captures_by_white_pieces,
                                      stip_length_type n,
                                      int const check_directions[8],
                                      unsigned int nr_of_check_directions)
{
  square to_be_blocked;
  int const current_dir = check_directions[nr_of_check_directions-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (to_be_blocked = king_square[White]+current_dir;
       e[to_be_blocked]==vide;
       to_be_blocked += current_dir)
  {
    mate_intercept_check_black(nr_remaining_black_moves,
                               nr_remaining_white_moves,
                               max_nr_allowed_captures_by_black_pieces,
                               max_nr_allowed_captures_by_white_pieces,
                               n,
                               to_be_blocked,
                               check_directions,
                               nr_of_check_directions-1);
    mate_intercept_check_white(nr_remaining_black_moves,
                               nr_remaining_white_moves,
                               max_nr_allowed_captures_by_black_pieces,
                               max_nr_allowed_captures_by_white_pieces,
                               n,
                               to_be_blocked,
                               check_directions,
                               nr_of_check_directions-1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_continue_intercepting_checks(unsigned int nr_remaining_black_moves,
                                              unsigned int nr_remaining_white_moves,
                                              unsigned int max_nr_allowed_captures_by_black_pieces,
                                              unsigned int max_nr_allowed_captures_by_white_pieces,
                                              stip_length_type n,
                                              int const check_directions[8],
                                              unsigned int nr_of_check_directions)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (nr_of_check_directions==0)
    mate_test_target_position(nr_remaining_black_moves,
                              nr_remaining_white_moves,
                              max_nr_allowed_captures_by_black_pieces,
                              max_nr_allowed_captures_by_white_pieces,
                              n);
  else
    mate_intercept_next_check(nr_remaining_black_moves,
                              nr_remaining_white_moves,
                              max_nr_allowed_captures_by_black_pieces,
                              max_nr_allowed_captures_by_white_pieces,
                              n,
                              check_directions,
                              nr_of_check_directions);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int find_check_directions(Side side, int check_directions[8])
{
  unsigned int result = 0;
  unsigned int i;

  /* don't use obs - intelligent mode supports boards with holes */
  piece const temporary_block = dummyb;

  for (i = vec_queen_end; i>=vec_queen_start ; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[side]+vec_i]==vide)
      e[king_square[side]+vec_i] = temporary_block;
  }

  for (i = vec_queen_end; i>=vec_queen_start ; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[side]+vec_i]==temporary_block)
    {
      e[king_square[side]+vec_i] = vide;
      if (echecc(nbply,side))
      {
        check_directions[result] = vec_i;
        ++result;
      }
      e[king_square[side]+vec_i] = temporary_block;
    }
  }

  for (i = vec_queen_end; i>=vec_queen_start ; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[side]+vec_i]==temporary_block)
      e[king_square[side]+vec_i] = vide;
  }

  return result;
}

static void mate_intercept_checks(unsigned int nr_remaining_black_moves,
                                  unsigned int nr_remaining_white_moves,
                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                  stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    int check_directions[8];
    unsigned int const nr_of_check_directions = find_check_directions(White,
                                                                      check_directions);

    mate_continue_intercepting_checks(nr_remaining_black_moves,
                                      nr_remaining_white_moves,
                                      max_nr_allowed_captures_by_black_pieces,
                                      max_nr_allowed_captures_by_white_pieces,
                                      n,
                                      check_directions,
                                      nr_of_check_directions);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_immobilise_by_pin_by_officer(unsigned int nr_remaining_black_moves,
                                              unsigned int nr_remaining_white_moves,
                                              unsigned int max_nr_allowed_captures_by_black_pieces,
                                              unsigned int max_nr_allowed_captures_by_white_pieces,
                                              stip_length_type n,
                                              piece pinner_orig_type,
                                              piece pinner_type,
                                              Flags pinner_flags,
                                              square pinner_comes_from,
                                              square pin_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TracePiece(pinner_orig_type);
  TracePiece(pinner_type);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(pinner_orig_type,
                                                                 pinner_comes_from,
                                                                 pinner_type,
                                                                 pin_from);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(pinner_type,pin_from,pinner_flags);
      mate_test_target_position(nr_remaining_black_moves,
                                nr_remaining_white_moves-time,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_immobilise_by_pin_by_promoted_pawn(unsigned int nr_remaining_black_moves,
                                                    unsigned int nr_remaining_white_moves,
                                                    unsigned int max_nr_allowed_captures_by_black_pieces,
                                                    unsigned int max_nr_allowed_captures_by_white_pieces,
                                                    stip_length_type n,
                                                    Flags pinner_flags,
                                                    square pinner_comes_from,
                                                    square pin_from,
                                                    boolean diagonal)
{
  piece const minor_pinner_type = diagonal ? fb : tb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  mate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    pb,minor_pinner_type,pinner_flags,
                                    pinner_comes_from,
                                    pin_from);
  mate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    pb,db,pinner_flags,
                                    pinner_comes_from,
                                    pin_from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_pin_black_piece_on_1square_by_1piece(unsigned int nr_remaining_black_moves,
                                                      unsigned int nr_remaining_white_moves,
                                                      unsigned int max_nr_allowed_captures_by_black_pieces,
                                                      unsigned int max_nr_allowed_captures_by_white_pieces,
                                                      stip_length_type n,
                                                      square sq_to_be_pinned,
                                                      square pin_on,
                                                      unsigned int pinner_index,
                                                      boolean diagonal)
{
  piece const pinner_type = white[pinner_index].type;
  Flags const pinner_flags = white[pinner_index].flags;
  square const pinner_comes_from = white[pinner_index].square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(sq_to_be_pinned);
  TraceSquare(pin_on);
  TraceFunctionParam("%u",pinner_index);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  switch (pinner_type)
  {
    case db:
      mate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                        nr_remaining_white_moves,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        db,db,pinner_flags,
                                        pinner_comes_from,
                                        pin_on);
      break;

    case tb:
      if (!diagonal)
        mate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          tb,tb,pinner_flags,
                                          pinner_comes_from,
                                          pin_on);
      break;

    case fb:
      if (diagonal)
        mate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          fb,fb,pinner_flags,
                                          pinner_comes_from,
                                          pin_on);
      break;

    case cb:
      break;

    case pb:
      mate_immobilise_by_pin_by_promoted_pawn(nr_remaining_black_moves,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n,
                                              pinner_flags,
                                              pinner_comes_from,
                                              pin_on,
                                              diagonal);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_pin_black_piece(unsigned int nr_remaining_black_moves,
                                 unsigned int nr_remaining_white_moves,
                                 unsigned int max_nr_allowed_captures_by_black_pieces,
                                 unsigned int max_nr_allowed_captures_by_white_pieces,
                                 stip_length_type n,
                                 square sq_to_be_pinned)
{
  int const dir = sq_to_be_pinned-king_square[Black];
  boolean const diagonal = SquareCol(king_square[Black]+dir)==SquareCol(king_square[Black]);
  square pin_on;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(sq_to_be_pinned);
  TraceFunctionParamListEnd();

  for (pin_on = sq_to_be_pinned+dir; e[pin_on]==vide; pin_on += dir)
  {
    unsigned int pinner_index;
    for (pinner_index = 1; pinner_index<MaxPiece[White]; ++pinner_index)
      if (white[pinner_index].usage==piece_is_unused)
      {
        white[pinner_index].usage = piece_pins;

        mate_pin_black_piece_on_1square_by_1piece(nr_remaining_black_moves,
                                                  nr_remaining_white_moves,
                                                  max_nr_allowed_captures_by_black_pieces,
                                                  max_nr_allowed_captures_by_white_pieces,
                                                  n,
                                                  sq_to_be_pinned,
                                                  pin_on,
                                                  pinner_index,
                                                  diagonal);

        white[pinner_index].usage = piece_is_unused;
      }

    e[pin_on] = vide;
    spec[pin_on] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_immobilise_by_pin_by_officer(unsigned int nr_remaining_black_moves,
                                                   unsigned int nr_remaining_white_moves,
                                                   unsigned int max_nr_allowed_captures_by_black_pieces,
                                                   unsigned int max_nr_allowed_captures_by_white_pieces,
                                                   stip_length_type n,
                                                   piece pinner_orig_type,
                                                   piece pinner_type,
                                                   Flags pinner_flags,
                                                   square pinner_comes_from,
                                                   square pin_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TracePiece(pinner_orig_type);
  TracePiece(pinner_type);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(pinner_orig_type,
                                                                 pinner_comes_from,
                                                                 pinner_type,
                                                                 pin_from);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(pinner_type,pin_from,pinner_flags);
      stalemate_test_target_position(nr_remaining_black_moves,
                                     nr_remaining_white_moves-time,
                                     max_nr_allowed_captures_by_black_pieces-1,
                                     max_nr_allowed_captures_by_white_pieces,
                                     n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_immobilise_by_pin_by_promoted_pawn(unsigned int nr_remaining_black_moves,
                                                         unsigned int nr_remaining_white_moves,
                                                         unsigned int max_nr_allowed_captures_by_black_pieces,
                                                         unsigned int max_nr_allowed_captures_by_white_pieces,
                                                         stip_length_type n,
                                                         Flags pinner_flags,
                                                         square pinner_comes_from,
                                                         square pin_from,
                                                         boolean diagonal)
{
  piece const minor_pinner_type = diagonal ? fb : tb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  stalemate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                         nr_remaining_white_moves,
                                         max_nr_allowed_captures_by_black_pieces,
                                         max_nr_allowed_captures_by_white_pieces,
                                         n,
                                         pb,minor_pinner_type,pinner_flags,
                                         pinner_comes_from,
                                         pin_from);
  stalemate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                         nr_remaining_white_moves,
                                         max_nr_allowed_captures_by_black_pieces,
                                         max_nr_allowed_captures_by_white_pieces,
                                         n,
                                         pb,db,pinner_flags,
                                         pinner_comes_from,
                                         pin_from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_line_empty(square start, square end, int dir)
{
  boolean result = true;
  square sq;

  TraceFunctionEntry(__func__);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (sq = start+dir; e[sq]==vide; sq += dir)
  {
    /* nothing */
  }

  result = sq==end;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void stalemate_immobilise_by_pin_on_1square_by_1piece(unsigned int nr_remaining_black_moves,
                                                             unsigned int nr_remaining_white_moves,
                                                             unsigned int max_nr_allowed_captures_by_black_pieces,
                                                             unsigned int max_nr_allowed_captures_by_white_pieces,
                                                             stip_length_type n,
                                                             square sq_to_be_pinned,
                                                             square pin_on,
                                                             unsigned int pinner_index,
                                                             boolean diagonal)
{
  piece const pinner_type = white[pinner_index].type;
  Flags const pinner_flags = white[pinner_index].flags;
  square const pinner_comes_from = white[pinner_index].square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(sq_to_be_pinned);
  TraceSquare(pin_on);
  TraceFunctionParam("%u",pinner_index);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();
  switch (pinner_type)
  {
    case cb:
      break;

    case tb:
      if (!diagonal)
        stalemate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                               nr_remaining_white_moves,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               tb,tb,pinner_flags,
                                               pinner_comes_from,
                                               pin_on);
      break;

    case fb:
      if (diagonal)
        stalemate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                               nr_remaining_white_moves,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               fb,fb,pinner_flags,
                                               pinner_comes_from,
                                               pin_on);
      break;

    case db:
      stalemate_immobilise_by_pin_by_officer(nr_remaining_black_moves,
                                             nr_remaining_white_moves,
                                             max_nr_allowed_captures_by_black_pieces,
                                             max_nr_allowed_captures_by_white_pieces,
                                             n,
                                             db,db,pinner_flags,
                                             pinner_comes_from,
                                             pin_on);
      break;

    case pb:
      stalemate_immobilise_by_pin_by_promoted_pawn(nr_remaining_black_moves,
                                                   nr_remaining_white_moves,
                                                   max_nr_allowed_captures_by_black_pieces,
                                                   max_nr_allowed_captures_by_white_pieces,
                                                   n,
                                                   pinner_flags,
                                                   pinner_comes_from,
                                                   pin_on,
                                                   diagonal);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_immobilise_by_pin(unsigned int nr_remaining_black_moves,
                                        unsigned int nr_remaining_white_moves,
                                        unsigned int max_nr_allowed_captures_by_black_pieces,
                                        unsigned int max_nr_allowed_captures_by_white_pieces,
                                        stip_length_type n,
                                        square sq_to_be_pinned)
{
  int const dir = CheckDirQueen[sq_to_be_pinned-king_square[Black]];
  piece const pinned_type = e[sq_to_be_pinned];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(sq_to_be_pinned);
  TraceFunctionParamListEnd();

  if (max_nr_allowed_captures_by_black_pieces>0
      && dir!=0          /* we can only pin on queen lines */
      && pinned_type!=dn /* queens cannot be pinned */
      /* bishops can only be pined on rook lines and vice versa */
      && !(CheckDirBishop[dir]!=0 && pinned_type==fn)
      && !(CheckDirRook[dir]!=0 && pinned_type==tn)
      && is_line_empty(king_square[Black],sq_to_be_pinned,dir))
  {
    boolean const diagonal = SquareCol(king_square[Black]+dir)==SquareCol(king_square[Black]);

    square pin_on;
    for (pin_on = sq_to_be_pinned+dir; e[pin_on]==vide; pin_on += dir)
    {
      unsigned int pinner_index;
      for (pinner_index = 1; pinner_index<MaxPiece[White]; ++pinner_index)
      {
        if (white[pinner_index].usage==piece_is_unused)
        {
          white[pinner_index].usage = piece_pins;

          stalemate_immobilise_by_pin_on_1square_by_1piece(nr_remaining_black_moves,
                                                           nr_remaining_white_moves,
                                                           max_nr_allowed_captures_by_black_pieces,
                                                           max_nr_allowed_captures_by_white_pieces,
                                                           n,
                                                           sq_to_be_pinned,
                                                           pin_on,
                                                           pinner_index,
                                                           diagonal);

          white[pinner_index].usage = piece_is_unused;
        }
      }

      e[pin_on] = vide;
      spec[pin_on] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_fix_white_king_on_diagram_square(unsigned int nr_remaining_black_moves,
                                                       unsigned int nr_remaining_white_moves,
                                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                                       stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!are_kings_too_close(white[index_of_king].square)
      && !is_white_king_uninterceptably_attacked_by_non_king(white[index_of_king].square))
  {
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;

    king_square[White] = white[index_of_king].square;
    SetPiece(roib,king_square[White],white[index_of_king].flags);

    if (is_white_king_interceptably_attacked())
    {
      unsigned int const nr_of_checks_to_black = 0;
      stalemate_intercept_checks(nr_remaining_black_moves,
                                 nr_remaining_white_moves,
                                 max_nr_allowed_captures_by_black_pieces,
                                 max_nr_allowed_captures_by_white_pieces,
                                 n,
                                 nr_of_checks_to_black,
                                 White);
    }
    else
      stalemate_test_target_position(nr_remaining_black_moves,
                                     nr_remaining_white_moves,
                                     max_nr_allowed_captures_by_black_pieces,
                                     max_nr_allowed_captures_by_white_pieces,
                                     n);

    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;

    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void stalemate_intercept_check_with_promoted_black_pawn(unsigned int nr_remaining_black_moves,
                                                               unsigned int nr_remaining_white_moves,
                                                               unsigned int max_nr_allowed_captures_by_black_pieces,
                                                               unsigned int max_nr_allowed_captures_by_white_pieces,
                                                               stip_length_type n,
                                                               Side side,
                                                               square to_be_blocked,
                                                               Flags blocker_flags,
                                                               square blocker_comes_from,
                                                               int const check_directions[8],
                                                               unsigned int nr_of_check_directions,
                                                               unsigned int nr_checks_to_opponent)
{
  /* A rough check whether it is worth thinking about promotions */
  unsigned int time = (blocker_comes_from>=square_a7
                       ? 5
                       : blocker_comes_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  assert(time<=5);

  if (to_be_blocked>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++time;

  if (nr_remaining_black_moves>=time)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!uninterceptably_attacks_king(White,to_be_blocked,pp))
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(blocker_comes_from,
                                                                           pp,
                                                                           to_be_blocked);
        if (time<=nr_remaining_black_moves)
        {
          boolean const white_check = guards(king_square[White],pp,to_be_blocked);
          if (!(side==White && white_check))
          {
            if (side==Black && white_check)
              ++nr_checks_to_opponent;
            SetPiece(pp,to_be_blocked,blocker_flags);
            stalemate_continue_intercepting_checks(nr_remaining_black_moves-time,
                                                   nr_remaining_white_moves,
                                                   max_nr_allowed_captures_by_black_pieces,
                                                   max_nr_allowed_captures_by_white_pieces,
                                                   n,
                                                   check_directions,
                                                   nr_of_check_directions,
                                                   nr_checks_to_opponent,
                                                   side);
          }
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_with_unpromoted_black_pawn(unsigned int nr_remaining_black_moves,
                                                                 unsigned int nr_remaining_white_moves,
                                                                 unsigned int max_nr_allowed_captures_by_black_pieces,
                                                                 unsigned int max_nr_allowed_captures_by_white_pieces,
                                                                 stip_length_type n,
                                                                 Side side,
                                                                 square to_be_blocked,
                                                                 Flags blocker_flags,
                                                                 square blocker_comes_from,
                                                                 int const check_directions[8],
                                                                 unsigned int nr_of_check_directions,
                                                                 unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,pn))
  {
    unsigned int const nr_required_captures = abs(blocker_comes_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                          blocker_comes_from,
                                                                          to_be_blocked);
    if (time<=nr_remaining_black_moves
        && nr_required_captures<=max_nr_allowed_captures_by_black_pieces
        && !(side==White && guards(king_square[White],pn,to_be_blocked)))
    {
      SetPiece(pn,to_be_blocked,blocker_flags);
      stalemate_continue_intercepting_checks(nr_remaining_black_moves-time,
                                             nr_remaining_white_moves,
                                             max_nr_allowed_captures_by_black_pieces-nr_required_captures,
                                             max_nr_allowed_captures_by_white_pieces,
                                             n,
                                             check_directions,
                                             nr_of_check_directions,
                                             nr_checks_to_opponent,
                                             side);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_with_black_officer(unsigned int nr_remaining_black_moves,
                                                         unsigned int nr_remaining_white_moves,
                                                         unsigned int max_nr_allowed_captures_by_black_pieces,
                                                         unsigned int max_nr_allowed_captures_by_white_pieces,
                                                         stip_length_type n,
                                                         Side side,
                                                         square to_be_blocked,
                                                         piece blocker_type,
                                                         Flags blocker_flags,
                                                         square blocker_comes_from,
                                                         int const check_directions[8],
                                                         unsigned int nr_of_check_directions,
                                                         unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocker_comes_from);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,blocker_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                 blocker_comes_from,
                                                                 blocker_type,
                                                                 to_be_blocked);
    if (time<=nr_remaining_black_moves)
    {
      boolean const white_check = guards(king_square[White],blocker_type,to_be_blocked);
      if (!(side==White && white_check))
      {
        if (side==Black && white_check)
          ++nr_checks_to_opponent;
        SetPiece(blocker_type,to_be_blocked,blocker_flags);
        stalemate_continue_intercepting_checks(nr_remaining_black_moves-time,
                                               nr_remaining_white_moves,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               check_directions,
                                               nr_of_check_directions,
                                               nr_checks_to_opponent,
                                               side);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_black(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n,
                                            Side side,
                                            square to_be_blocked,
                                            int const check_directions[8],
                                            unsigned int nr_of_check_directions,
                                            unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (max_nr_allowed_captures_by_white_pieces>=1)
  {
    unsigned int i;

    --max_nr_allowed_captures_by_white_pieces;

    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        piece const blocker_type = black[i].type;
        Flags const blocker_flags = black[i].flags;
        square const blocker_comes_from = black[i].square;

        black[i].usage = piece_intercepts;

        if (blocker_type==pn)
        {
          stalemate_intercept_check_with_promoted_black_pawn(nr_remaining_black_moves,
                                                             nr_remaining_white_moves,
                                                             max_nr_allowed_captures_by_black_pieces,
                                                             max_nr_allowed_captures_by_white_pieces,
                                                             n,
                                                             side,
                                                             to_be_blocked,
                                                             blocker_flags,
                                                             blocker_comes_from,
                                                             check_directions,
                                                             nr_of_check_directions,
                                                             nr_checks_to_opponent);
          if (to_be_blocked>=square_a2)
            stalemate_intercept_check_with_unpromoted_black_pawn(nr_remaining_black_moves,
                                                                 nr_remaining_white_moves,
                                                                 max_nr_allowed_captures_by_black_pieces,
                                                                 max_nr_allowed_captures_by_white_pieces,
                                                                 n,
                                                                 side,
                                                                 to_be_blocked,
                                                                 blocker_flags,
                                                                 blocker_comes_from,
                                                                 check_directions,
                                                                 nr_of_check_directions,
                                                                 nr_checks_to_opponent);
        }
        else
          stalemate_intercept_check_with_black_officer(nr_remaining_black_moves,
                                                       nr_remaining_white_moves,
                                                       max_nr_allowed_captures_by_black_pieces,
                                                       max_nr_allowed_captures_by_white_pieces,
                                                       n,
                                                       side,
                                                       to_be_blocked,
                                                       blocker_type,
                                                       blocker_flags,
                                                       blocker_comes_from,
                                                       check_directions,
                                                       nr_of_check_directions,
                                                       nr_checks_to_opponent);

        black[i].usage = piece_is_unused;
      }

    e[to_be_blocked] = vide;
    spec[to_be_blocked] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_test_target_position(unsigned int nr_remaining_black_moves,
                                           unsigned int nr_remaining_white_moves,
                                           unsigned int max_nr_allowed_captures_by_black_pieces,
                                           unsigned int max_nr_allowed_captures_by_white_pieces,
                                           stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,Black));
  assert(!echecc(nbply,White));
  if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
    stalemate_deal_with_unused_pieces(nr_remaining_black_moves,
                                      nr_remaining_white_moves,
                                      max_nr_allowed_captures_by_black_pieces,
                                      max_nr_allowed_captures_by_white_pieces,
                                      n);
  else
    stalemate_immobilise_black(nr_remaining_black_moves,
                               nr_remaining_white_moves,
                               max_nr_allowed_captures_by_black_pieces,
                               max_nr_allowed_captures_by_white_pieces,
                               n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_continue_after_block(unsigned int nr_remaining_black_moves,
                                           unsigned int nr_remaining_white_moves,
                                           unsigned int max_nr_allowed_captures_by_black_pieces,
                                           unsigned int max_nr_allowed_captures_by_white_pieces,
                                           stip_length_type n,
                                           Side side,
                                           square to_be_blocked,
                                           piece blocker_type,
                                           unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (guards(king_square[side],blocker_type,to_be_blocked))
  {
    unsigned int const nr_checks_to_opponent = 0;
    stalemate_intercept_checks(nr_remaining_black_moves,
                               nr_remaining_white_moves,
                               max_nr_allowed_captures_by_black_pieces,
                               max_nr_allowed_captures_by_white_pieces,
                               n,
                               nr_checks_to_opponent,
                               side);
  }
  else
    stalemate_test_target_position(nr_remaining_black_moves,
                                   nr_remaining_white_moves,
                                   max_nr_allowed_captures_by_black_pieces,
                                   max_nr_allowed_captures_by_white_pieces,
                                   n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_black_promotion(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n,
                                            square to_be_blocked,
                                            Flags blocker_flags,
                                            square blocker_comes_from)
{
  /* A rough check whether it is worth thinking about promotions */
  unsigned int time = (blocker_comes_from>=square_a7
                       ? 5
                       : blocker_comes_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  assert(time<=5);

  if (to_be_blocked>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++time;

  if (nr_remaining_black_moves>=time)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!uninterceptably_attacks_king(White,to_be_blocked,pp))
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(blocker_comes_from,
                                                                           pp,
                                                                           to_be_blocked);
        if (time<=nr_remaining_black_moves)
        {
          unsigned int const nr_checks_to_black = 0;
          SetPiece(pp,to_be_blocked,blocker_flags);
          stalemate_continue_after_block(nr_remaining_black_moves-time,
                                         nr_remaining_white_moves,
                                         max_nr_allowed_captures_by_black_pieces,
                                         max_nr_allowed_captures_by_white_pieces,
                                         n,
                                         White,
                                         to_be_blocked,
                                         pp,
                                         nr_checks_to_black);
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_unpromoted_black_pawn(unsigned int nr_remaining_black_moves,
                                                  unsigned int nr_remaining_white_moves,
                                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                                  stip_length_type n,
                                                  square to_be_blocked,
                                                  Flags blocker_flags,
                                                  square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,pn))
  {
    unsigned int const nr_required_captures = abs(blocker_comes_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                          blocker_comes_from,
                                                                          to_be_blocked);
    if (time<=nr_remaining_black_moves
        && nr_required_captures<=max_nr_allowed_captures_by_black_pieces)
    {
      SetPiece(pn,to_be_blocked,blocker_flags);
      stalemate_test_target_position(nr_remaining_black_moves-time,
                                     nr_remaining_white_moves,
                                     max_nr_allowed_captures_by_black_pieces-nr_required_captures,
                                     max_nr_allowed_captures_by_white_pieces,
                                     n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_black_officer(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          square to_be_blocked,
                                          piece blocker_type,
                                          Flags blocker_flags,
                                          square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,blocker_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                 blocker_comes_from,
                                                                 blocker_type,
                                                                 to_be_blocked);
    if (time<=nr_remaining_black_moves)
    {
      unsigned int const nr_checks_to_black = 0;
      SetPiece(blocker_type,to_be_blocked,blocker_flags);
      stalemate_continue_after_block(nr_remaining_black_moves-time,
                                     nr_remaining_white_moves,
                                     max_nr_allowed_captures_by_black_pieces,
                                     max_nr_allowed_captures_by_white_pieces,
                                     n,
                                     White,
                                     to_be_blocked,
                                     blocker_type,
                                     nr_checks_to_black);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_black_block(unsigned int nr_remaining_black_moves,
                                  unsigned int nr_remaining_white_moves,
                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                  stip_length_type n,
                                  square to_be_blocked)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (max_nr_allowed_captures_by_white_pieces>=1)
  {
    --max_nr_allowed_captures_by_white_pieces;

    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        piece const blocker_type = black[i].type;
        Flags const blocker_flags = black[i].flags;
        square const blocker_comes_from = black[i].square;

        black[i].usage = piece_blocks;

        if (blocker_type==pn)
        {
          stalemate_block_black_promotion(nr_remaining_black_moves,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          to_be_blocked,
                                          blocker_flags,
                                          blocker_comes_from);
          if (to_be_blocked>=square_a2)
            stalemate_block_unpromoted_black_pawn(nr_remaining_black_moves,
                                                  nr_remaining_white_moves,
                                                  max_nr_allowed_captures_by_black_pieces,
                                                  max_nr_allowed_captures_by_white_pieces,
                                                  n,
                                                  to_be_blocked,
                                                  blocker_flags,
                                                  blocker_comes_from);
        }
        else
          stalemate_block_black_officer(nr_remaining_black_moves,
                                        nr_remaining_white_moves,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        to_be_blocked,
                                        blocker_type,
                                        blocker_flags,
                                        blocker_comes_from);

        black[i].usage = piece_is_unused;
      }

    e[to_be_blocked] = vide;
    spec[to_be_blocked] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_with_unpromoted_white_pawn(unsigned int nr_remaining_black_moves,
                                                                 unsigned int nr_remaining_white_moves,
                                                                 unsigned int max_nr_allowed_captures_by_black_pieces,
                                                                 unsigned int max_nr_allowed_captures_by_white_pieces,
                                                                 stip_length_type n,
                                                                 Side side,
                                                                 unsigned int blocker_index,
                                                                 square to_be_blocked,
                                                                 int const check_directions[8],
                                                                 unsigned int nr_of_check_directions,
                                                                 unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,pb))
  {
    square const blocks_from = white[blocker_index].square;
    unsigned int const nr_captures_required = abs(blocks_from%onerow
                                                  - to_be_blocked%onerow);
    if (max_nr_allowed_captures_by_white_pieces>=nr_captures_required)
    {
      unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                            blocks_from,
                                                                            to_be_blocked);
      if (time<=nr_remaining_white_moves
          && !(side==Black && guards(king_square[Black],pb,to_be_blocked)))
      {
        SetPiece(pb,to_be_blocked,white[blocker_index].flags);
        stalemate_continue_intercepting_checks(nr_remaining_black_moves,
                                               nr_remaining_white_moves-time,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces-nr_captures_required,
                                               n,
                                               check_directions,
                                               nr_of_check_directions,
                                               nr_checks_to_opponent,
                                               side);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_with_promoted_white_pawn(unsigned int nr_remaining_black_moves,
                                                               unsigned int nr_remaining_white_moves,
                                                               unsigned int max_nr_allowed_captures_by_black_pieces,
                                                               unsigned int max_nr_allowed_captures_by_white_pieces,
                                                               stip_length_type n,
                                                               Side side,
                                                               unsigned int blocker_index,
                                                               square to_be_blocked,
                                                               int const check_directions[8],
                                                               unsigned int nr_of_check_directions,
                                                               unsigned int nr_checks_to_opponent)
{
  unsigned int const nr_moves_required = (to_be_blocked<=square_h7
                                          ? moves_to_white_prom[blocker_index]+1
                                          : moves_to_white_prom[blocker_index]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_moves_required);
  if (nr_remaining_white_moves>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!uninterceptably_attacks_king(Black,to_be_blocked,pp))
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(white[blocker_index].square,
                                                                           pp,
                                                                           to_be_blocked);
        if (time<=nr_remaining_white_moves)
        {
          boolean const black_check = guards(king_square[Black],pp,to_be_blocked);
          if (!(side==Black && black_check))
          {
            if (side==White && black_check)
              ++nr_checks_to_opponent;
            SetPiece(pp,to_be_blocked,white[blocker_index].flags);
            stalemate_continue_intercepting_checks(nr_remaining_black_moves,
                                                   nr_remaining_white_moves-time,
                                                   max_nr_allowed_captures_by_black_pieces,
                                                   max_nr_allowed_captures_by_white_pieces,
                                                   n,
                                                   check_directions,
                                                   nr_of_check_directions,
                                                   nr_checks_to_opponent,
                                                   side);
          }
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_with_white_king(unsigned int nr_remaining_black_moves,
                                                      unsigned int nr_remaining_white_moves,
                                                      unsigned int max_nr_allowed_captures_by_black_pieces,
                                                      unsigned int max_nr_allowed_captures_by_white_pieces,
                                                      stip_length_type n,
                                                      Side side,
                                                      square to_be_blocked,
                                                      int const check_directions[8],
                                                      unsigned int nr_of_check_directions,
                                                      unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!are_kings_too_close(to_be_blocked)
      && !is_white_king_uninterceptably_attacked_by_non_king(to_be_blocked))
  {
    unsigned int const time = count_nr_of_moves_from_to_king(roib,
                                                             white[index_of_king].square,
                                                             to_be_blocked);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(roib,to_be_blocked,white[index_of_king].flags);
      king_square[White] = to_be_blocked;

      if (is_white_king_interceptably_attacked())
        ++nr_checks_to_opponent;

      stalemate_continue_intercepting_checks(nr_remaining_black_moves,
                                             nr_remaining_white_moves-time,
                                             max_nr_allowed_captures_by_black_pieces,
                                             max_nr_allowed_captures_by_white_pieces,
                                             n,
                                             check_directions,
                                             nr_of_check_directions,
                                             nr_checks_to_opponent,
                                             side);

      king_square[White] = initsquare;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_with_white_officer(unsigned int nr_remaining_black_moves,
                                                         unsigned int nr_remaining_white_moves,
                                                         unsigned int max_nr_allowed_captures_by_black_pieces,
                                                         unsigned int max_nr_allowed_captures_by_white_pieces,
                                                         stip_length_type n,
                                                         Side side,
                                                         piece blocker_type,
                                                         unsigned int blocker_index,
                                                         square to_be_blocked,
                                                         int const check_directions[8],
                                                         unsigned int nr_of_check_directions,
                                                         unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TracePiece(blocker_type);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,blocker_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                 white[blocker_index].square,
                                                                 blocker_type,
                                                                 to_be_blocked);
    if (time<=nr_remaining_white_moves)
    {
      boolean const black_check = guards(king_square[Black],blocker_type,to_be_blocked);
      if (!(side==Black && black_check))
      {
        if (side==White && black_check)
          ++nr_checks_to_opponent;
        SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
        stalemate_continue_intercepting_checks(nr_remaining_black_moves,
                                               nr_remaining_white_moves-time,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               check_directions,
                                               nr_of_check_directions,
                                               nr_checks_to_opponent,
                                               side);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_check_white(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n,
                                            Side side,
                                            square to_be_blocked,
                                            int const check_directions[8],
                                            unsigned int nr_of_check_directions,
                                            unsigned int nr_checks_to_opponent)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_intercepts;
    stalemate_intercept_check_with_white_king(nr_remaining_black_moves,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n,
                                              side,
                                              to_be_blocked,
                                              check_directions,
                                              nr_of_check_directions,
                                              nr_checks_to_opponent);
    white[index_of_king].usage = piece_is_unused;
  }

  if (max_nr_allowed_captures_by_black_pieces>=1)
  {
    --max_nr_allowed_captures_by_black_pieces;

    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        piece const blocker_type = white[blocker_index].type;

        white[blocker_index].usage = piece_intercepts;

        if (blocker_type==pb)
        {
          stalemate_intercept_check_with_promoted_white_pawn(nr_remaining_black_moves,
                                                             nr_remaining_white_moves,
                                                             max_nr_allowed_captures_by_black_pieces,
                                                             max_nr_allowed_captures_by_white_pieces,
                                                             n,
                                                             side,
                                                             blocker_index,
                                                             to_be_blocked,
                                                             check_directions,
                                                             nr_of_check_directions,
                                                             nr_checks_to_opponent);
          stalemate_intercept_check_with_unpromoted_white_pawn(nr_remaining_black_moves,
                                                               nr_remaining_white_moves,
                                                               max_nr_allowed_captures_by_black_pieces,
                                                               max_nr_allowed_captures_by_white_pieces,
                                                               n,
                                                               side,
                                                               blocker_index,
                                                               to_be_blocked,
                                                               check_directions,
                                                               nr_of_check_directions,
                                                               nr_checks_to_opponent);
        }
        else
          stalemate_intercept_check_with_white_officer(nr_remaining_black_moves,
                                                       nr_remaining_white_moves,
                                                       max_nr_allowed_captures_by_black_pieces,
                                                       max_nr_allowed_captures_by_white_pieces,
                                                       n,
                                                       side,
                                                       blocker_type,
                                                       blocker_index,
                                                       to_be_blocked,
                                                       check_directions,
                                                       nr_of_check_directions,
                                                       nr_checks_to_opponent);

        white[blocker_index].usage = piece_is_unused;
      }
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean can_white_pin(unsigned int nr_remaining_white_moves)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; i++)
    if (!(white[i].usage!=piece_is_unused
          || white[i].type==cb
          || (white[i].type==pb && nr_remaining_white_moves<moves_to_white_prom[i])))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
boolean can_we_block_all_necessary_squares(unsigned int const nr_blocks_needed[nr_sides])
{
  boolean result;
  unsigned int nr_unused_pieces[nr_sides] = { 0, 0 };
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

 for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
      ++nr_unused_pieces[Black];

  if (nr_unused_pieces[Black]<nr_blocks_needed[Black])
    result = false;
  else
  {
    for (i = 0; i<MaxPiece[White]; ++i)
      if (white[i].usage==piece_is_unused)
        ++nr_unused_pieces[White];

    result = (nr_unused_pieces[White]+nr_unused_pieces[Black]
                                                       >=nr_blocks_needed[Black]+nr_blocks_needed[White]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef enum
{
  no_block_needed_on_square,
  white_block_sufficient_on_square,
  black_block_needed_on_square
} block_requirement_type;

/* Find the most expensive square (if any) that must be blocked by Black
 * @param nr_remaining_black_moves number of remaining black moves
 * @param block_requirement blocking requirements for each square
 * @return * nullsquare more squares need to be blocked than Black can in the
 *                      nr_remaining_black_moves remaining moves
 *         * initsquare no square is required to be blocked by Black
 *         * otherwise: most expensive square that must be blocked by Black
 */
static square find_most_expensive_square_to_be_blocked_by_black(unsigned int nr_remaining_black_moves,
                                                                block_requirement_type const block_requirement[maxsquare+4])
{
  square result = initsquare;
  int max_number_black_moves_to_squares_to_be_blocked = -1;
  unsigned int total_number_black_moves_to_squares_to_be_blocked = 0;

  square const *bnp;
  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (block_requirement[*bnp]==black_block_needed_on_square)
    {
      int const nr_black_blocking_moves = count_nr_black_moves_to_square(*bnp,nr_remaining_black_moves);
      total_number_black_moves_to_squares_to_be_blocked += nr_black_blocking_moves;
      if (total_number_black_moves_to_squares_to_be_blocked>nr_remaining_black_moves)
      {
        result = nullsquare;
        break;
      }
      else if (nr_black_blocking_moves>max_number_black_moves_to_squares_to_be_blocked)
      {
        max_number_black_moves_to_squares_to_be_blocked = nr_black_blocking_moves;
        result = *bnp;
      }
    }


  return result;
}

typedef enum
{
  no_requirement,
  block_required,
  king_block_required,
  pin_required,
  immobilisation_impossible
} last_found_trouble_square_status_type;

typedef struct
{
  square position_of_trouble_maker;
  square last_found_trouble_square;
  unsigned int nr_blocks_needed[nr_sides];
  block_requirement_type block_requirement[maxsquare+4];
  last_found_trouble_square_status_type last_found_trouble_square_status;
} immobilisation_state_type;

static immobilisation_state_type const null_immobilisation_state;

static void update_block_requirements(immobilisation_state_type *state)
{
  switch (state->block_requirement[state->last_found_trouble_square])
  {
    case no_block_needed_on_square:
      if (pjoue[nbply]==pn)
      {
        state->block_requirement[state->last_found_trouble_square] = white_block_sufficient_on_square;
        ++state->nr_blocks_needed[White];
      }
      else
      {
        state->block_requirement[state->last_found_trouble_square] = black_block_needed_on_square;
        ++state->nr_blocks_needed[Black];
      }
      break;

    case white_block_sufficient_on_square:
      if (pjoue[nbply]!=pn)
      {
        state->block_requirement[state->last_found_trouble_square] = black_block_needed_on_square;
        --state->nr_blocks_needed[White];
        ++state->nr_blocks_needed[Black];
      }
      break;

    case black_block_needed_on_square:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }
}

static immobilisation_state_type * current_immobilisation_state;

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type intelligent_immobilisation_counter_can_help(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_immobilisation_state->position_of_trouble_maker = move_generation_stack[nbcou].departure;
  switch (e[move_generation_stack[nbcou].arrival])
  {
    case roin: /* unpinnable leaper */
      current_immobilisation_state->last_found_trouble_square = move_generation_stack[nbcou].arrival;
      current_immobilisation_state->last_found_trouble_square_status = pprise[nbply]==vide ? king_block_required : immobilisation_impossible;
      break;

    case cn: /* pinnable leaper */
      current_immobilisation_state->last_found_trouble_square = move_generation_stack[nbcou].arrival;
      current_immobilisation_state->last_found_trouble_square_status = pprise[nbply]==vide ? block_required : pin_required;
      break;

    case dn: /* unpinnable rider */
    {
      int const diff = (move_generation_stack[nbcou].arrival
                        -move_generation_stack[nbcou].departure);
      current_immobilisation_state->last_found_trouble_square = (move_generation_stack[nbcou].departure
                                                                 +CheckDirQueen[diff]);
      if (move_generation_stack[nbcou].arrival==current_immobilisation_state->last_found_trouble_square
          && pprise[nbply]!=vide)
        current_immobilisation_state->last_found_trouble_square_status = immobilisation_impossible;
      else
        current_immobilisation_state->last_found_trouble_square_status = block_required;
      break;
    }

    case tn:
    case fn:
    case pn: /* pinnable riders */
    {
      int const diff = (move_generation_stack[nbcou].arrival
                        -move_generation_stack[nbcou].departure);
      current_immobilisation_state->last_found_trouble_square = (move_generation_stack[nbcou].departure
                                                                 +CheckDirQueen[diff]);
      if (move_generation_stack[nbcou].arrival==current_immobilisation_state->last_found_trouble_square
          && pprise[nbply]!=vide)
        current_immobilisation_state->last_found_trouble_square_status = pin_required;
      else
        current_immobilisation_state->last_found_trouble_square_status = block_required;
      break;
    }

    default:  /* no support for fairy chess */
      assert(0);
      break;
  }

  update_block_requirements(current_immobilisation_state);

  if (current_immobilisation_state->last_found_trouble_square_status<king_block_required)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void stalemate_block_unpromoted_white_pawn(unsigned int nr_remaining_black_moves,
                                                  unsigned int nr_remaining_white_moves,
                                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                                  stip_length_type n,
                                                  unsigned int blocker_index,
                                                  square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,pb))
  {
    square const blocks_from = white[blocker_index].square;
    unsigned int const nr_captures_required = abs(blocks_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                          blocks_from,
                                                                          to_be_blocked);
    if (max_nr_allowed_captures_by_white_pieces>=nr_captures_required
        && time<=nr_remaining_white_moves)
    {
      SetPiece(pb,to_be_blocked,white[blocker_index].flags);
      stalemate_test_target_position(nr_remaining_black_moves,
                                     nr_remaining_white_moves-time,
                                     max_nr_allowed_captures_by_black_pieces,
                                     max_nr_allowed_captures_by_white_pieces-nr_captures_required,
                                     n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_white_promotion(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n,
                                            unsigned int blocker_index,
                                            square to_be_blocked)
{
  unsigned int const nr_moves_required = (to_be_blocked<=square_h7
                                          ? moves_to_white_prom[blocker_index]+1
                                          : moves_to_white_prom[blocker_index]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_moves_required);
  if (nr_remaining_white_moves>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!uninterceptably_attacks_king(Black,to_be_blocked,pp))
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(white[blocker_index].square,
                                                                           pp,
                                                                           to_be_blocked);
        if (time<=nr_remaining_white_moves)
        {
          unsigned int const nr_checks_to_white = 0;
          SetPiece(pp,to_be_blocked,white[blocker_index].flags);
          stalemate_continue_after_block(nr_remaining_black_moves,
                                         nr_remaining_white_moves-time,
                                         max_nr_allowed_captures_by_black_pieces,
                                         max_nr_allowed_captures_by_white_pieces,
                                         n,
                                         Black,
                                         to_be_blocked,
                                         pp,
                                         nr_checks_to_white);
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_white_king(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!are_kings_too_close(to_be_blocked)
      && !is_white_king_uninterceptably_attacked_by_non_king(to_be_blocked))
  {
    unsigned int const time = count_nr_of_moves_from_to_king(roib,
                                                             white[index_of_king].square,
                                                             to_be_blocked);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(roib,to_be_blocked,white[index_of_king].flags);
      king_square[White] = to_be_blocked;

      if (is_white_king_interceptably_attacked())
      {
        unsigned int const nr_of_checks_to_black = 0;
        stalemate_intercept_checks(nr_remaining_black_moves,
                                   nr_remaining_white_moves-time,
                                   max_nr_allowed_captures_by_black_pieces,
                                   max_nr_allowed_captures_by_white_pieces,
                                   n,
                                   nr_of_checks_to_black,
                                   White);
      }
      else
        stalemate_test_target_position(nr_remaining_black_moves,
                                       nr_remaining_white_moves-time,
                                       max_nr_allowed_captures_by_black_pieces,
                                       max_nr_allowed_captures_by_white_pieces,
                                       n);

      king_square[White] = initsquare;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_white_officer(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          piece blocker_type,
                                          unsigned int blocker_index,
                                          square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,blocker_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                 white[blocker_index].square,
                                                                 blocker_type,
                                                                 to_be_blocked);
    if (time<=nr_remaining_white_moves)
    {
      unsigned int const nr_checks_to_white = 0;
      SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
      stalemate_continue_after_block(nr_remaining_black_moves,
                                     nr_remaining_white_moves-time,
                                     max_nr_allowed_captures_by_black_pieces,
                                     max_nr_allowed_captures_by_white_pieces,
                                     n,
                                     Black,
                                     to_be_blocked,
                                     blocker_type,
                                     nr_checks_to_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_white_block(unsigned int nr_remaining_black_moves,
                                  unsigned int nr_remaining_white_moves,
                                  unsigned int max_nr_allowed_captures_by_black_pieces,
                                  unsigned int max_nr_allowed_captures_by_white_pieces,
                                  stip_length_type n,
                                  square to_be_blocked)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_blocks;
    stalemate_block_white_king(nr_remaining_black_moves,
                               nr_remaining_white_moves,
                               max_nr_allowed_captures_by_black_pieces,
                               max_nr_allowed_captures_by_white_pieces,
                               n,
                               to_be_blocked);
    white[index_of_king].usage = piece_is_unused;
  }

  if (max_nr_allowed_captures_by_black_pieces>=1)
  {
    --max_nr_allowed_captures_by_black_pieces;

    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        piece const blocker_type = white[blocker_index].type;

        white[blocker_index].usage = piece_blocks;

        if (blocker_type==pb)
        {
          stalemate_block_white_promotion(nr_remaining_black_moves,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          blocker_index,
                                          to_be_blocked);
          stalemate_block_unpromoted_white_pawn(nr_remaining_black_moves,
                                                nr_remaining_white_moves,
                                                max_nr_allowed_captures_by_black_pieces,
                                                max_nr_allowed_captures_by_white_pieces,
                                                n,
                                                blocker_index,
                                                to_be_blocked);
        }
        else
          stalemate_block_white_officer(nr_remaining_black_moves,
                                        nr_remaining_white_moves,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        blocker_type,
                                        blocker_index,
                                        to_be_blocked);

        white[blocker_index].usage = piece_is_unused;
      }
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_immobilise_black(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n)
{
  immobilisation_state_type immobilisation_state = null_immobilisation_state;

  if (max_nr_solutions_found_in_phase())
    return;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_immobilisation_state = &immobilisation_state;
  slice_has_solution(slices[temporary_hack_intelligent_immobilisation_tester[Black]].u.fork.fork);
  current_immobilisation_state = 0;

  assert(immobilisation_state.last_found_trouble_square_status>no_requirement);
  assert(immobilisation_state.position_of_trouble_maker!=initsquare);

  if (immobilisation_state.last_found_trouble_square_status<immobilisation_impossible)
  {
    if (immobilisation_state.last_found_trouble_square_status!=king_block_required
        && can_white_pin(nr_remaining_white_moves))
      stalemate_immobilise_by_pin(nr_remaining_black_moves,
                                  nr_remaining_white_moves,
                                  max_nr_allowed_captures_by_black_pieces,
                                  max_nr_allowed_captures_by_white_pieces,
                                  n,
                                  immobilisation_state.position_of_trouble_maker);

    if (immobilisation_state.last_found_trouble_square_status<pin_required
        && can_we_block_all_necessary_squares(immobilisation_state.nr_blocks_needed))
    {
      square const most_expensive_square_to_be_blocked_by_black
        = find_most_expensive_square_to_be_blocked_by_black(nr_remaining_black_moves,
                                                            immobilisation_state.block_requirement);
      switch (most_expensive_square_to_be_blocked_by_black)
      {
        case nullsquare:
          /* Black doesn't have time to provide all required blocks */
          break;

        case initsquare:
          assert(immobilisation_state.block_requirement[immobilisation_state.last_found_trouble_square]
                 ==white_block_sufficient_on_square);
        {
          /* All required blocks can equally well be provided by White or Black,
           * i.e. they all concern black pawns!
           * We could now try to find the most expensive one, but we assume that
           * there isn't much difference; so simply pick
           * immobilisation_state.last_found_trouble_square.
           */
          stalemate_black_block(nr_remaining_black_moves,
                                nr_remaining_white_moves,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n,
                                immobilisation_state.last_found_trouble_square);
          stalemate_white_block(nr_remaining_black_moves,
                                nr_remaining_white_moves,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n,
                                immobilisation_state.last_found_trouble_square);
          break;
        }

        default:
        {
          /* most_expensive_square_to_be_blocked_by_black is the most expensive
           * square among those that Black must block */
          stalemate_black_block(nr_remaining_black_moves,
                                nr_remaining_white_moves,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n,
                                most_expensive_square_to_be_blocked_by_black);
          break;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_next_check(unsigned int nr_remaining_black_moves,
                                           unsigned int nr_remaining_white_moves,
                                           unsigned int max_nr_allowed_captures_by_black_pieces,
                                           unsigned int max_nr_allowed_captures_by_white_pieces,
                                           stip_length_type n,
                                           Side side,
                                           int const check_directions[8],
                                           unsigned int nr_of_check_directions,
                                           unsigned int nr_checks_to_opponent)
{
  square to_be_blocked;
  int const current_dir = check_directions[nr_of_check_directions-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (to_be_blocked = king_square[side]+current_dir;
       e[to_be_blocked]==vide;
       to_be_blocked += current_dir)
  {
    stalemate_intercept_check_black(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    side,
                                    to_be_blocked,
                                    check_directions,
                                    nr_of_check_directions-1,
                                    nr_checks_to_opponent);
    stalemate_intercept_check_white(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    side,
                                    to_be_blocked,
                                    check_directions,
                                    nr_of_check_directions-1,
                                    nr_checks_to_opponent);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_continue_intercepting_checks(unsigned int nr_remaining_black_moves,
                                                   unsigned int nr_remaining_white_moves,
                                                   unsigned int max_nr_allowed_captures_by_black_pieces,
                                                   unsigned int max_nr_allowed_captures_by_white_pieces,
                                                   stip_length_type n,
                                                   int const check_directions[8],
                                                   unsigned int nr_of_check_directions,
                                                   unsigned int nr_checks_to_opponent,
                                                   Side side)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (nr_of_check_directions==0)
  {
    Side const opponent = advers(side);
    if (nr_checks_to_opponent>0)
    {
      unsigned int nr_checks_to_black = 0;
      stalemate_intercept_checks(nr_remaining_black_moves,
                                 nr_remaining_white_moves,
                                 max_nr_allowed_captures_by_black_pieces,
                                 max_nr_allowed_captures_by_white_pieces,
                                 n,
                                 nr_checks_to_black,
                                 opponent);
    }
    else
      stalemate_test_target_position(nr_remaining_black_moves,
                                     nr_remaining_white_moves,
                                     max_nr_allowed_captures_by_black_pieces,
                                     max_nr_allowed_captures_by_white_pieces,
                                     n);
  }
  else
    stalemate_intercept_next_check(nr_remaining_black_moves,
                                   nr_remaining_white_moves,
                                   max_nr_allowed_captures_by_black_pieces,
                                   max_nr_allowed_captures_by_white_pieces,
                                   n,
                                   side,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_intercept_checks(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       unsigned int nr_checks_to_opponent,
                                       Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  {
    int check_directions[8];
    unsigned int const nr_of_check_directions = find_check_directions(side,
                                                                      check_directions);

    stalemate_continue_intercepting_checks(nr_remaining_black_moves,
                                           nr_remaining_white_moves,
                                           max_nr_allowed_captures_by_black_pieces,
                                           max_nr_allowed_captures_by_white_pieces,
                                           n,
                                           check_directions,
                                           nr_of_check_directions,
                                           nr_checks_to_opponent,
                                           side);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_continue_after_block(unsigned int nr_remaining_black_moves,
                                      unsigned int nr_remaining_white_moves,
                                      unsigned int max_nr_allowed_captures_by_black_pieces,
                                      unsigned int max_nr_allowed_captures_by_white_pieces,
                                      stip_length_type n,
                                      unsigned int nr_of_checks_to_white)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_checks_to_white);
  TraceFunctionParamListEnd();

  if (nr_of_checks_to_white>0)
    mate_intercept_checks(nr_remaining_black_moves,
                          nr_remaining_white_moves,
                          max_nr_allowed_captures_by_black_pieces,
                          max_nr_allowed_captures_by_white_pieces,
                          n);
  else
    mate_test_target_position(nr_remaining_black_moves,
                              nr_remaining_white_moves,
                              max_nr_allowed_captures_by_black_pieces,
                              max_nr_allowed_captures_by_white_pieces,
                              n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_promoted_black_pawn_on(unsigned int nr_remaining_black_moves,
                                              unsigned int nr_remaining_white_moves,
                                              unsigned int max_nr_allowed_captures_by_black_pieces,
                                              unsigned int max_nr_allowed_captures_by_white_pieces,
                                              stip_length_type n,
                                              unsigned int placed_index,
                                              square placed_on)
{
  square const placed_from = black[placed_index].square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = (placed_from>=square_a7
                         ? 5
                         : placed_from/onerow - nr_of_slack_rows_below_board);
    assert(time<=5);

    if (placed_on>=square_a2)
      /* square is not on 1st rank -- 1 move necessary to get there */
      ++time;

    if (time<=nr_remaining_black_moves)
    {
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
        if (!uninterceptably_attacks_king(White,placed_on,pp))
        {
          unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                             pp,
                                                                             placed_on);
          if (time<=nr_remaining_black_moves)
          {
            unsigned int diffcol = 0;
            if (pp==fn)
            {
              unsigned int const placed_from_file = placed_from%nr_files_on_board;
              square const promotion_square_on_same_file = square_a1+placed_from_file;
              if (SquareCol(placed_on)!=SquareCol(promotion_square_on_same_file))
                diffcol = 1;
            }

            if (diffcol<=max_nr_allowed_captures_by_black_pieces)
            {
              unsigned int const nr_of_checks_to_white = guards(king_square[White],
                                                                pp,
                                                                placed_on);
              SetPiece(pp,placed_on,black[placed_index].flags);
              mate_continue_after_block(nr_remaining_black_moves-time,
                                        nr_remaining_white_moves,
                                        max_nr_allowed_captures_by_black_pieces-diffcol,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        nr_of_checks_to_white);
            }
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_unpromoted_black_pawn_on(unsigned int nr_remaining_black_moves,
                                                unsigned int nr_remaining_white_moves,
                                                unsigned int max_nr_allowed_captures_by_black_pieces,
                                                unsigned int max_nr_allowed_captures_by_white_pieces,
                                                stip_length_type n,
                                                unsigned int placed_index,
                                                square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,placed_on,pn))
  {
    square const placed_from = black[placed_index].square;
    unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);
    if (diffcol<=max_nr_allowed_captures_by_black_pieces)
    {
      unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                            placed_from,
                                                                            placed_on);
      if (time<=nr_remaining_black_moves)
      {
        SetPiece(pn,placed_on,black[placed_index].flags);
        mate_test_target_position(nr_remaining_black_moves-time,
                                  nr_remaining_white_moves,
                                  max_nr_allowed_captures_by_black_pieces-diffcol,
                                  max_nr_allowed_captures_by_white_pieces,
                                  n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_black_officer_on(unsigned int nr_remaining_black_moves,
                                        unsigned int nr_remaining_white_moves,
                                        unsigned int max_nr_allowed_captures_by_black_pieces,
                                        unsigned int max_nr_allowed_captures_by_white_pieces,
                                        stip_length_type n,
                                        unsigned int placed_index,
                                        piece placed_type, square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,placed_on,placed_type))
  {
    square const placed_from = black[placed_index].square;
    unsigned int const time = count_nr_of_moves_from_to_no_check(placed_type,
                                                                 placed_from,
                                                                 placed_type,
                                                                 placed_on);
    if (time<=nr_remaining_black_moves)
    {
      unsigned int const nr_of_checks_to_white = guards(king_square[White],
                                                        placed_type,
                                                        placed_on);
      SetPiece(placed_type,placed_on,black[placed_index].flags);
      mate_continue_after_block(nr_remaining_black_moves-time,
                                nr_remaining_white_moves,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n,
                                nr_of_checks_to_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_any_black_piece_on(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          square placed_on)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
    if (black[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = black[placed_index].type;

      black[placed_index].usage = piece_intercepts;

      if (placed_type==pn)
      {
        if (placed_on>=square_a2)
          mate_place_unpromoted_black_pawn_on(nr_remaining_black_moves,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n,
                                              placed_index,placed_on);
        mate_place_promoted_black_pawn_on(nr_remaining_black_moves,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          placed_index,placed_on);
      }
      else
        mate_place_black_officer_on(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    placed_index,placed_type,placed_on);

      black[placed_index].usage = piece_is_unused;
    }

  e[placed_on] = vide;
  spec[placed_on] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_unpromoted_white_pawn_on(unsigned int nr_remaining_black_moves,
                                                unsigned int nr_remaining_white_moves,
                                                unsigned int max_nr_allowed_captures_by_black_pieces,
                                                unsigned int max_nr_allowed_captures_by_white_pieces,
                                                stip_length_type n,
                                                unsigned int placed_index,
                                                square placed_on)
{
  square const placed_from = white[placed_index].square;
  unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (diffcol<=max_nr_allowed_captures_by_white_pieces
      && !(is_initial_check_uninterceptable
           && uninterceptably_attacks_king(Black,placed_on,pb)))
  {
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                          placed_from,
                                                                          placed_on);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(pb,placed_on,white[placed_index].flags);
      mate_test_target_position(nr_remaining_black_moves,
                                nr_remaining_white_moves-time,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces-diffcol,
                                n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_promoted_white_pawn_on(unsigned int nr_remaining_black_moves,
                                              unsigned int nr_remaining_white_moves,
                                              unsigned int max_nr_allowed_captures_by_black_pieces,
                                              unsigned int max_nr_allowed_captures_by_white_pieces,
                                              stip_length_type n,
                                              unsigned int placed_index,
                                              square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = moves_to_white_prom[placed_index];
    if (placed_on<=square_h7)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++time;

    if (time<=nr_remaining_white_moves)
    {
      square const placed_from = white[placed_index].square;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!(is_initial_check_uninterceptable
              && uninterceptably_attacks_king(Black,placed_on,pp)))
        {
          unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                             pp,
                                                                             placed_on);
          unsigned int diffcol;
          if (pp==fb && SquareCol(placed_on)==SquareCol(placed_from%onerow))
            diffcol= 1;
          else
            diffcol= 0;
          TracePiece(pp);
          TraceValue("%u",diffcol);
          TraceValue("%u\n",time);

          if (diffcol<=max_nr_allowed_captures_by_white_pieces
              && time<=nr_remaining_white_moves)
          {
            SetPiece(pp,placed_on,white[placed_index].flags);
            mate_test_target_position(nr_remaining_black_moves,
                                      nr_remaining_white_moves-time,
                                      max_nr_allowed_captures_by_black_pieces,
                                      max_nr_allowed_captures_by_white_pieces-diffcol,
                                      n);
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_white_officer_on(unsigned int nr_remaining_black_moves,
                                        unsigned int nr_remaining_white_moves,
                                        unsigned int max_nr_allowed_captures_by_black_pieces,
                                        unsigned int max_nr_allowed_captures_by_white_pieces,
                                        stip_length_type n,
                                        unsigned int placed_index,
                                        piece placed_type, square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!(is_initial_check_uninterceptable
        && uninterceptably_attacks_king(Black,placed_on,placed_type)))
  {
    square const placed_from = white[placed_index].square;
    unsigned int const time= count_nr_of_moves_from_to_no_check(placed_type,
                                                                placed_from,
                                                                placed_type,
                                                                placed_on);
    if (time<=nr_remaining_white_moves)
    {
      Flags const placed_flags = white[placed_index].flags;
      SetPiece(placed_type,placed_on,placed_flags);
      mate_test_target_position(nr_remaining_black_moves,
                                nr_remaining_white_moves-time,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_any_white_piece_on(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          square placed_on)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
    if (white[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = white[placed_index].type;

      white[placed_index].usage = piece_intercepts;

      if (placed_type==pb)
      {
        if (placed_on<=square_h7)
          mate_place_unpromoted_white_pawn_on(nr_remaining_black_moves,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n,
                                              placed_index,placed_on);
        mate_place_promoted_white_pawn_on(nr_remaining_black_moves,
                                          nr_remaining_white_moves,
                                          max_nr_allowed_captures_by_black_pieces,
                                          max_nr_allowed_captures_by_white_pieces,
                                          n,
                                          placed_index,placed_on);
      }
      else
        mate_place_white_officer_on(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    placed_index,placed_type,placed_on);

      white[placed_index].usage = piece_is_unused;
    }

  e[placed_on]= vide;
  spec[placed_on]= EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_neutralise_guarding_pieces(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n)
{
  square trouble = initsquare;
  square trto = initsquare;
#if !defined(NDEBUG)
  has_solution_type search_result;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  init_legal_move_finder();

#if !defined(NDEBUG)
  search_result =
#endif
  slice_has_solution(slices[temporary_hack_legal_move_finder[Black]].u.fork.fork);
  assert(search_result==has_solution);
  assert(legal_move_finder_departure!=initsquare);
  trouble = legal_move_finder_departure;
  trto = legal_move_finder_arrival;

  fini_legal_move_finder();

  mate_pin_black_piece(nr_remaining_black_moves,
                       nr_remaining_white_moves,
                       max_nr_allowed_captures_by_black_pieces,
                       max_nr_allowed_captures_by_white_pieces,
                       n,
                       trouble);

  if (is_rider(abs(e[trouble])))
  {
    int const dir = CheckDirQueen[trto-trouble];

    square sq;
    for (sq = trouble+dir; sq!=trto; sq+=dir)
    {
      mate_place_any_black_piece_on(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    sq);
      mate_place_any_white_piece_on(nr_remaining_black_moves,
                                    nr_remaining_white_moves,
                                    max_nr_allowed_captures_by_black_pieces,
                                    max_nr_allowed_captures_by_white_pieces,
                                    n,
                                    sq);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int count_nr_black_moves_to_square_with_promoted_pawn(square pawn_comes_from,
                                                                      square to_be_blocked,
                                                                      unsigned int nr_remaining_black_moves)
{
  unsigned int result = maxply+1;

  TraceFunctionEntry(__func__);
  TraceSquare(pawn_comes_from);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int moves = (pawn_comes_from>=square_a7
                          ? 5
                          : pawn_comes_from/onerow - nr_of_slack_rows_below_board);
    assert(moves<=5);

    if (to_be_blocked>=square_a2)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++moves;

    if (nr_remaining_black_moves>=moves)
    {
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(pawn_comes_from,
                                                                           pp,
                                                                           to_be_blocked);
        if (time<result)
          result = time;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_black_moves_to_square(square to_be_blocked,
                                                   unsigned int nr_remaining_black_moves)
{
  unsigned int result = maxply+1;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
  {
    piece const blocker_type = black[i].type;
    square const blocker_comes_from = black[i].square;

    if (blocker_type==pn)
    {
      if (to_be_blocked>=square_a2)
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                              blocker_comes_from,
                                                                              to_be_blocked);
        if (time<result)
          result = time;
      }

      {
        unsigned int const time_prom = count_nr_black_moves_to_square_with_promoted_pawn(blocker_comes_from,
                                                                                         to_be_blocked,
                                                                                         nr_remaining_black_moves);
        if (time_prom<result)
          result = time_prom;
      }
    }
    else
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                   blocker_comes_from,
                                                                   blocker_type,
                                                                   to_be_blocked);
      if (time<result)
        result = time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void finalise_blocking(unsigned int nr_remaining_black_moves,
                              unsigned int nr_remaining_white_moves,
                              unsigned int max_nr_allowed_captures_by_black_pieces,
                              unsigned int max_nr_allowed_captures_by_white_pieces,
                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const is_white_in_check = echecc(nbply,White);
    unsigned int const is_black_in_check = echecc(nbply,Black);
    if (goal_to_be_reached==goal_stale)
    {
      if (is_black_in_check)
        stalemate_intercept_checks(nr_remaining_black_moves,
                                   nr_remaining_white_moves,
                                   max_nr_allowed_captures_by_black_pieces,
                                   max_nr_allowed_captures_by_white_pieces,
                                   n,
                                   is_white_in_check,
                                   Black);
      else if (is_white_in_check)
        stalemate_intercept_checks(nr_remaining_black_moves,
                                   nr_remaining_white_moves,
                                   max_nr_allowed_captures_by_black_pieces,
                                   max_nr_allowed_captures_by_white_pieces,
                                   n,
                                   is_black_in_check,
                                   White);
      else
        stalemate_test_target_position(nr_remaining_black_moves,
                                       nr_remaining_white_moves,
                                       max_nr_allowed_captures_by_black_pieces,
                                       max_nr_allowed_captures_by_white_pieces,
                                       n);
    }
    else if (is_black_in_check)
      mate_continue_after_block(nr_remaining_black_moves,
                                nr_remaining_white_moves,
                                max_nr_allowed_captures_by_black_pieces,
                                max_nr_allowed_captures_by_white_pieces,
                                n,
                                is_white_in_check);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_flights(unsigned int nr_remaining_black_moves,
                          unsigned int nr_remaining_white_moves,
                          unsigned int max_nr_allowed_captures_by_black_pieces,
                          unsigned int max_nr_allowed_captures_by_white_pieces,
                          stip_length_type n,
                          unsigned int nr_flights_to_block);

static void block_one_flight_officer(unsigned int nr_remaining_black_moves,
                                     unsigned int nr_remaining_white_moves,
                                     unsigned int max_nr_allowed_captures_by_black_pieces,
                                     unsigned int max_nr_allowed_captures_by_white_pieces,
                                     stip_length_type n,
                                     square to_be_blocked,
                                     piece blocker_type,
                                     Flags blocker_flags,
                                     square blocks_from,
                                     unsigned int nr_moves_needed,
                                     unsigned int nr_remaining_flights_to_block)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",nr_moves_needed);
  TraceFunctionParam("%u",nr_remaining_flights_to_block);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,blocker_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,blocks_from,blocker_type,to_be_blocked);
    TraceValue("%u\n",nr_moves_needed);
    if (time>=nr_moves_needed)
    {
      unsigned int const wasted = time-nr_moves_needed;
      if (wasted<=nr_remaining_black_moves)
      {
        SetPiece(blocker_type,to_be_blocked,blocker_flags);
        block_flights(nr_remaining_black_moves-wasted,nr_remaining_white_moves,
                      max_nr_allowed_captures_by_black_pieces,
                      max_nr_allowed_captures_by_white_pieces,
                      n,
                      nr_remaining_flights_to_block);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_one_flight_pawn_no_prom(unsigned int nr_remaining_black_moves,
                                          unsigned int nr_remaining_white_moves,
                                          unsigned int max_nr_allowed_captures_by_black_pieces,
                                          unsigned int max_nr_allowed_captures_by_white_pieces,
                                          stip_length_type n,
                                          square to_be_blocked,
                                          Flags blocker_flags,
                                          square blocks_from,
                                          unsigned int nr_moves_needed,
                                          unsigned int nr_remaining_flights_to_block)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",nr_moves_needed);
  TraceFunctionParam("%u",nr_remaining_flights_to_block);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,pn))
  {
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                          blocks_from,
                                                                          to_be_blocked);
    TraceValue("%u\n",nr_moves_needed);
    if (time>=nr_moves_needed)
    {
      unsigned int const wasted = time-nr_moves_needed;
      if (wasted<=nr_remaining_black_moves)
      {
        unsigned int const diffcol = abs(blocks_from%onerow - to_be_blocked%onerow);
        SetPiece(pn,to_be_blocked,blocker_flags);
        if (diffcol<=max_nr_allowed_captures_by_black_pieces)
          block_flights(nr_remaining_black_moves-wasted,
                        nr_remaining_white_moves,
                        max_nr_allowed_captures_by_black_pieces-diffcol,
                        max_nr_allowed_captures_by_white_pieces,
                        n,
                        nr_remaining_flights_to_block);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_one_flight_with_prom(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       square to_be_blocked,
                                       square blocks_from,
                                       Flags blocker_flags,
                                       unsigned int nr_moves_needed,
                                       unsigned int nr_remaining_flights_to_block)
{
  unsigned int nr_moves_guesstimate = blocks_from/onerow - nr_of_slack_rows_below_board;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",nr_moves_needed);
  TraceFunctionParam("%u",nr_remaining_flights_to_block);
  TraceFunctionParamListEnd();

  /* A rough check whether it is worth thinking about promotions */
  if (nr_moves_guesstimate==6)
    --nr_moves_guesstimate; /* double step! */

  if (to_be_blocked>=square_a2)
    /* square is not on 8th rank -- 1 move necessary to get there */
    ++nr_moves_guesstimate;

  TraceValue("%u\n",nr_moves_guesstimate);
  if (nr_remaining_black_moves+nr_moves_needed>=nr_moves_guesstimate)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!uninterceptably_attacks_king(White,to_be_blocked,pp))
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(blocks_from,
                                                                           pp,
                                                                           to_be_blocked);
        if (time>=nr_moves_needed)
        {
          unsigned int const wasted = time-nr_moves_needed;
          unsigned int diffcol = 0;
          if (pp==fn)
          {
            unsigned int const placed_from_file = blocks_from%nr_files_on_board;
            square const promotion_square_on_same_file = square_a1+placed_from_file;
            if (SquareCol(to_be_blocked)!=SquareCol(promotion_square_on_same_file))
              diffcol = 1;
          }
          if (diffcol<=max_nr_allowed_captures_by_black_pieces
              && wasted<=nr_remaining_black_moves)
          {
            SetPiece(pp,to_be_blocked,blocker_flags);
            block_flights(nr_remaining_black_moves-wasted,
                          nr_remaining_white_moves,
                          max_nr_allowed_captures_by_black_pieces-diffcol,
                          max_nr_allowed_captures_by_white_pieces,
                          n,
                          nr_remaining_flights_to_block);
          }
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int nr_king_flights_to_be_blocked;
static struct
{
  square flight;
  unsigned int nr_moves_needed;
} king_flights_to_be_blocked[8];

static void block_last_flight(unsigned int nr_remaining_black_moves,
                              unsigned int nr_remaining_white_moves,
                              unsigned int max_nr_allowed_captures_by_black_pieces,
                              unsigned int max_nr_allowed_captures_by_white_pieces,
                              stip_length_type n,
                              unsigned int nr_flights_to_block)
{
  unsigned int index_of_current_blocker;
  unsigned int const current_flight = nr_flights_to_block-1;
  square const to_be_blocked = king_flights_to_be_blocked[current_flight].flight;
  unsigned int const nr_moves_needed = king_flights_to_be_blocked[current_flight].nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_flights_to_block);
  TraceFunctionParamListEnd();

  TraceSquare(to_be_blocked);TraceText("\n");
  for (index_of_current_blocker = 1;
       index_of_current_blocker<MaxPiece[Black];
       index_of_current_blocker++)
    if (black[index_of_current_blocker].usage==piece_is_unused)
    {
      piece const blocker_type = black[index_of_current_blocker].type;
      square const blocks_from = black[index_of_current_blocker].square;
      Flags const blocker_flags = black[index_of_current_blocker].flags;

      black[index_of_current_blocker].usage = piece_blocks;

      if (blocker_type==pn)
      {
        if (to_be_blocked>=square_a2)
          block_one_flight_pawn_no_prom(nr_remaining_black_moves,
                                        nr_remaining_white_moves,
                                        max_nr_allowed_captures_by_black_pieces,
                                        max_nr_allowed_captures_by_white_pieces,
                                        n,
                                        to_be_blocked,blocker_flags,blocks_from,
                                        nr_moves_needed,
                                        nr_flights_to_block-1);

        block_one_flight_with_prom(nr_remaining_black_moves,
                                   nr_remaining_white_moves,
                                   max_nr_allowed_captures_by_black_pieces,
                                   max_nr_allowed_captures_by_white_pieces,
                                   n,
                                   to_be_blocked,blocks_from,blocker_flags,
                                   nr_moves_needed,
                                   nr_flights_to_block-1);
      }
      else
        block_one_flight_officer(nr_remaining_black_moves,
                                 nr_remaining_white_moves,
                                 max_nr_allowed_captures_by_black_pieces,
                                 max_nr_allowed_captures_by_white_pieces,
                                 n,
                                 to_be_blocked,
                                 blocker_type,blocker_flags,blocks_from,
                                 nr_moves_needed,
                                 nr_flights_to_block-1);

      black[index_of_current_blocker].usage = piece_is_unused;
    }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_flights(unsigned int nr_remaining_black_moves,
                          unsigned int nr_remaining_white_moves,
                          unsigned int max_nr_allowed_captures_by_black_pieces,
                          unsigned int max_nr_allowed_captures_by_white_pieces,
                          stip_length_type n,
                          unsigned int nr_flights_to_block)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",nr_flights_to_block);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase())
  {
    /* nothing */
  }
  else if (nr_flights_to_block==0)
    finalise_blocking(nr_remaining_black_moves,
                      nr_remaining_white_moves,
                      max_nr_allowed_captures_by_black_pieces,
                      max_nr_allowed_captures_by_white_pieces,
                      n);
  else
    block_last_flight(nr_remaining_black_moves,
                      nr_remaining_white_moves,
                      max_nr_allowed_captures_by_black_pieces,
                      max_nr_allowed_captures_by_white_pieces,
                      n,
                      nr_flights_to_block);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type capture_finder_can_help(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type selfcheck_guard_can_help2(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,Black))
    result = n+4;
  else
    result = capture_finder_can_help(next,n);
//    result = can_help(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type stalemate_flight_optimiser_can_help(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (goal_to_be_reached==goal_stale)
  {
    /* we also need to block the flights that are currently guarded by a
     * line piece through the king's square - that line is going to be
     * intercepted in the play, so intercept it here as well */
    e[move_generation_stack[nbcou].departure] = obs;
    result = selfcheck_guard_can_help2(next,n);
//    result = can_help(next,n);
    e[move_generation_stack[nbcou].departure] = vide;
  }
  else
    result = selfcheck_guard_can_help2(next,n);
//    result = can_help(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int nr_available_blockers;

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type flights_to_be_blocked_finder_can_help(slice_index si,
                                                              stip_length_type n)
{
  stip_length_type result;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = stalemate_flight_optimiser_can_help(next,n);
//    result = can_help(next,n);
  switch (result)
  {
    case slack_length_help+4:
      /* this 'flight' is guarded */
      break;

    case slack_length_help+2:
      if (nr_king_flights_to_be_blocked<nr_available_blockers)
      {
        king_flights_to_be_blocked[nr_king_flights_to_be_blocked].flight = move_generation_stack[nbcou].arrival;
        ++nr_king_flights_to_be_blocked;
      }
      else
        /* more blocks are required than there are blockers available */
        result = slack_length_help;
      break;

    case slack_length_help:
      /* this flight is occupied by White and therefore can't be blocked */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int nr_available_blockers;

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type move_can_help2(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = no_slice;
//  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while(encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && flights_to_be_blocked_finder_can_help(next,n-1)==n-1)
    {
      result = n;
      repcoup();
      break;
    }
    else
      repcoup();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
static stip_length_type move_generator_can_help2(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
//  Side const side_at_move = slices[si].starter;
//  slice_index const next = slices[si].u.pipe.next;
  Side const side_at_move = Black;
  slice_index const next = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  genmove(side_at_move);
//  result = can_help(next,n);
  result = move_can_help2(next,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int plan_blocks_of_flights(unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  nr_king_flights_to_be_blocked = 0;

  if (min_nr_captures_by_white<MaxPiece[Black])
  {
    nr_available_blockers = MaxPiece[Black]-1-min_nr_captures_by_white;
    if (move_generator_can_help2(no_slice,slack_length_help+1)
        ==slack_length_help+1)
      /* at least 1 flight was found that can't be blocked */
      nr_king_flights_to_be_blocked = MaxPiece[Black];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",nr_king_flights_to_be_blocked);
  TraceFunctionResultEnd();
  return nr_king_flights_to_be_blocked;
}

static int count_max_nr_allowed_black_pawn_captures(void)
{
  int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; ++i)
    if (white[i].usage==piece_is_unused)
      ++result;


  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_min_nr_black_moves_for_blocks(unsigned int nr_remaining_black_moves,
                                                        unsigned int nr_flights_to_block)
{
  unsigned int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_flights_to_block && result<=nr_remaining_black_moves; ++i)
  {
    unsigned int const time = count_nr_black_moves_to_square(king_flights_to_be_blocked[i].flight,
                                                             nr_remaining_black_moves);
    king_flights_to_be_blocked[i].nr_moves_needed = time;
    result += time;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static void fix_white_king_on_diagram_square(unsigned int nr_remaining_white_moves,
                                             unsigned int nr_remaining_black_moves,
                                             unsigned int min_nr_captures_by_white,
                                             stip_length_type n)
{
  square const king_diagram_square = white[index_of_king].square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide && !are_kings_too_close(king_diagram_square))
  {
    unsigned int const nr_flights_to_block = plan_blocks_of_flights(min_nr_captures_by_white);
    if (min_nr_captures_by_white+nr_flights_to_block<MaxPiece[Black])
    {
      unsigned int const mtba = count_min_nr_black_moves_for_blocks(nr_remaining_black_moves,
                                                                    nr_flights_to_block);
      if (mtba<=nr_remaining_black_moves)
      {
        unsigned int const max_nr_allowed_captures_by_black_pieces = count_max_nr_allowed_black_pawn_captures();

        king_square[White] = king_diagram_square;
        SetPiece(roib,king_square[White],white[index_of_king].flags);
        white[index_of_king].usage = piece_is_fixed_to_diagram_square;
        block_flights(nr_remaining_black_moves-mtba,
                      nr_remaining_white_moves,
                      max_nr_allowed_captures_by_black_pieces,
                      MaxPiece[Black]-1-min_nr_captures_by_white,
                      n,
                      nr_flights_to_block);
        white[index_of_king].usage = piece_is_unused;
        e[king_square[White]] = vide;
        spec[king_square[White]] = EmptySpec;
        king_square[White] = initsquare;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void FinaliseGuarding(unsigned int nr_remaining_white_moves,
                             unsigned int nr_remaining_black_moves,
                             unsigned int min_nr_captures_by_white,
                             stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused
      && white[index_of_king].square!=square_e1
      && nr_remaining_white_moves==0)
    fix_white_king_on_diagram_square(nr_remaining_white_moves,
                                     nr_remaining_black_moves,
                                     min_nr_captures_by_white,
                                     n);
  else
  {
    unsigned int const nr_flights_to_block = plan_blocks_of_flights(min_nr_captures_by_white);

    if (min_nr_captures_by_white+nr_flights_to_block<MaxPiece[Black])
    {
      unsigned int const mtba = count_min_nr_black_moves_for_blocks(nr_remaining_black_moves,
                                                                    nr_flights_to_block);
      if (mtba<=nr_remaining_black_moves)
      {
        unsigned int const max_nr_allowed_captures_by_black_pieces = count_max_nr_allowed_black_pawn_captures();
        unsigned int const max_nr_allowed_captures_by_white_pieces = MaxPiece[Black]-1-min_nr_captures_by_white;
        block_flights(nr_remaining_black_moves-mtba,
                      nr_remaining_white_moves,
                      max_nr_allowed_captures_by_black_pieces,
                      max_nr_allowed_captures_by_white_pieces,
                      n,
                      nr_flights_to_block);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_non_king(unsigned int nr_remaining_white_moves,
                                   unsigned int nr_remaining_black_moves,
                                   stip_length_type n,
                                   unsigned int index_of_first_guarding_piece,
                                   unsigned int min_nr_captures_by_white);

static void guard_flight_unpromoted_pawn(unsigned int nr_remaining_white_moves,
                                         unsigned int nr_remaining_black_moves,
                                         stip_length_type n,
                                         unsigned int index,
                                         unsigned int min_nr_captures_by_white)
{
  Flags const pawn_flags = white[index].flags;
  square const guard_from = white[index].square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide
        && !(is_initial_check_uninterceptable
             && uninterceptably_attacks_king(Black,*bnp,pb)))
    {
      unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                            guard_from,
                                                                            *bnp);
      if (time<=nr_remaining_white_moves
          && guards_black_flight(pb,*bnp)
          && !(index<index_of_designated_piece_delivering_check
               && guards(king_square[Black],pb,*bnp)))
      {
        unsigned int const diffcol = abs(guard_from % onerow - *bnp % onerow);
        SetPiece(pb,*bnp,pawn_flags);
        guard_flights_non_king(nr_remaining_white_moves-time,
                               nr_remaining_black_moves,
                               n,
                               index+1,
                               min_nr_captures_by_white+diffcol);

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flight_promoted_pawn(unsigned int nr_remaining_white_moves,
                                       unsigned int nr_remaining_black_moves,
                                       stip_length_type n,
                                       unsigned int index,
                                       unsigned int min_nr_captures_by_white)
{
  Flags const pawn_flags = white[index].flags;
  square const guard_from = white[index].square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (*bnp<=square_h7
                                              ? moves_to_white_prom[index]+1
                                              : moves_to_white_prom[index]);
      if (nr_remaining_white_moves>=min_nr_moves_by_p)
      {
        piece pp;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
          if (!(is_initial_check_uninterceptable
                && uninterceptably_attacks_king(Black,*bnp,pp)))
          {
            unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(guard_from,
                                                                               pp,
                                                                               *bnp);
            if (time<=nr_remaining_white_moves
                && guards_black_flight(pp,*bnp)
                && !(index<index_of_designated_piece_delivering_check
                     && guards(king_square[Black],pp,*bnp)))
            {
              SetPiece(pp,*bnp,pawn_flags);
              guard_flights_non_king(nr_remaining_white_moves-time,
                                     nr_remaining_black_moves,
                                     n,
                                     index+1,
                                     min_nr_captures_by_white);
            }
          }

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flight_officer(unsigned int nr_remaining_white_moves,
                                 unsigned int nr_remaining_black_moves,
                                 stip_length_type n,
                                 unsigned int index,
                                 piece guard_type,
                                 unsigned int min_nr_captures_by_white)
{
  Flags const guard_flags = white[index].flags;
  square const guard_from = white[index].square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index);
  TracePiece(guard_type);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide
        && !(is_initial_check_uninterceptable
             && uninterceptably_attacks_king(Black,*bnp,guard_type)))
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(guard_type,
                                                                   guard_from,
                                                                   guard_type,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves && guards_black_flight(guard_type,*bnp)
          && !(index<index_of_designated_piece_delivering_check
               && guards(king_square[Black],guard_type,*bnp)))
      {
        SetPiece(guard_type,*bnp,guard_flags);
        guard_flights_non_king(nr_remaining_white_moves-time,
                               nr_remaining_black_moves,
                               n,
                               index+1,
                               min_nr_captures_by_white);

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_non_king(unsigned int nr_remaining_white_moves,
                                   unsigned int nr_remaining_black_moves,
                                   stip_length_type n,
                                   unsigned int index_of_first_guarding_piece,
                                   unsigned int min_nr_captures_by_white)
{
  unsigned int index_of_current_guarding_piece;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",index_of_first_guarding_piece);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  assert(index_of_first_guarding_piece>index_of_king);

  if (!max_nr_solutions_found_in_phase()
      && min_nr_captures_by_white<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
  {
    for (index_of_current_guarding_piece = index_of_first_guarding_piece;
         index_of_current_guarding_piece<MaxPiece[White];
         ++index_of_current_guarding_piece)
      if (index_of_current_guarding_piece!=index_of_designated_piece_delivering_check)
      {
        piece const guard_type = white[index_of_current_guarding_piece].type;
        white[index_of_current_guarding_piece].usage = piece_guards;

        if (guard_type==pb)
        {
          guard_flight_unpromoted_pawn(nr_remaining_white_moves,
                                       nr_remaining_black_moves,
                                       n,
                                       index_of_current_guarding_piece,
                                       min_nr_captures_by_white);
          guard_flight_promoted_pawn(nr_remaining_white_moves,
                                     nr_remaining_black_moves,
                                     n,
                                     index_of_current_guarding_piece,
                                     min_nr_captures_by_white);
        }
        else
          guard_flight_officer(nr_remaining_white_moves,
                               nr_remaining_black_moves,
                               n,
                               index_of_current_guarding_piece,
                               guard_type,
                               min_nr_captures_by_white);

        white[index_of_current_guarding_piece].usage = piece_is_unused;
      }

    if (goal_to_be_reached==goal_stale || echecc(nbply,Black))
      FinaliseGuarding(nr_remaining_white_moves,
                       nr_remaining_black_moves,
                       min_nr_captures_by_white,
                       n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_king(unsigned int nr_remaining_white_moves,
                               unsigned int nr_remaining_black_moves,
                               stip_length_type n,
                               unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  if (!max_nr_solutions_found_in_phase()
      && min_nr_captures_by_white<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
  {
    if (white[index_of_king].usage==piece_is_unused)
    {
      square const guard_from = white[index_of_king].square;
      square const *bnp;

      white[index_of_king].usage = piece_guards;

      /* try using white king for guarding from every square */
      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (e[*bnp]==vide && !are_kings_too_close(*bnp))
        {
          unsigned int const time = count_nr_of_moves_from_to_king(roib,
                                                                   guard_from,
                                                                   *bnp);
          TraceSquare(*bnp);TraceText("\n");
          if (time<=nr_remaining_white_moves && guards_black_flight(roib,*bnp))
          {
            king_square[White]= *bnp;
            SetPiece(roib,*bnp,white[index_of_king].flags);
            guard_flights_non_king(nr_remaining_white_moves-time,
                                   nr_remaining_black_moves,
                                   n,
                                   1,
                                   min_nr_captures_by_white);
            e[*bnp] = vide;
            spec[*bnp] = EmptySpec;
          }
        }

      king_square[White] = initsquare;
      white[index_of_king].usage = piece_is_unused;
    }

    /* try not using white king for guarding */
    guard_flights_non_king(nr_remaining_white_moves,
                           nr_remaining_black_moves,
                           n,
                           1,
                           min_nr_captures_by_white);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move_by_promoted_pawn(unsigned int nr_remaining_white_moves,
                                                         unsigned int nr_remaining_black_moves,
                                                         stip_length_type n)
{
  Flags const checker_flags = white[index_of_designated_piece_delivering_check].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (*bnp<=square_h7
                                              ? moves_to_white_prom[index_of_designated_piece_delivering_check]+1
                                              : moves_to_white_prom[index_of_designated_piece_delivering_check]);
      if (nr_remaining_white_moves>=min_nr_moves_by_p)
      {
        piece pp;
        square const pawn_origin = white[index_of_designated_piece_delivering_check].square;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        {
          unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(pawn_origin,
                                                                             pp,
                                                                             *bnp);
          if (time<=nr_remaining_white_moves
              && guards(king_square[Black],pp,*bnp))
          {
            unsigned int const min_nr_captures_by_white = 0;
            is_initial_check_uninterceptable = uninterceptably_attacks_king(Black,*bnp,pp);
            SetPiece(pp,*bnp,checker_flags);
            guard_flights_king(nr_remaining_white_moves-time,
                               nr_remaining_black_moves,
                               n,
                               min_nr_captures_by_white);
          }
        }
      }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move_by_unpromoted_pawn(unsigned int nr_remaining_white_moves,
                                                           unsigned int nr_remaining_black_moves,
                                                           stip_length_type n)
{
  Flags const checker_flags = white[index_of_designated_piece_delivering_check].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide)
    {
      square const pawn_origin = white[index_of_designated_piece_delivering_check].square;
      unsigned int const time = count_nr_of_moves_from_to_checking(pb,
                                                                   pawn_origin,
                                                                   pb,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves
          && guards(king_square[Black],pb,*bnp))
      {
        unsigned int const min_nr_captures_by_white = abs(pawn_origin%onerow - *bnp%onerow);
        is_initial_check_uninterceptable = uninterceptably_attacks_king(Black,*bnp,pb);
        SetPiece(pb,*bnp,checker_flags);
        guard_flights_king(nr_remaining_white_moves-time,
                           nr_remaining_black_moves,
                           n,
                           min_nr_captures_by_white);
      }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move_by_officer(unsigned int nr_remaining_white_moves,
                                                   unsigned int nr_remaining_black_moves,
                                                   stip_length_type n,
                                                   piece checker_type)
{
  Flags const checker_flags = white[index_of_designated_piece_delivering_check].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TracePiece(checker_type);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide)
    {
      square const checker_origin = white[index_of_designated_piece_delivering_check].square;
      unsigned int const time = count_nr_of_moves_from_to_checking(checker_type,
                                                                   checker_origin,
                                                                   checker_type,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves
          && guards(king_square[Black],checker_type,*bnp))
      {
        unsigned int const min_nr_captures_by_white = 0;
        is_initial_check_uninterceptable = uninterceptably_attacks_king(Black,*bnp,checker_type);
        SetPiece(checker_type,*bnp,checker_flags);
        guard_flights_king(nr_remaining_white_moves-time,
                           nr_remaining_black_moves,
                           n,
                           min_nr_captures_by_white);
      }

      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move(unsigned int nr_remaining_white_moves,
                                        unsigned int nr_remaining_black_moves,
                                        stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (index_of_designated_piece_delivering_check = 1;
       index_of_designated_piece_delivering_check<MaxPiece[White];
       ++index_of_designated_piece_delivering_check)
  {
    piece const checker_type = white[index_of_designated_piece_delivering_check].type;

    TraceValue("%u",index_of_designated_piece_delivering_check);
    TraceSquare(white[index_of_designated_piece_delivering_check].square);
    TracePiece(checker_type);
    TraceText("\n");

    white[index_of_designated_piece_delivering_check].usage = piece_gives_check;

    if (checker_type==pb)
    {
      mate_generate_checking_move_by_unpromoted_pawn(nr_remaining_white_moves,
                                                     nr_remaining_black_moves,
                                                     n);
      mate_generate_checking_move_by_promoted_pawn(nr_remaining_white_moves,
                                                   nr_remaining_black_moves,
                                                   n);
    }
    else
      mate_generate_checking_move_by_officer(nr_remaining_white_moves,
                                             nr_remaining_black_moves,
                                             n,
                                             checker_type);

    white[index_of_designated_piece_delivering_check].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void GenerateBlackKing(stip_length_type n)
{
  Flags const king_flags = black[index_of_king].flags;
  unsigned int const nr_remaining_white_moves = MovesLeft[White];
  unsigned int const nr_remaining_black_moves = MovesLeft[Black];
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(black[index_of_king].type==roin);

  for (bnp = boardnum; *bnp!=initsquare && !hasMaxtimeElapsed(); ++bnp)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]!=obs)
    {
      unsigned int const time = count_nr_of_moves_from_to_king(roin,
                                                               black[index_of_king].square,
                                                               *bnp);
      if (time<=nr_remaining_black_moves)
      {
        SetPiece(roin,*bnp,king_flags);
        king_square[Black] = *bnp;
        black[index_of_king].usage = piece_is_king;
        if (goal_to_be_reached==goal_mate)
          mate_generate_checking_move(nr_remaining_white_moves,
                                      nr_remaining_black_moves-time,
                                      n);
        else
        {
          unsigned int const min_nr_captures_by_white = 0;
          index_of_designated_piece_delivering_check = 0;
          /* prevent uninterceptable checks to the black king: */
          is_initial_check_uninterceptable = true;
          guard_flights_king(nr_remaining_white_moves,
                             nr_remaining_black_moves-time,
                             n,
                             min_nr_captures_by_white);
        }

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int count_moves_to_white_promotion(square from_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceFunctionParamListEnd();

  if (MovesLeft[White]==5
      && from_square<=square_h2
      && (e[from_square+dir_up]>vide || e[from_square+2*dir_up]>vide))
    /* pawn can't reach the promotion square */
    result = maxply+1;
  else
  {
    unsigned int const rank = from_square/onerow - nr_of_slack_rows_below_board;
    result = 7-rank;

    if (result==6)
    {
      --result; /* double step! */

      if (MovesLeft[White]<=6)
      {
        /* immediate double step is required if this pawn is to promote */
        if (e[from_square+dir_up]==pn
            && (e[from_square+dir_left]<=roib
                && e[from_square+dir_right]<=roib))
          /* Black can't immediately get rid of block on 3th row
           * -> no immediate double step possible */
          ++result;

        else if (e[from_square+2*dir_up]==pn
                 && (e[from_square+dir_up+dir_left]<=roib
                     && e[from_square+dir_up+dir_right]<=roib
                     && ep[1]!=from_square+dir_up+dir_left
                     && ep[1]!=from_square+dir_up+dir_right))
          /* Black can't immediately get rid of block on 4th row
           * -> no immediate double step possible */
          ++result;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void IntelligentRegulargoal_types(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  testcastling =
      TSTCASTLINGFLAGMASK(0,White,q_castling&castling_flag[castlings_flags_no_castling])==q_castling
      || TSTCASTLINGFLAGMASK(0,White,k_castling&castling_flag[castlings_flags_no_castling])==k_castling
      || TSTCASTLINGFLAGMASK(0,Black,q_castling&castling_flag[castlings_flags_no_castling])==q_castling
      || TSTCASTLINGFLAGMASK(0,Black,k_castling&castling_flag[castlings_flags_no_castling])==k_castling;

  where_to_start_placing_unused_black_pieces = boardnum;

  assert(castling_supported);
  castling_supported = false;

  save_ep_1 = ep[1];
  save_ep2_1 = ep2[1];

  MaxPiece[Black] = 0;
  MaxPiece[White] = 0;

  black[index_of_king].type= e[king_square[Black]];
  black[index_of_king].flags= spec[king_square[Black]];
  black[index_of_king].square= king_square[Black];
  ++MaxPiece[Black];

  if (king_square[White]==initsquare)
    white[index_of_king].usage = piece_is_missing;
  else
  {
    white[index_of_king].usage = piece_is_unused;
    white[index_of_king].type = e[king_square[White]];
    white[index_of_king].flags = spec[king_square[White]];
    white[index_of_king].square = king_square[White];
    assert(white[index_of_king].type==roib);
  }

  ++MaxPiece[White];

  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (king_square[White]!=*bnp && e[*bnp]>obs)
      {
        white[MaxPiece[White]].type = e[*bnp];
        white[MaxPiece[White]].flags = spec[*bnp];
        white[MaxPiece[White]].square = *bnp;
        white[MaxPiece[White]].usage = piece_is_unused;
        if (e[*bnp]==pb)
          moves_to_white_prom[MaxPiece[White]] = count_moves_to_white_promotion(*bnp);
        ++MaxPiece[White];
      }

    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (king_square[Black]!=*bnp && e[*bnp]<vide)
      {
        black[MaxPiece[Black]].type = e[*bnp];
        black[MaxPiece[Black]].flags = spec[*bnp];
        black[MaxPiece[Black]].square = *bnp;
        black[MaxPiece[Black]].usage = piece_is_unused;
        ++MaxPiece[Black];
      }
  }

  StorePosition();
  ep[1] = initsquare;
  ep[1] = initsquare;

  /* clear board */
  {
    square const *bnp;
    for (bnp= boardnum; *bnp!=initsquare; ++bnp)
      if (e[*bnp] != obs)
      {
        e[*bnp]= vide;
        spec[*bnp]= EmptySpec;
      }
  }

  {
    piece p;
    for (p = roib; p<=fb; ++p)
    {
      nbpiece[p] = 2;
      nbpiece[-p] = 2;
    }
  }

  /* generate final positions */
  GenerateBlackKing(n);

  ResetPosition();

  castling_supported = true;
  ep[1] = save_ep_1;
  ep2[1] = save_ep2_1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void goal_to_be_reached_goal(slice_index si,
                                    stip_structure_traversal *st)
{
  goal_type * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*goal==no_goal);
  *goal = slices[si].u.goal_tester.goal.type;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the variable holding the goal to be reached
 */
static goal_type determine_goal_to_be_reached(slice_index si)
{
  stip_structure_traversal st;
  goal_type result = no_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &goal_to_be_reached_goal);
  stip_structure_traversal_override_single(&st,
                                           STTemporaryHackFork,
                                           &stip_traverse_structure_pipe);
  stip_traverse_structure(si,&st);

  TraceValue("%u",goal_to_be_reached);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise a STGoalReachableGuardFilter slice
 * @return identifier of allocated slice
 */
static slice_index alloc_goalreachable_guard_filter(goal_type goal)
{
  slice_index result;
  slice_type type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_mate:
      type = STGoalReachableGuardFilterMate;
      break;

    case goal_stale:
      type = STGoalReachableGuardFilterStalemate;
      break;

    case goal_proofgame:
    case goal_atob:
      type = proof_make_goal_reachable_type();
      break;

    default:
      assert(0);
      type = no_slice_type;
      break;
  }

  if (type!=no_slice_type)
    result = alloc_pipe(type);
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_mate_help(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,slices[si].starter,"");
  TraceEnumerator(Side,just_moved,"");
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  if (mate_isGoalReachable())
    result = help(slices[si].u.pipe.next,n);
  else
    result = n+2;

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_stalemate_help(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,slices[si].starter,"");
  TraceEnumerator(Side,just_moved,"");
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  if (stalemate_isGoalReachable())
    result = help(slices[si].u.pipe.next,n);
  else
    result = n+2;

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_mate_can_help(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  --MovesLeft[just_moved];

  if (mate_isGoalReachable())
    result = can_help(slices[si].u.pipe.next,n);
  else
    result = n+2;

  ++MovesLeft[just_moved];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_stalemate_can_help(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  --MovesLeft[just_moved];

  if (mate_isGoalReachable())
    result = can_help(slices[si].u.pipe.next,n);
  else
    result = n+2;

  ++MovesLeft[just_moved];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
void goalreachable_guards_inserter_help_move(slice_index si,
                                             stip_structure_traversal *st)
{
  goal_type const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_goalreachable_guard_filter(*goal);
    if (prototype!=no_slice)
      help_branch_insert_slices(si,&prototype,1);

    if (is_max_nr_solutions_per_target_position_limited())
    {
      slice_index const prototype = alloc_intelligent_limit_nr_solutions_per_target_position_slice();
      help_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void
goalreachable_guards_duplicate_avoider_inserter(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.goal_tester.goal.type==goal_mate
      || slices[si].u.goal_tester.goal.type==goal_stale)
  {
    slice_index const prototype = alloc_intelligent_duplicate_avoider_slice();
    leaf_branch_insert_slices(si,&prototype,1);

    if (is_max_nr_solutions_per_target_position_limited())
    {
      slice_index const prototype = alloc_intelligent_nr_solutions_per_target_position_counter_slice();
      leaf_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goalreachable_guards_inserters[] =
{
  { STReadyForHelpMove,          &goalreachable_guards_inserter_help_move         },
  { STGoalReachedTester,         &goalreachable_guards_duplicate_avoider_inserter },
  { STGoalImmobileReachedTester, &stip_traverse_structure_pipe                    },
  { STTemporaryHackFork,         &stip_traverse_structure_pipe                    }
};

enum
{
  nr_goalreachable_guards_inserters = (sizeof goalreachable_guards_inserters
                                       / sizeof goalreachable_guards_inserters[0])
};

/* Instrument stipulation with STgoal_typereachableGuard slices
 * @param si identifies slice where to start
 */
static void stip_insert_goalreachable_guards(slice_index si, goal_type goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(goal!=no_goal);

  stip_structure_traversal_init(&st,&goal);
  stip_structure_traversal_override(&st,
                                    goalreachable_guards_inserters,
                                    nr_goalreachable_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_guards_inserter(slice_index si,
                                        stip_structure_traversal *st)
{
  goal_type const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*goal==goal_proofgame || *goal==goal_atob)
  {
    slice_index const prototype = alloc_intelligent_proof();
    help_branch_insert_slices(si,&prototype,1);
  }
  else
  {
    slice_index const prototype = alloc_intelligent_filter();
    help_branch_insert_slices(si,&prototype,1);
  }

  {
    slice_index const prototype = alloc_intelligent_moves_left_initialiser();
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors intelligent_filters_inserters[] =
{
  { STHelpAdapter,       &intelligent_guards_inserter  },
  { STTemporaryHackFork, &stip_traverse_structure_pipe }
};

enum
{
  nr_intelligent_filters_inserters = (sizeof intelligent_filters_inserters
                                     / sizeof intelligent_filters_inserters[0])
};

/* Instrument stipulation with STgoal_typereachableGuard slices
 * @param si identifies slice where to start
 */
static void stip_insert_intelligent_filters(slice_index si, goal_type goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&goal);
  stip_structure_traversal_override(&st,
                                    intelligent_filters_inserters,
                                    nr_intelligent_filters_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* How well does the stipulation support intelligent mode?
 */
typedef enum
{
  intelligent_not_supported,
  intelligent_not_active_by_default,
  intelligent_active_by_default
} support_for_intelligent_mode;

typedef struct
{
  support_for_intelligent_mode support;
  goal_type goal;
} detector_state_type;

static
void intelligent_mode_support_detector_or(slice_index si,
                                          stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;
  support_for_intelligent_mode support1;
  support_for_intelligent_mode support2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->support!=intelligent_not_supported)
  {
    stip_traverse_structure(slices[si].u.binary.op1,st);
    support1 = state->support;

    stip_traverse_structure(slices[si].u.binary.op2,st);
    support2 = state->support;

    /* enumerators are ordered so that the weakest support has the
     * lowest enumerator etc. */
    assert(intelligent_not_supported<intelligent_not_active_by_default);
    assert(intelligent_not_active_by_default<intelligent_active_by_default);

    state->support = support1<support2 ? support1 : support2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_mode_support_none(slice_index si,
                                          stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->support = intelligent_not_supported;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_mode_support_goal_tester(slice_index si,
                                                 stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;
  goal_type const goal = slices[si].u.goal_tester.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->goal==no_goal)
  {
    switch (goal)
    {
      case goal_mate:
      case goal_stale:
        if (state->support!=intelligent_not_supported)
          state->support = intelligent_not_active_by_default;
        break;

      case goal_proofgame:
      case goal_atob:
        if (state->support!=intelligent_not_supported)
          state->support = intelligent_active_by_default;
        break;

      default:
        state->support = intelligent_not_supported;
        break;
    }

    state->goal = goal;
  }
  else if (state->goal!=goal)
    state->support = intelligent_not_supported;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors intelligent_mode_support_detectors[] =
{
  { STAnd,               &intelligent_mode_support_none        },
  { STOr,                &intelligent_mode_support_detector_or },
  { STCheckZigzagJump,   &intelligent_mode_support_detector_or },
  { STNot,               &intelligent_mode_support_none        },
  { STConstraint,        &intelligent_mode_support_none        },
  { STReadyForDefense,   &intelligent_mode_support_none        },
  { STGoalReachedTester, &intelligent_mode_support_goal_tester },
  { STTemporaryHackFork, &stip_traverse_structure_pipe         }
};

enum
{
  nr_intelligent_mode_support_detectors
  = (sizeof intelligent_mode_support_detectors
     / sizeof intelligent_mode_support_detectors[0])
};

/* Determine whether the stipulation supports intelligent mode, and
 * how much so
 * @param si identifies slice where to start
 * @return degree of support for ingelligent mode by the stipulation
 */
static support_for_intelligent_mode stip_supports_intelligent(slice_index si)
{
  detector_state_type state = { intelligent_not_active_by_default, no_goal };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    intelligent_mode_support_detectors,
                                    nr_intelligent_mode_support_detectors);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.support);
  TraceFunctionResultEnd();
  return state.support;
}

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 *         stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  goal_to_be_reached = no_goal;

  switch (stip_supports_intelligent(si))
  {
    case intelligent_not_supported:
      result = !OptFlag[intelligent];
      break;

    case intelligent_not_active_by_default:
      result = true;
      if (OptFlag[intelligent])
      {
        goal_to_be_reached = determine_goal_to_be_reached(si);
        stip_insert_intelligent_filters(si,goal_to_be_reached);
        stip_insert_goalreachable_guards(si,goal_to_be_reached);
      }
      break;

    case intelligent_active_by_default:
      result = true;
      goal_to_be_reached = determine_goal_to_be_reached(si);
      stip_insert_intelligent_filters(si,goal_to_be_reached);
      stip_insert_goalreachable_guards(si,goal_to_be_reached);
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
