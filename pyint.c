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
#include "optimisations/intelligent/mate/finish.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "optimisations/intelligent/stalemate/white_block.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "options/maxsolutions/maxsolutions.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
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
#define ENUMERATION_MAKESTRINGS
#include "pyenum.h"


static goal_type goal_to_be_reached;

unsigned int MaxPiece[nr_sides];

PIECE white[nr_squares_on_board];
PIECE black[nr_squares_on_board];
static square save_ep_1;
static square save_ep2_1;
unsigned int moves_to_white_prom[nr_squares_on_board];
square const *where_to_start_placing_unused_black_pieces;

static PIECE target_position[MaxPieceId+1];

slice_index current_start_slice = no_slice;

boolean solutions_found;

static boolean testcastling;

static unsigned int MovesRequired[nr_sides][maxply+1];
unsigned int CapturesLeft[maxply+1];

static unsigned int PieceId2index[MaxPieceId+1];


unsigned int nr_reasons_for_staying_empty[maxsquare+4];

void remember_to_keep_rider_line_open(square from, square to,
                                      int dir, int delta)
{
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  for (s = from+dir; s!=to; s+=dir)
  {
    assert(e[s]==vide);
    nr_reasons_for_staying_empty[s] += delta;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_to_keep_rider_guard_line_open(square from, square to,
                                                   piece type, int delta)
{
  int const dir = CheckDirQueen[to-from];
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TracePiece(type);
  TraceFunctionParamListEnd();

  TraceValue("%d\n",dir);

  /* the guard line only needs to be kept open up to the flight closest to
   * from; e.g. reset to to c1 with from:a1 to:e1 king_square[Black]:d2
   */
  for (s = to-dir; s!=from && move_diff_code[abs(king_square[Black]-s)]<=2; s -= dir)
    to = s;

  remember_to_keep_rider_line_open(from,to,dir,delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_to_keep_guard_line_open(square from, square to,
                                             piece type, int delta)
{
  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TracePiece(type);
  TraceFunctionParamListEnd();

  switch (type)
  {
    case Queen:
      if (move_diff_code[abs(king_square[Black]-from)]>10)
        remember_to_keep_rider_guard_line_open(from,to,type,delta);
      else
      {
        /* queen may be guarding on two lines*/
      }
      break;

    case Rook:
      if (move_diff_code[abs(king_square[Black]-from)]>2)
        remember_to_keep_rider_guard_line_open(from,to,type,delta);
      else
      {
        /* rook may be guarding on two lines*/
      }
      break;

    case Bishop:
      remember_to_keep_rider_guard_line_open(from,to,type,delta);
      break;

    case Knight:
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

boolean guards(square to_be_guarded, piece guarding, square guarding_from)
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

/* Determine whether there would be king contact if the white king were placed
 * on a particular square
 * @param white_king_square square where white king would be placed
 * @return true iff there would be king contact if the white king were placed
 *              on white_king_square
 */
static boolean would_there_be_king_contact(square white_king_square)
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

/* Determine whether the white king would guard if it were placed on a
 * particular square
 * @param white_king_square square where white king would be placed
 * @return true iff the white king would guard from white_king_square
 */
boolean would_white_king_guard_from(square white_king_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_diff_code[abs(white_king_square-king_square[Black])]<9;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean uninterceptably_attacks_king(Side side, square from, piece p)
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

boolean is_white_king_interceptably_attacked(void)
{
  return ((*checkfunctions[Bishop])(king_square[White],fn,eval_ortho)
          || (*checkfunctions[Rook])(king_square[White],tn,eval_ortho)
          || (*checkfunctions[Queen])(king_square[White],dn,eval_ortho));
}

boolean is_white_king_uninterceptably_attacked_by_non_king(square s)
{
  return ((*checkfunctions[Pawn])(s,pn,eval_ortho)
          || (*checkfunctions[Knight])(s,cn,eval_ortho)
          || (*checkfunctions[Fers])(s,fn,eval_ortho)
          || (*checkfunctions[Wesir])(s,tn,eval_ortho)
          || (*checkfunctions[ErlKing])(s,dn,eval_ortho));
}

static square guards_black_flight(piece as_piece, square from)
{
  int i;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TracePiece(as_piece);
  TraceSquare(from);
  TraceSquare(king_square[Black]);
  TraceFunctionParamListEnd();

  e[king_square[Black]]= vide;

  for (i = 8; i!=0; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && guards(king_square[Black]+vec[i],as_piece,from))
    {
      result = king_square[Black]+vec[i];
      break;
    }

  e[king_square[Black]]= roin;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int count_nr_of_moves_from_to_pawn_promotion(square from_square,
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

unsigned int count_nr_of_moves_from_to_pawn_no_promotion(piece pawn,
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

unsigned int count_nr_of_moves_from_to_king(piece piece,
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

unsigned int count_nr_of_moves_from_to_no_check(piece from_piece,
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

unsigned int count_nr_of_moves_from_to_checking(piece from_piece,
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
      && target_position[GetPieceId(pprispec[nbply])].diagram_square!=initsquare)
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
          if (target_position[id].diagram_square!=initsquare)
          {
            Side const from_side = from_piece>vide ? White : Black;
            if (from_side==White
                && white[PieceId2index[id]].usage==piece_gives_check
                && MovesLeft[White]>0)
            {
              square const save_king_square = king_square[Black];
              PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
              king_square[Black] = target_position[id_king].diagram_square;
              MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_checking(from_piece,
                                                                                    from_square,
                                                                                    target_position[id].type,
                                                                                    target_position[id].diagram_square);
              king_square[Black] = save_king_square;
            }
            else
              MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_no_check(from_piece,
                                                                                    from_square,
                                                                                    target_position[id].type,
                                                                                    target_position[id].diagram_square);
          }
        }
      }
    }
    else
    {
      PieceIdType const id = GetPieceId(jouespec[nbply]);
      MovesRequired[White][nbply] = MovesRequired[White][nbply-1];
      MovesRequired[Black][nbply] = MovesRequired[Black][nbply-1];

      if (target_position[id].diagram_square!=initsquare)
      {
        unsigned int time_before;
        unsigned int time_now;
        if (trait[nbply]==White
            && white[PieceId2index[id]].usage==piece_gives_check)
        {
          square const save_king_square = king_square[Black];
          PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
          king_square[Black] = target_position[id_king].diagram_square;
          time_before = count_nr_of_moves_from_to_checking(pjoue[nbply],
                                                           move_generation_stack[nbcou].departure,
                                                           target_position[id].type,
                                                           target_position[id].diagram_square);
          king_square[Black] = save_king_square;
        }
        else
          time_before = count_nr_of_moves_from_to_no_check(pjoue[nbply],
                                                           move_generation_stack[nbcou].departure,
                                                           target_position[id].type,
                                                           target_position[id].diagram_square);

        if (trait[nbply]==White
            && white[PieceId2index[id]].usage==piece_gives_check
            && MovesLeft[White]>0)
        {
          square const save_king_square = king_square[Black];
          PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
          king_square[Black] = target_position[id_king].diagram_square;
          time_now = count_nr_of_moves_from_to_checking(e[move_generation_stack[nbcou].arrival],
                                                        move_generation_stack[nbcou].arrival,
                                                        target_position[id].type,
                                                        target_position[id].diagram_square);
          king_square[Black] = save_king_square;
        }
        else
          time_now = count_nr_of_moves_from_to_no_check(e[move_generation_stack[nbcou].arrival],
                                                        move_generation_stack[nbcou].arrival,
                                                        target_position[id].type,
                                                        target_position[id].diagram_square);

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
      && target_position[GetPieceId(pprispec[nbply])].diagram_square!=initsquare)
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
          if (target_position[id].diagram_square!=initsquare)
          {
            Side const from_side = from_piece>vide ? White : Black;
            MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_no_check(from_piece,
                                                                                  from_square,
                                                                                  target_position[id].type,
                                                                                  target_position[id].diagram_square);
          }
        }
      }
    }
    else
    {
      PieceIdType const id = GetPieceId(jouespec[nbply]);
      MovesRequired[White][nbply] = MovesRequired[White][nbply-1];
      MovesRequired[Black][nbply] = MovesRequired[Black][nbply-1];

      if (target_position[id].diagram_square!=initsquare)
      {
        unsigned int const time_before = count_nr_of_moves_from_to_no_check(pjoue[nbply],
                                                                            move_generation_stack[nbcou].departure,
                                                                            target_position[id].type,
                                                                            target_position[id].diagram_square);

        unsigned int const time_now = count_nr_of_moves_from_to_no_check(e[move_generation_stack[nbcou].arrival],
                                                                         move_generation_stack[nbcou].arrival,
                                                                         target_position[id].type,
                                                                         target_position[id].diagram_square);

        TracePiece(pjoue[nbply]);
        TraceSquare(move_generation_stack[nbcou].departure);
        TracePiece(e[move_generation_stack[nbcou].arrival]);
        TraceSquare(move_generation_stack[nbcou].arrival);
        TracePiece(target_position[id].type);
        TraceSquare(target_position[id].diagram_square);
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

void solve_target_position(stip_length_type n)
{
  square const save_king_square[nr_sides] = { king_square[White],
                                              king_square[Black] };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      target_position[id].diagram_square = initsquare;
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
        target_position[id].diagram_square = *bnp;
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
      if (target_position[id].diagram_square != initsquare)
      {
        e[target_position[id].diagram_square] = target_position[id].type;
        spec[target_position[id].diagram_square] = target_position[id].flags;
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

unsigned int find_check_directions(Side side, int check_directions[8])
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

  if (current_immobilisation_state->nr_of_trouble_makers==0
      || move_generation_stack[nbcou].departure
         !=current_immobilisation_state->positions_of_trouble_makers[current_immobilisation_state->nr_of_trouble_makers-1])
  {
    current_immobilisation_state->positions_of_trouble_makers[current_immobilisation_state->nr_of_trouble_makers] = move_generation_stack[nbcou].departure;
    ++current_immobilisation_state->nr_of_trouble_makers;
  }

  switch (e[move_generation_stack[nbcou].arrival])
  {
    case roin: /* unpinnable leaper */
      current_immobilisation_state->last_found_trouble_square = move_generation_stack[nbcou].arrival;
      current_immobilisation_state->last_found_trouble_square_status = pprise[nbply]==vide ? king_block_required : immobilisation_impossible;
      update_block_requirements(current_immobilisation_state);
      break;

    case cn: /* pinnable leaper */
      current_immobilisation_state->last_found_trouble_square = move_generation_stack[nbcou].arrival;
      current_immobilisation_state->last_found_trouble_square_status = pprise[nbply]==vide ? block_required : pin_required;
      update_block_requirements(current_immobilisation_state);
      break;

    case dn: /* unpinnable rider */
    {
      int const diff = (move_generation_stack[nbcou].arrival
                        -move_generation_stack[nbcou].departure);
      current_immobilisation_state->last_found_trouble_square = (move_generation_stack[nbcou].departure
                                                                 +CheckDirQueen[diff]);
      if (move_generation_stack[nbcou].arrival
          ==current_immobilisation_state->last_found_trouble_square)
      {
        if (pprise[nbply]==vide)
          current_immobilisation_state->last_found_trouble_square_status = block_required;
        else
          current_immobilisation_state->last_found_trouble_square_status = immobilisation_impossible;

        update_block_requirements(current_immobilisation_state);
      }
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
      if (move_generation_stack[nbcou].arrival
          ==current_immobilisation_state->last_found_trouble_square)
      {
        if (pprise[nbply]==vide)
          current_immobilisation_state->last_found_trouble_square_status = block_required;
        else
          current_immobilisation_state->last_found_trouble_square_status = pin_required;

        update_block_requirements(current_immobilisation_state);
      }
      break;
    }

    default:  /* no support for fairy chess */
      assert(0);
      break;
  }

  if (current_immobilisation_state->last_found_trouble_square_status<king_block_required)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

unsigned int count_nr_black_moves_to_square(square to_be_blocked,
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
    square const blocker_comes_from = black[i].diagram_square;

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
        intelligent_stalemate_intercept_checks(nr_remaining_black_moves,
                                               nr_remaining_white_moves,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               is_white_in_check,
                                               Black);
      else if (is_white_in_check)
        intelligent_stalemate_intercept_checks(nr_remaining_black_moves,
                                               nr_remaining_white_moves,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               is_black_in_check,
                                               White);
      else
        intelligent_stalemate_test_target_position(nr_remaining_black_moves,
                                                   nr_remaining_white_moves,
                                                   max_nr_allowed_captures_by_black_pieces,
                                                   max_nr_allowed_captures_by_white_pieces,
                                                   n);
    }
    else if (is_black_in_check)
      intelligent_mate_finish(nr_remaining_black_moves,
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
  if (nr_reasons_for_staying_empty[to_be_blocked]==0)
  {
    for (index_of_current_blocker = 1;
         index_of_current_blocker<MaxPiece[Black];
         index_of_current_blocker++)
      if (black[index_of_current_blocker].usage==piece_is_unused)
      {
        piece const blocker_type = black[index_of_current_blocker].type;
        square const blocks_from = black[index_of_current_blocker].diagram_square;
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
  }

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

static void start_blocking_king_flights(unsigned int nr_remaining_white_moves,
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

static void fix_white_king_on_diagram_square(unsigned int nr_remaining_white_moves,
                                             unsigned int nr_remaining_black_moves,
                                             unsigned int min_nr_captures_by_white,
                                             stip_length_type n)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && nr_reasons_for_staying_empty[king_diagram_square]==0
      && !would_white_king_guard_from(king_diagram_square))
  {
    king_square[White] = king_diagram_square;
    SetPiece(roib,king_square[White],white[index_of_king].flags);
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;

    start_blocking_king_flights(nr_remaining_white_moves,
                                nr_remaining_black_moves,
                                min_nr_captures_by_white,
                                n);

    white[index_of_king].usage = piece_is_unused;
    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;
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
      && white[index_of_king].diagram_square!=square_e1
      && nr_remaining_white_moves==0)
    fix_white_king_on_diagram_square(nr_remaining_white_moves,
                                     nr_remaining_black_moves,
                                     min_nr_captures_by_white,
                                     n);
  else
    start_blocking_king_flights(nr_remaining_white_moves,
                                nr_remaining_black_moves,
                                min_nr_captures_by_white,
                                n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_non_king(unsigned int nr_remaining_white_moves,
                                   unsigned int nr_remaining_black_moves,
                                   stip_length_type n,
                                   unsigned int index_of_next_guarding_piece,
                                   unsigned int min_nr_captures_by_white);

static void guard_flight_unpromoted_pawn(unsigned int nr_remaining_white_moves,
                                         unsigned int nr_remaining_black_moves,
                                         stip_length_type n,
                                         unsigned int index,
                                         unsigned int min_nr_captures_by_white)
{
  Flags const pawn_flags = white[index].flags;
  square const starts_from = white[index].diagram_square;
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
        && nr_reasons_for_staying_empty[*bnp]==0
        && !uninterceptably_attacks_king(Black,*bnp,pb))
    {
      unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                            starts_from,
                                                                            *bnp);
      if (time<=nr_remaining_white_moves)
      {
        square const guarded = guards_black_flight(pb,*bnp);
        if (guarded!=initsquare)
        {
          unsigned int const diffcol = abs(starts_from % onerow - *bnp % onerow);
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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_on_guarded_square_officer(unsigned int nr_remaining_white_moves,
                                                      unsigned int nr_remaining_black_moves,
                                                      stip_length_type n,
                                                      unsigned int index_of_next_guarding_piece,
                                                      square to_be_intercepted,
                                                      unsigned int index_of_intercepting_piece,
                                                      unsigned int min_nr_captures_by_white)
{
  piece const intercepter_type = white[index_of_intercepting_piece].type;
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_intercepted,intercepter_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(intercepter_type,
                                                                 intercepter_diagram_square,
                                                                 intercepter_type,
                                                                 to_be_intercepted);
    if (time<=nr_remaining_white_moves
        /* avoid duplicate: if intercepter has already been used as guarding
         * piece, it shouldn't guard now again */
        && !(index_of_intercepting_piece<index_of_next_guarding_piece
             && guards_black_flight(intercepter_type,to_be_intercepted)))
    {
      SetPiece(intercepter_type,to_be_intercepted,intercepter_flags);
      guard_flights_non_king(nr_remaining_white_moves-time,
                             nr_remaining_black_moves,
                             n,
                             index_of_next_guarding_piece,
                             min_nr_captures_by_white);
      e[to_be_intercepted] = vide;
      spec[to_be_intercepted] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_on_guarded_square_promoted_pawn(unsigned int nr_remaining_white_moves,
                                                            unsigned int nr_remaining_black_moves,
                                                            stip_length_type n,
                                                            unsigned int index_of_next_guarding_piece,
                                                            square to_be_intercepted,
                                                            unsigned int index_of_intercepting_piece,
                                                            unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int const min_nr_moves_by_p = (to_be_intercepted<=square_h7
                                            ? moves_to_white_prom[index_of_intercepting_piece]+1
                                            : moves_to_white_prom[index_of_intercepting_piece]);
    if (nr_remaining_white_moves>=min_nr_moves_by_p)
    {
      square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
      Flags const intercepter_flags = white[index_of_intercepting_piece].flags;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!uninterceptably_attacks_king(Black,to_be_intercepted,pp))
        {
          unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(intercepter_diagram_square,
                                                                             pp,
                                                                             to_be_intercepted);
          if (time<=nr_remaining_white_moves
              && !(index_of_intercepting_piece<index_of_next_guarding_piece
                   && guards_black_flight(pp,to_be_intercepted)))
          {
            SetPiece(pp,to_be_intercepted,intercepter_flags);
            guard_flights_non_king(nr_remaining_white_moves-time,
                                   nr_remaining_black_moves,
                                   n,
                                   index_of_next_guarding_piece,
                                   min_nr_captures_by_white);
            e[to_be_intercepted] = vide;
            spec[to_be_intercepted] = EmptySpec;
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_on_guarded_square_unpromoted_pawn(unsigned int nr_remaining_white_moves,
                                                              unsigned int nr_remaining_black_moves,
                                                              stip_length_type n,
                                                              unsigned int index_of_next_guarding_piece,
                                                              square to_be_intercepted,
                                                              unsigned int index_of_intercepting_piece,
                                                              unsigned int min_nr_captures_by_white)
{
  square const intercepter_diagram_square = white[index_of_intercepting_piece].diagram_square;
  Flags const intercepter_flags = white[index_of_intercepting_piece].flags;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",index_of_intercepting_piece);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_intercepted,pb))
  {
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                          intercepter_diagram_square,
                                                                          to_be_intercepted);
    if (time<=nr_remaining_white_moves)
    {
      unsigned int const diffcol = abs(intercepter_diagram_square % onerow
                                       - to_be_intercepted % onerow);
      SetPiece(pb,to_be_intercepted,intercepter_flags);
      guard_flights_non_king(nr_remaining_white_moves-time,
                             nr_remaining_black_moves,
                             n,
                             index_of_next_guarding_piece,
                             min_nr_captures_by_white+diffcol);
      e[to_be_intercepted] = vide;
      spec[to_be_intercepted] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_on_guarded_square(unsigned int nr_remaining_white_moves,
                                              unsigned int nr_remaining_black_moves,
                                              stip_length_type n,
                                              unsigned int index_of_next_guarding_piece,
                                              square to_be_intercepted,
                                              unsigned int min_nr_captures_by_white)
{
  unsigned int index_of_intercepting_piece;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_next_guarding_piece);
  TraceSquare(to_be_intercepted);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (index_of_intercepting_piece = 1;
       index_of_intercepting_piece<MaxPiece[White];
       ++index_of_intercepting_piece)
  {
    TraceValue("%u",index_of_intercepting_piece);
    TraceEnumerator(piece_usage,white[index_of_intercepting_piece].usage,"\n");
    if (white[index_of_intercepting_piece].usage==piece_is_unused)
    {
      piece const guard_type = white[index_of_intercepting_piece].type;
      white[index_of_intercepting_piece].usage = piece_intercepts;

      switch (guard_type)
      {
        case db:
          break;

        case tb:
        case fb:
        case cb:
          intercept_check_on_guarded_square_officer(nr_remaining_white_moves,
                                                    nr_remaining_black_moves,
                                                    n,
                                                    index_of_next_guarding_piece,
                                                    to_be_intercepted,
                                                    index_of_intercepting_piece,
                                                    min_nr_captures_by_white);
          break;

        case pb:
          intercept_check_on_guarded_square_unpromoted_pawn(nr_remaining_white_moves,
                                                            nr_remaining_black_moves,
                                                            n,
                                                            index_of_next_guarding_piece,
                                                            to_be_intercepted,
                                                            index_of_intercepting_piece,
                                                            min_nr_captures_by_white);
          intercept_check_on_guarded_square_promoted_pawn(nr_remaining_white_moves,
                                                          nr_remaining_black_moves,
                                                          n,
                                                          index_of_next_guarding_piece,
                                                          to_be_intercepted,
                                                          index_of_intercepting_piece,
                                                          min_nr_captures_by_white);
          break;

        default:
          assert(0);
          break;
      }

      white[index_of_intercepting_piece].usage = piece_is_unused;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square where_to_intercept_check_from_guard(piece guard_type,
                                                  square guard_from)
{
  int const diff = king_square[Black]-guard_from;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (guard_type)
  {
    case db:
    {
      int const dir = CheckDirQueen[diff];
      /* don't intercept wQc8 guarding the flight b7 (but not b8!) of bKa8 */
      if (diff!=2*dir && rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    case tb:
    {
      int const dir = CheckDirRook[diff];
      if (rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    case fb:
    {
      int const dir = CheckDirBishop[diff];
      if (rider_guards(king_square[Black],guard_from,dir))
        result = king_square[Black]-dir;
      break;
    }

    default:
      assert(0);
      break;
  }


  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static void guard_flight_rider_from(unsigned int nr_remaining_white_moves,
                                    unsigned int nr_remaining_black_moves,
                                    stip_length_type n,
                                    unsigned int index_of_rider,
                                    piece guard_type,
                                    square guard_from,
                                    unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_rider);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  {
    square const guarded = guards_black_flight(guard_type,guard_from);
    if (guarded!=initsquare)
    {
      square const to_be_intercepted = where_to_intercept_check_from_guard(guard_type,guard_from);
      SetPiece(guard_type,guard_from,white[index_of_rider].flags);
      remember_to_keep_guard_line_open(guard_from,guarded,guard_type,+1);
      if (to_be_intercepted==initsquare)
        guard_flights_non_king(nr_remaining_white_moves,
                               nr_remaining_black_moves,
                               n,
                               index_of_rider+1,
                               min_nr_captures_by_white);
      else
        intercept_check_on_guarded_square(nr_remaining_white_moves,
                                          nr_remaining_black_moves,
                                          n,
                                          index_of_rider+1,
                                          to_be_intercepted,
                                          min_nr_captures_by_white);
      remember_to_keep_guard_line_open(guard_from,guarded,guard_type,-1);
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flight_leaper_from(unsigned int nr_remaining_white_moves,
                                     unsigned int nr_remaining_black_moves,
                                     stip_length_type n,
                                     unsigned int index_of_leaper,
                                     piece guard_type,
                                     square guard_from,
                                     unsigned int min_nr_captures_by_white)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_leaper);
  TracePiece(guard_type);
  TraceSquare(guard_from);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  {
    square const guarded = guards_black_flight(guard_type,guard_from);
    if (guarded!=initsquare)
    {
      SetPiece(guard_type,guard_from,white[index_of_leaper].flags);
      guard_flights_non_king(nr_remaining_white_moves,
                             nr_remaining_black_moves,
                             n,
                             index_of_leaper+1,
                             min_nr_captures_by_white);
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flight_promoted_pawn(unsigned int nr_remaining_white_moves,
                                       unsigned int nr_remaining_black_moves,
                                       stip_length_type n,
                                       unsigned int index_of_pawn,
                                       unsigned int min_nr_captures_by_white)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_pawn);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide
        && nr_reasons_for_staying_empty[*bnp]==0)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (*bnp<=square_h7
                                              ? moves_to_white_prom[index_of_pawn]+1
                                              : moves_to_white_prom[index_of_pawn]);
      if (nr_remaining_white_moves>=min_nr_moves_by_p)
      {
        piece pp;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
          if (!uninterceptably_attacks_king(Black,*bnp,pp))
          {
            unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(white[index_of_pawn].diagram_square,
                                                                               pp,
                                                                               *bnp);
            if (time<=nr_remaining_white_moves)
              switch (pp)
              {
                case db:
                case tb:
                case fb:
                  guard_flight_rider_from(nr_remaining_white_moves-time,
                                          nr_remaining_black_moves,
                                          n,
                                          index_of_pawn,
                                          pp,
                                          *bnp,
                                          min_nr_captures_by_white);
                  break;

                case cb:
                  guard_flight_leaper_from(nr_remaining_white_moves-time,
                                           nr_remaining_black_moves,
                                           n,
                                           index_of_pawn,
                                           pp,
                                           *bnp,
                                           min_nr_captures_by_white);
                  break;

                default:
                  assert(0);
                  break;
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

static void guard_flight_rider(unsigned int nr_remaining_white_moves,
                               unsigned int nr_remaining_black_moves,
                               stip_length_type n,
                               unsigned int index_of_rider,
                               piece guard_type,
                               unsigned int min_nr_captures_by_white)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_rider);
  TracePiece(guard_type);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0
        && !uninterceptably_attacks_king(Black,*bnp,guard_type))
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(guard_type,
                                                                   white[index_of_rider].diagram_square,
                                                                   guard_type,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves)
        guard_flight_rider_from(nr_remaining_white_moves-time,
                                nr_remaining_black_moves,
                                n,
                                index_of_rider,
                                guard_type,
                                *bnp,
                                min_nr_captures_by_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flight_leaper(unsigned int nr_remaining_white_moves,
                                unsigned int nr_remaining_black_moves,
                                stip_length_type n,
                                unsigned int index_of_leaper,
                                piece guard_type,
                                unsigned int min_nr_captures_by_white)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",nr_remaining_white_moves);
  TraceValue("%u",nr_remaining_black_moves);
  TraceValue("%u",n);
  TraceValue("%u",index_of_leaper);
  TracePiece(guard_type);
  TraceValue("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0
        && !uninterceptably_attacks_king(Black,*bnp,guard_type))
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(guard_type,
                                                                   white[index_of_leaper].diagram_square,
                                                                   guard_type,
                                                                   *bnp);
      if (time<=nr_remaining_white_moves)
        guard_flight_leaper_from(nr_remaining_white_moves-time,
                                 nr_remaining_black_moves,
                                 n,
                                 index_of_leaper,
                                 guard_type,
                                 *bnp,
                                 min_nr_captures_by_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_non_king(unsigned int nr_remaining_white_moves,
                                   unsigned int nr_remaining_black_moves,
                                   stip_length_type n,
                                   unsigned int index_of_next_guarding_piece,
                                   unsigned int min_nr_captures_by_white)
{
  unsigned int index_of_current_guarding_piece;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",index_of_next_guarding_piece);
  TraceFunctionParam("%u",min_nr_captures_by_white);
  TraceFunctionParamListEnd();

  assert(index_of_next_guarding_piece>index_of_king);

  if (!max_nr_solutions_found_in_phase()
      && min_nr_captures_by_white<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
  {
    TraceValue("%u\n",MaxPiece[White]);
    for (index_of_current_guarding_piece = index_of_next_guarding_piece;
         index_of_current_guarding_piece<MaxPiece[White];
         ++index_of_current_guarding_piece)
    {
      TraceValue("%u",index_of_current_guarding_piece);
      TraceEnumerator(piece_usage,white[index_of_current_guarding_piece].usage,"\n");
      if (white[index_of_current_guarding_piece].usage==piece_is_unused)
      {
        piece const guard_type = white[index_of_current_guarding_piece].type;
        white[index_of_current_guarding_piece].usage = piece_guards;

        switch (guard_type)
        {
          case pb:
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
            break;

          case db:
          case tb:
          case fb:
            guard_flight_rider(nr_remaining_white_moves,
                               nr_remaining_black_moves,
                               n,
                               index_of_current_guarding_piece,
                               guard_type,
                               min_nr_captures_by_white);
            break;

          case cb:
            guard_flight_leaper(nr_remaining_white_moves,
                                nr_remaining_black_moves,
                                n,
                                index_of_current_guarding_piece,
                                guard_type,
                                min_nr_captures_by_white);
            break;

          default:
            assert(0);
            break;
        }

        white[index_of_current_guarding_piece].usage = piece_is_unused;
      }
    }

    FinaliseGuarding(nr_remaining_white_moves,
                     nr_remaining_black_moves,
                     min_nr_captures_by_white,
                     n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void guard_flights_king(unsigned int nr_remaining_white_moves,
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
      square const guard_from = white[index_of_king].diagram_square;
      square const *bnp;

      white[index_of_king].usage = piece_guards;

      /* try using white king for guarding from every square */
      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (e[*bnp]==vide
            && nr_reasons_for_staying_empty[*bnp]==0
            && !would_there_be_king_contact(*bnp))
        {
          unsigned int const time = count_nr_of_moves_from_to_king(roib,
                                                                   guard_from,
                                                                   *bnp);
          TraceSquare(*bnp);TraceText("\n");
          if (time<=nr_remaining_white_moves)
          {
            square const guarded = guards_black_flight(roib,*bnp);
            if (guarded!=initsquare)
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
                                                               black[index_of_king].diagram_square,
                                                               *bnp);
      if (time<=nr_remaining_black_moves)
      {
        {
          square s;
          for (s = 0; s!=maxsquare+4; ++s)
          {
            if (nr_reasons_for_staying_empty[s]>0)
              WriteSquare(s);
            assert(nr_reasons_for_staying_empty[s]==0);
          }
        }

        SetPiece(roin,*bnp,king_flags);
        king_square[Black] = *bnp;
        black[index_of_king].usage = piece_is_king;
        if (goal_to_be_reached==goal_mate)
        {
          intelligent_mate_generate_checking_moves(nr_remaining_white_moves,
                                                   nr_remaining_black_moves-time,
                                                   1,
                                                   0,
                                                   n);
          intelligent_mate_generate_checking_moves(nr_remaining_white_moves,
                                                   nr_remaining_black_moves-time,
                                                   2,
                                                   0,
                                                   n);
        }
        else
        {
          unsigned int const min_nr_captures_by_white = 0;
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
  black[index_of_king].diagram_square= king_square[Black];
  PieceId2index[GetPieceId(spec[king_square[Black]])] = index_of_king;
  ++MaxPiece[Black];

  if (king_square[White]==initsquare)
    white[index_of_king].usage = piece_is_missing;
  else
  {
    white[index_of_king].usage = piece_is_unused;
    white[index_of_king].type = e[king_square[White]];
    white[index_of_king].flags = spec[king_square[White]];
    white[index_of_king].diagram_square = king_square[White];
    PieceId2index[GetPieceId(spec[king_square[White]])] = index_of_king;
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
        white[MaxPiece[White]].diagram_square = *bnp;
        white[MaxPiece[White]].usage = piece_is_unused;
        if (e[*bnp]==pb)
          moves_to_white_prom[MaxPiece[White]] = count_moves_to_white_promotion(*bnp);
        PieceId2index[GetPieceId(spec[*bnp])] = MaxPiece[White];
        ++MaxPiece[White];
      }

    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (king_square[Black]!=*bnp && e[*bnp]<vide)
      {
        black[MaxPiece[Black]].type = e[*bnp];
        black[MaxPiece[Black]].flags = spec[*bnp];
        black[MaxPiece[Black]].diagram_square = *bnp;
        black[MaxPiece[Black]].usage = piece_is_unused;
        PieceId2index[GetPieceId(spec[*bnp])] = MaxPiece[Black];
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
