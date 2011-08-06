/******************** MODIFICATIONS to pyint.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/06/14 TLi  bug fix in function guards_black_flight()
 **
 ** 2007/12/27 TLi  bug fix in function stalemate_immobilise()
 **
 **************************** End of List ******************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "py.h"
#include "pyproc.h"
#include "pyhash.h"
#include "pymsg.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "stipulation/help_play/branch.h"
#include "pybrafrk.h"
#include "pyproof.h"
#include "pypipe.h"
#include "pymovenb.h"
#include "optimisations/intelligent/filter.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "options/maxsolutions/maxsolutions.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/legal_move_finder.h"
#include "platform/maxtime.h"
#include "trace.h"

typedef unsigned int index_type;

static index_type indices[MaxPieceId];

#define SetIndex(s,i) indices[GetPieceId(s)] = (i)
#define GetIndex(s)   indices[GetPieceId(s)]

typedef struct {
    square  sq;
    Flags   sp;
    piece   p;
    boolean used;
} PIECE;

static goal_type goal_to_be_reached;

static unsigned int MaxPieceAll;
static unsigned int MaxPiece[nr_sides];
unsigned int MovesLeft[nr_sides];

static unsigned long MatesMax;

static PIECE white[nr_squares_on_board];
static PIECE black[nr_squares_on_board];
static boolean is_cast_supp;
static square is_ep, is_ep2;
static unsigned int moves_to_prom[nr_squares_on_board];
static square squarechecking;
static square const *deposebnp;
static piece piecechecking;
static unsigned int index_of_piece_delivering_check;
enum { index_of_king = 0 };

static PIECE Mate[nr_squares_on_board];
static unsigned int IndxChP;

static slice_index current_start_slice;

static boolean solutions_found;

#define SetPiece(P, SQ, SP) {e[SQ]= P; spec[SQ]= SP;}


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

  switch (guarding)
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

static boolean are_kings_too_close(void)
{
  return (king_square[White]!=initsquare
          && move_diff_code[abs(king_square[White]-king_square[Black])]<3);
}

static boolean is_black_king_attacked_by_non_king_too_often(void)
{
  unsigned int nrChecks = 0;
  numvec k;
  for (k = vec_rook_start; k<=vec_rook_end; k++)
    if (e[king_square[Black]+vec[k]]==tb || e[king_square[Black]+vec[k]]==db)
      ++nrChecks;

  for (k = vec_bishop_start; k<=vec_bishop_end; k++)
    if (e[king_square[Black]+vec[k]]==fb || e[king_square[Black]+vec[k]]==db)
      ++nrChecks;

  for (k = vec_knight_start; k<=vec_knight_end; k++)
    if (e[king_square[Black]+vec[k]] == cb)
      ++nrChecks;

  if (e[king_square[Black]+dir_down+dir_right]==pb
      || e[king_square[Black]+dir_down+dir_left]==pb)
    ++nrChecks;

  return nrChecks>(goal_to_be_reached==goal_stale ? 0: 1);
}

static boolean is_white_king_attacked_by_non_king(void)
{
  return ((*checkfunctions[Pawn])(king_square[White],pn,eval_ortho)
          || (*checkfunctions[Knight])(king_square[White],cn,eval_ortho)
          || (*checkfunctions[Fers])(king_square[White],fn,eval_ortho)
          || (*checkfunctions[Wesir])(king_square[White],tn,eval_ortho)
          || (*checkfunctions[ErlKing])(king_square[White],dn,eval_ortho));
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

static int FroToKing(square f_sq, square t_sq) {
  int diffcol= f_sq % onerow - t_sq % onerow;
  int diffrow= f_sq / onerow - t_sq / onerow;

  if (diffcol < 0)
    diffcol= -diffcol;

  if (diffrow < 0)
    diffrow= -diffrow;

  return (diffcol > diffrow) ? diffcol : diffrow;
}

static unsigned int count_nr_of_moves_from_to_no_check(piece from_piece,
                                                       square from_square,
                                                       piece to_piece,
                                                       square to_square)
{
  if (from_square==to_square && from_piece==to_piece)
    return 0;
  else
  {
    switch (abs(from_piece))
    {
      case Knight:
        return ProofKnightMoves[abs(from_square-to_square)];

      case Rook:
        return CheckDirRook[from_square-to_square]==0 ? 2 : 1;

      case Queen:
        return CheckDirRook[from_square-to_square]==0 && CheckDirBishop[from_square-to_square]==0 ? 2 : 1;

      case Bishop:
        if (SquareCol(from_square)==SquareCol(to_square))
          return CheckDirBishop[from_square-to_square]==0 ? 2 : 1;
        else
          return maxply+1;

      case King:
      {
        int minmoves= FroToKing(from_square,to_square);
        /* castling */
        if (testcastling)
        {
          if (from_piece == roib)
          {
            /* white king */
            if (from_square == square_e1)
            {
              if (TSTCASTLINGFLAGMASK(nbply,White,ra_cancastle&castling_flag[castlings_flags_no_castling]))
              {
                int const withcast= FroToKing(square_c1,to_square);
                if (withcast < minmoves)
                  minmoves = withcast;
              }
              if (TSTCASTLINGFLAGMASK(nbply,White,rh_cancastle&castling_flag[castlings_flags_no_castling]))
              {
                int const withcast= FroToKing(square_g1,to_square);
                if (withcast < minmoves)
                  minmoves = withcast;
              }
            }
          }
          else {
            /* black king */
            if (from_square == square_e8)
            {
              if (TSTCASTLINGFLAGMASK(nbply,Black,ra_cancastle&castling_flag[castlings_flags_no_castling]))
              {
                int const withcast= FroToKing(square_c8,to_square);
                if (withcast < minmoves)
                  minmoves = withcast;
              }
              if (TSTCASTLINGFLAGMASK(nbply,Black,rh_cancastle&castling_flag[castlings_flags_no_castling]))
              {
                int const withcast= FroToKing(square_g8,to_square);
                if (withcast < minmoves)
                  minmoves = withcast;
              }
            }
          }
        }
        return minmoves;
      }

      case Pawn:
      {
        if (from_piece == to_piece)
        {
          int const diffcol = abs(from_square % onerow - to_square % onerow);
          int const diffrow = from_square / onerow - to_square / onerow;
          if (from_piece<vide)
          {
            /* black pawn */
            if (diffrow<diffcol)
              /* if diffrow <= 0 then this test is true, since diffcol is always
               * non-negative
               */
              return maxply+1;

            if (from_square>=square_a7 && diffrow-2 >= diffcol)
              /* double step */
              return diffrow-1;

            return diffrow;
          }
          else
          {
            /* white pawn */
            if (-diffrow<diffcol)
              return maxply+1;

            if (from_square<=square_h2 && -diffrow-2 >= diffcol)
              /* double step */
              return -diffrow-1;

            return -diffrow;
          }
        }
        else
        {
          /* promotion */
          unsigned int minmoves = maxply+1;
          unsigned int curmoves;
          square v_sq;
          square const start = from_piece<vide ? square_a1 : square_a8;

          for (v_sq = start; v_sq<start+8; ++v_sq)
          {
            curmoves = (count_nr_of_moves_from_to_no_check(from_piece,from_square,from_piece,v_sq)
                        + count_nr_of_moves_from_to_no_check(to_piece,v_sq,to_piece,to_square));
            if (curmoves < minmoves)
              minmoves = curmoves;
          }

          return minmoves;
        }
      }

      default:
        return 1;
    }
  }
} /* count_nr_of_moves_from_to_no_check */

static unsigned int count_nr_of_moves_from_to_checking(piece from_piece,
                                                       square from_square,
                                                       piece to_piece,
                                                       square to_square)
{
  if (from_square==to_square && from_piece==to_piece)
  {
    if (from_piece == pb)
      return maxply+1;

    else if (from_piece == cb)
      return 2;

    /* it's a rider */
    else if (move_diff_code[abs(king_square[Black]-to_square)]<3)
      return 2;
  }

  return count_nr_of_moves_from_to_no_check(from_piece,from_square,to_piece,to_square);
}

static unsigned int MovesRequired[nr_sides][maxply+1];
static unsigned int CapturesLeft[maxply+1];

static boolean isGoalReachableRegularGoals(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (sol_per_matingpos>=maxsol_per_matingpos)
  {
    FlagMaxSolsPerMatingPosReached = true;
    result = false;
  }

  else if (pprise[nbply] && Mate[GetIndex(pprispec[nbply])].sq!=initsquare)
    /* a piece has been captured that participates in the mate */
    result = false;

  else
  {
    TraceValue("%u",MovesLeft[White]);
    TraceValue("%u\n",MovesLeft[Black]);

    if (nbply == 2
        || (testcastling && castling_flag[nbply] != castling_flag[nbply-1]))
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
          unsigned const index = GetIndex(spec[from_square]);
          if (Mate[index].sq!=initsquare)
          {
            Side const from_side = from_piece>vide ? White : Black;
            if (index==IndxChP && MovesLeft[White]>0)
            {
              square const save_king_square = king_square[Black];
              king_square[Black] = Mate[GetIndex(spec[king_square[Black]])].sq;
              MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_checking(from_piece,
                                                                                    from_square,
                                                                                    Mate[index].p,
                                                                                    Mate[index].sq);
              king_square[Black] = save_king_square;
            }
            else
              MovesRequired[from_side][nbply] += count_nr_of_moves_from_to_no_check(from_piece,
                                                                                    from_square,
                                                                                    Mate[index].p,
                                                                                    Mate[index].sq);
          }
        }
      }
    }
    else
    {
      unsigned int const index = GetIndex(jouespec[nbply]);
      MovesRequired[White][nbply] = MovesRequired[White][nbply-1];
      MovesRequired[Black][nbply] = MovesRequired[Black][nbply-1];

      if (Mate[index].sq!=initsquare)
      {
        unsigned int time_before;
        unsigned int time_now;
        if (index==IndxChP)
        {
          square const save_king_square = king_square[Black];
          king_square[Black] = Mate[GetIndex(spec[king_square[Black]])].sq;
          time_before = count_nr_of_moves_from_to_checking(pjoue[nbply],
                                                           move_generation_stack[nbcou].departure,
                                                           Mate[index].p,
                                                           Mate[index].sq);
          king_square[Black] = save_king_square;
        }
        else
          time_before = count_nr_of_moves_from_to_no_check(pjoue[nbply],
                                                           move_generation_stack[nbcou].departure,
                                                           Mate[index].p,
                                                           Mate[index].sq);

        if (index==IndxChP && MovesLeft[White]>0)
        {
          square const save_king_square = king_square[Black];
          king_square[Black] = Mate[GetIndex(spec[king_square[Black]])].sq;
          time_now = count_nr_of_moves_from_to_checking(e[move_generation_stack[nbcou].arrival],
                                                        move_generation_stack[nbcou].arrival,
                                                        Mate[index].p,
                                                        Mate[index].sq);
          king_square[Black] = save_king_square;
        }
        else
          time_now = count_nr_of_moves_from_to_no_check(e[move_generation_stack[nbcou].arrival],
                                                        move_generation_stack[nbcou].arrival,
                                                        Mate[index].p,
                                                        Mate[index].sq);

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

/* declarations */
static void stalemate_block_square_black(unsigned int blmoves, unsigned int whmoves,
                                         unsigned int blpcallowed, unsigned int whpcallowed,
                                         square toblock,
                                         boolean morethanonecheck,
                                         stip_length_type n);
static void stalemate_place_an_unused_black_piece(unsigned int blmoves, unsigned int whmoves,
                                                  unsigned int blpcallowed, unsigned int whpcallowed,
                                                  stip_length_type n);
static void stalemate_immobilise(unsigned int blmoves, unsigned int whmoves,
                                 unsigned int blpcallowed, unsigned int whpcallowed,
                                 stip_length_type n);
static void stalemate_avoid_check(unsigned int blmoves, unsigned int whmoves,
                                  unsigned int blpcallowed, unsigned int whpcallowed,
                                  stip_length_type n);
static int MovesToBlock(square sq, unsigned int blmoves);
static void stalemate_fix_white_king_on_diagram_square(unsigned int blmoves, unsigned int whmoves,
                                                       unsigned int blpcallowed, unsigned int whpcallowed,
                                                       stip_length_type n);
static void mate_neutralise_guarding_pieces(unsigned int blmoves, unsigned int whmoves,
                                            unsigned int blpc, unsigned int whpc,
                                            stip_length_type n);
static void mate_place_any_black_piece_on(square placed_on,
                                          unsigned int blmoves, unsigned int whmoves,
                                          unsigned int blpc, unsigned int whpc,
                                          stip_length_type n);
static void mate_place_any_white_piece_on(square placed_on,
                                          unsigned int blmoves, unsigned int whmoves,
                                          unsigned int blpc, unsigned int whpc,
                                          stip_length_type n);
static void stalemate_avoid_check_to_white_king(unsigned int blmoves, unsigned int whmoves,
                                                unsigned int blpcallowed, unsigned int whpcallowed,
                                                stip_length_type n);

static void stalemate_store_target_position(unsigned int blmoves, unsigned int whmoves,
                                            unsigned int blpcallowed, unsigned int whpcallowed,
                                            stip_length_type n)
{
  unsigned int i, index, unused = 0;
  square const *bnp;
  square _rb, _rn;
  Flags sp;

  if (hasMaxtimeElapsed())
    return;

  if (!white[index_of_king].used
      && white[index_of_king].sq!=square_e1
      && whmoves==0)
  {
    if (e[white[index_of_king].sq]==vide)
      stalemate_fix_white_king_on_diagram_square(blmoves,whmoves,
                                                 blpcallowed,whpcallowed,
                                                 n);
    return;
  }

  for (i= 1; i < MaxPiece[Black]; ++i)
    if (!black[i].used)
      ++unused;

  if (unused>0)
  {
#if defined(DETAILS)
    WritePosition();
    sprintf(GlobalStr,"unused= %d\n",unused);
    StdString(GlobalStr);
#endif
    stalemate_place_an_unused_black_piece(blmoves,whmoves,blpcallowed,whpcallowed,n);
  }
#if defined(DEBUG)
  sprintf(GlobalStr,
          "unused: %d, MovesLeft[White]: %d\n",unused,MovesLeft[White]);
  StdString(GlobalStr);
#endif

  if (unused > MovesLeft[White])
    return;

  /* checks against the wKing should be coped with earlier !!! */
  if (echecc(nbply,White))
    stalemate_avoid_check_to_white_king(blmoves,whmoves,blpcallowed,whpcallowed,n);

  CapturesLeft[1] = unused;

  ++MatesMax;

#if defined(DETAILS)
  sprintf(GlobalStr,"mate no. %d\n",MatesMax);
  StdString(GlobalStr);
  WritePosition();
#endif

  for (i= 0; i < MaxPieceAll; ++i)
    Mate[i].sq= initsquare;

#if defined(DEBUG)
  StdString("target position:\n");
  WritePosition();
#endif

  for (bnp= boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];
    if (p!=vide && p!=obs)
    {
      sp= spec[*bnp];
      index= GetIndex(sp);
      Mate[index].p= p;
      Mate[index].sp= sp;
      Mate[index].sq= *bnp;
    }
  }

  IndxChP= index_of_piece_delivering_check == 0
    ? UINT_MAX
    : GetIndex(white[index_of_piece_delivering_check].sp);
  _rb= king_square[White];
  _rn= king_square[Black];

  /* solve the problem */
  ResetPosition();
  castling_supported= is_cast_supp;
  ep[1]= is_ep; ep2[1]= is_ep2;

#if defined(DETAILS)
  {
    int blm= 0, whm= 0, m;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp] != vide) {
        sp= spec[*bnp];
        index= GetIndex(sp);
        if (Mate[index].sq != vide
            && (*bnp != Mate[index].sq || e[*bnp] != Mate[index].p))
        {
          WritePiece(e[*bnp]); WriteSquare(*bnp);
          StdString("-->");
          if (e[*bnp] != Mate[index].p) {
            WritePiece(Mate[index].p);
          }
          WriteSquare(Mate[index].sq);
          m= count_nr_of_moves_from_to_no_check(e[*bnp],*bnp,Mate[index].p,Mate[index].sq,0);
          if (e[*bnp] < vide)
            blm+= m;
          else
            whm+= m;
          sprintf(GlobalStr,"(%d)  ",m);
          StdString(GlobalStr);
        }
      }
    sprintf(GlobalStr,
            "\nblack moves: %d, white moves: %d\n",blm,whm);
    StdString(GlobalStr);
  }
#endif

  sol_per_matingpos = 0;

  closehash();
  inithash(current_start_slice);

  {
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    if (help(slices[current_start_slice].u.pipe.next,n)<=n)
      solutions_found = true;
    OptFlag[movenbr] = save_movenbr;
  }

  /* reset the old mating position */
  for (bnp= boardnum; *bnp; bnp++) {
    e[*bnp]= vide;
    spec[*bnp]= EmptySpec;
  }

  for (i= 0; i < MaxPieceAll; i++) {
    if (Mate[i].sq != initsquare) {
      e[Mate[i].sq]= Mate[i].p;
      spec[Mate[i].sq]= Mate[i].sp;
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

  king_square[White]= _rb;
  king_square[Black]= _rn;

  ep[1]= ep2[1]= initsquare;

  castling_supported= false;
} /* stalemate_store_target_position */

void write_indentation(void)
{
}

static void stalemate_place_an_unused_black_piece(unsigned int blmoves, unsigned int whmoves,
                                                  unsigned int blpcallowed, unsigned int whpcallowed,
                                                  stip_length_type n)
{
  square const *bnp;
  square const * const isbnp = deposebnp;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "DeposeBlPiece(%d,%d,%d), *deposebnp=%d\n",
          blmoves,whmoves,blpcallowed,*deposebnp);
  StdString(GlobalStr);
#endif

  for (bnp= deposebnp; *bnp; bnp++)
    if (e[*bnp] == vide)
    {
#if defined(DEBUG)
      StdString("deposing piece on ");
      WriteSquare(*bnp);
      StdString(" ");
#endif
      deposebnp= bnp;
      stalemate_block_square_black(blmoves,whmoves,blpcallowed,whpcallowed,*bnp,false,n);
    }

  deposebnp= isbnp;
#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "leaving DeposeBlPiece, deposebnp=%d\n",*deposebnp);
  StdString(GlobalStr);
#endif

} /* stalemate_place_an_unused_black_piece */

static void mate_prevent_check_against_white_king(unsigned int blmoves, unsigned int whmoves,
                                                  unsigned int blpc, unsigned int whpc,
                                                  stip_length_type n)
{
  square trouble= initsquare;
  boolean fbm= flagmummer[Black];

  flagmummer[Black]= false;
  genmove(Black);
  flagmummer[Black]= fbm;

  while(encore() && (trouble == initsquare)) {
    if (move_generation_stack[nbcou].arrival == king_square[White]) {
      trouble= move_generation_stack[nbcou].departure;
    }
    nbcou--;
  }

  finply();

  if (trouble == initsquare) {
        WritePosition();
    FtlMsg(ErrUndef);
  }

  if (is_rider(abs(e[trouble]))) {
    square sq;
    int dir= CheckDirQueen[king_square[White]-trouble];

    for (sq= trouble+dir; sq != king_square[White]; sq+=dir)
    {
      mate_place_any_black_piece_on(sq,blmoves,whmoves,blpc,whpc,n);
      mate_place_any_white_piece_on(sq,blmoves,whmoves,blpc,whpc,n);
    }
  }

  return;
}

static boolean mate_exists_redundant_white_piece(void)
{
  boolean result = false;
  square const *bnp;

  /* check for redundant white pieces */
  for (bnp = boardnum; !result && *bnp; bnp++)
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

  return result;
} /* mate_exists_redundant_white_piece */

static void mate_store_target_position(unsigned int blmoves, unsigned int whmoves,
                                       unsigned int blpc, unsigned int whpc,
                                       stip_length_type n)
{
  unsigned int i, index;
  square const *bnp;
  square _rb, _rn;
  Flags sp;

  if (slice_has_solution(slices[current_start_slice].u.fork.fork)!=has_solution) {
    mate_neutralise_guarding_pieces(blmoves,whmoves,blpc,whpc,n);
    return;
  }

  if (mate_exists_redundant_white_piece())
    return;

  if (king_square[White]==initsquare
      && white[index_of_king].sq!=initsquare
      && white[index_of_king].sq!=square_e1
      && whmoves==0)
    return;

  if (echecc(nbply,White))
    mate_prevent_check_against_white_king(blmoves,whmoves,blpc,whpc,n);

  ++MatesMax;

#if defined(DETAILS)
  sprintf(GlobalStr,"mate no. %d\n",MatesMax);
  StdString(GlobalStr);
  WritePosition();
#endif

  for (i= 0; i < MaxPieceAll; i++) {
    Mate[i].sq= initsquare;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    if ((e[*bnp] != vide) && (e[*bnp] != obs)) {
      sp= spec[*bnp];
      index= GetIndex(sp);
      Mate[index].p= e[*bnp];
      Mate[index].sp= sp;
      Mate[index].sq= *bnp;
    }
  }

  IndxChP= index_of_piece_delivering_check == 0
    ? UINT_MAX
    : GetIndex(white[index_of_piece_delivering_check].sp);
  _rb= king_square[White];
  _rn= king_square[Black];

  /* solve the problem */
  ResetPosition();
  castling_supported= is_cast_supp;

  closehash();
  inithash(current_start_slice);

  sol_per_matingpos = 0;

#if defined(DETAILS)
  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] != vide) {
      sp= spec[*bnp];
      index= GetIndex(sp);
      if (Mate[index].sq != vide) {
        WritePiece(e[*bnp]); WriteSquare(*bnp);
        StdString("-->");
        WriteSquare(Mate[index].sq);
        StdString("  ");
      }
    }
  }
  StdString("\n");
#endif

  {
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    if (help(slices[current_start_slice].u.pipe.next,n)<=n)
      solutions_found = true;
    OptFlag[movenbr] = save_movenbr;
  }

  /* reset the old mating position */
  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] != obs) {
      e[*bnp]= vide;
      spec[*bnp]= EmptySpec;
    }
  }

  for (i= 0; i < MaxPieceAll; i++) {
    if (Mate[i].sq != initsquare) {
      e[Mate[i].sq]= Mate[i].p;
      spec[Mate[i].sq]= Mate[i].sp;
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

  king_square[White]= _rb;
  king_square[Black]= _rn;

  ep[1]= ep2[1]= initsquare;

  castling_supported= false;
} /* mate_store_target_position */

static void mate_pin_black_piece(square    topin,
                                 unsigned int blmoves, unsigned int whmoves,
                                 unsigned int blpc, unsigned int whpc,
                                 stip_length_type n)
{
  square    sq= topin;
  int dir;
  unsigned int time;
  unsigned int i;
  boolean   diagonal;
  piece f_p;

  dir= sq-king_square[Black];
  diagonal= SquareCol(sq) == SquareCol(king_square[Black]);
  while (e[sq+=dir] == vide) {
    for (i= 1; i < MaxPiece[White]; i++) {
      if (!white[i].used && (f_p= white[i].p) != cb) {
        if (f_p == (diagonal ? tb : fb)) {
          continue;
        }
        white[i].used= true;
        if (f_p == pb) {
          if (diagonal) {
            time= count_nr_of_moves_from_to_no_check(f_p,white[i].sq,Bishop,sq);
            if (time <= whmoves) {
              SetPiece(Bishop,sq,white[i].sp);
              mate_store_target_position(blmoves,whmoves-time,blpc,whpc,n);
            }
          }
          else {
            time=
              count_nr_of_moves_from_to_no_check(f_p,white[i].sq,Rook,sq);
            if (time <= whmoves) {
              SetPiece(Rook,sq,white[i].sp);
              mate_store_target_position(blmoves,whmoves-time,blpc,whpc,n);
            }
          }
          time= count_nr_of_moves_from_to_no_check(f_p,white[i].sq,Queen,sq);
          if (time <= whmoves) {
            SetPiece(Queen,sq,white[i].sp);
            mate_store_target_position(blmoves,whmoves-time,blpc,whpc,n);
          }
        }
        else {
          time= count_nr_of_moves_from_to_no_check(f_p,white[i].sq,f_p,sq);
          if (time <= whmoves) {
            SetPiece(f_p,sq,white[i].sp);
            mate_store_target_position(blmoves,whmoves-time,blpc,whpc,n);
          }
        }
        white[i].used= false;
      }
    }
    e[sq]= vide;
    spec[sq]= EmptySpec;
  }
}

static void stalemate_immobilise_by_pin(unsigned int blmoves, unsigned int whmoves,
                                        unsigned int blpcallowed, unsigned int whpcallowed,
                                        square topin,
                                        stip_length_type n)
{
  int dir;
  unsigned int time;
  unsigned int i;
  boolean   diagonal;
  square    sq;
  piece f_p;

  dir= CheckDirQueen[topin-king_square[Black]];
  diagonal= SquareCol(king_square[Black]+dir) == SquareCol(king_square[Black]);

  /* we can only pin in Q-lines */
  /* black Queens cannot be pinned */
  if (!dir || (e[topin] == -Queen)) {
    return;
  }

  /* black Bishops cannot be pinned on B-lines */
  if (CheckDirBishop[dir] && (e[topin] == -Bishop)) {
    return;
  }

  /* black Rook cannot be pinned on R-lines */
  if (CheckDirRook[dir] && (e[topin] == -Rook)) {
    return;
  }

  /* check if there are any pieces between black king and the
   * the piece to be pinned
   */
  sq= king_square[Black];
  while (e[(sq+=dir)]==vide)
    ;
  if (sq != topin)
    return;

  if (blpcallowed==0)
    return;

  sq= topin;
  while (e[sq+=dir] == vide) {
    for (i= 1; i < MaxPiece[White]; i++) {
      if (!white[i].used && ((f_p= white[i].p) != cb)) {
        if (f_p == (diagonal ? tb : fb))
          continue;

        white[i].used= true;
        if (f_p == pb)
        {
          if (diagonal) {
            time = count_nr_of_moves_from_to_no_check(f_p,white[i].sq,Bishop,sq);
            if (time<=whmoves)
            {
              SetPiece(Bishop,sq,white[i].sp);
              if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
                stalemate_store_target_position(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
              else
                stalemate_immobilise(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
            }
          }
          else {
            time= count_nr_of_moves_from_to_no_check(f_p,white[i].sq,Rook,sq);
            if (time<=whmoves)
            {
              SetPiece(Rook,sq,white[i].sp);
              if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
                stalemate_store_target_position(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
              else
                stalemate_immobilise(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
            }
          }
          time= count_nr_of_moves_from_to_no_check(f_p,white[i].sq,Queen,sq);
          if (time<=whmoves)
          {
            SetPiece(Queen,sq,white[i].sp);
            if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
              stalemate_store_target_position(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
            else
              stalemate_immobilise(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
          }
        }
        else
        {
          time = count_nr_of_moves_from_to_no_check(f_p,white[i].sq,f_p,sq);
          if (time<=whmoves)
          {
            SetPiece(f_p,sq,white[i].sp);
            if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
              stalemate_store_target_position(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
            else
              stalemate_immobilise(blmoves,whmoves-time,blpcallowed-1,whpcallowed,n);
          }
        }
        white[i].used= false;
      }
    }
    e[sq]= vide;
    spec[sq]= EmptySpec;
  }
} /* stalemate_immobilise_by_pin */

static boolean attacks_white_king(square from, piece p)
{
  if (king_square[White]==initsquare)
    return false;
  else
  {
    int const dir = from-king_square[White];
    switch(p)
    {
      case dn:
        return CheckDirQueen[dir]==dir;

      case cn:
        return CheckDirKnight[king_square[White]-from]!=0;

      case pn:
        return dir==dir_up+dir_right || dir==dir_up+dir_left;

      case fn:
        return CheckDirBishop[dir]==dir;

      case tn:
        return CheckDirRook[dir]==dir;

      default:
        assert(0);
        return false;
    }
  }
}

static void stalemate_fix_white_king_on_diagram_square(unsigned int blmoves,
                                                       unsigned int whmoves,
                                                       unsigned int blpcallowed,
                                                       unsigned int whpcallowed,
                                                       stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  white[index_of_king].used = true;

  king_square[White] = white[index_of_king].sq;
  SetPiece(roib,king_square[White],white[index_of_king].sp);

  if (!are_kings_too_close()
      && !is_black_king_attacked_by_non_king_too_often()
      && !is_white_king_attacked_by_non_king())
  {
    if (echecc(nbply,Black))
      stalemate_avoid_check(blmoves,whmoves,blpcallowed,whpcallowed,n);
    else if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
      stalemate_store_target_position(blmoves,whmoves,blpcallowed,whpcallowed,n);
    else
      stalemate_immobilise(blmoves,whmoves,blpcallowed,whpcallowed,n);
  }

  e[king_square[White]] = vide;
  spec[king_square[White]] = EmptySpec;
  king_square[White] = initsquare;

  white[index_of_king].used= false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_square_black(unsigned int blmoves, unsigned int whmoves,
                                         unsigned int blpcallowed, unsigned int whpcallowed,
                                         square toblock,
                                         boolean morethanonecheck,
                                         stip_length_type n)
{
  unsigned int pcreq;
  unsigned int time;
  unsigned int i;
  piece f_p;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering ImmobiliseByBlackBlock(%u,%u,%d)\n",
          blmoves,whmoves,toblock);
  StdString(GlobalStr);
#endif

  for (i= 1; i < MaxPiece[Black]; i++) {
    if (!black[i].used) {
      f_p= black[i].p;
      black[i].used= true;

      /* promotions */
      if (f_p == -Pawn) {
        /* A rough check whether it is worth thinking about
           promotions.
        */
        unsigned int moves= black[i].sq / onerow - nr_of_slack_rows_below_board;
        if (moves > 5) {
          /* double step possible */
          moves= 5;
        }
        if (toblock>=square_a2)
          /* square is not on 1st rank -- 1 move
             necessary to get there
          */
          moves++;

        if (blmoves >= moves) {
          piece pp= -getprompiece[vide];
          while (pp != vide)
          {
            time= count_nr_of_moves_from_to_no_check(f_p,black[i].sq,-pp,toblock);
            if (time <= blmoves
                && whpcallowed>=1
                && !attacks_white_king(toblock,pp))
            {
              SetPiece(pp,toblock,black[i].sp);
              if (morethanonecheck) {
                stalemate_avoid_check(blmoves-time,whmoves,
                                      blpcallowed,whpcallowed-1,
                                      n);
              }
              else if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
                stalemate_store_target_position(blmoves-time,whmoves,
                               blpcallowed,whpcallowed-1,
                               n);
              else
                stalemate_immobilise(blmoves-time,whmoves,
                           blpcallowed,whpcallowed-1,
                           n);
            }
            pp= -getprompiece[-pp];
          }
        }
        pcreq= abs(black[i].sq%onerow - toblock%onerow);
      }
      else {
        pcreq= 0;
      }

      if (f_p!=-Pawn || toblock>=square_a2)
      {
        time= count_nr_of_moves_from_to_no_check(f_p,black[i].sq,f_p,toblock);
        if (time <= blmoves
            && pcreq <= blpcallowed
            && whpcallowed>=1
            && !attacks_white_king(toblock,f_p))
        {
          SetPiece(f_p,toblock,black[i].sp);
          if (morethanonecheck)
            stalemate_avoid_check(blmoves-time,whmoves,
                                  blpcallowed-pcreq,whpcallowed-1,n);
          else if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
            stalemate_store_target_position(blmoves-time,whmoves,
                           blpcallowed-pcreq,whpcallowed-1,n);
          else
            stalemate_immobilise(blmoves-time,whmoves,
                       blpcallowed-pcreq,whpcallowed-1,n);
        }
      }
      black[i].used= false;
    }
  }
  e[toblock]= vide;
  spec[toblock]= EmptySpec;

#if defined(DEBUG)
  write_indentation();StdString("leaving ImmobilizeByblBlock\n");
#endif
} /* stalemate_block_square_black */

static void stalemate_block_square_with_unpromoted_white_pawn(unsigned int blocker_index,
                                                              unsigned int blmoves,
                                                              unsigned int whmoves,
                                                              unsigned int blpcallowed,
                                                              unsigned int whpcallowed,
                                                              square to_be_blocked,
                                                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const blocks_from = white[blocker_index].sq;
    unsigned int const nr_captures_required = abs(blocks_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = count_nr_of_moves_from_to_no_check(pb,
                                                                 blocks_from,
                                                                 pb,
                                                                 to_be_blocked);
    if (time<=whmoves)
    {
      SetPiece(pb,to_be_blocked,white[blocker_index].sp);

      if (blpcallowed>=1
          && whpcallowed>=nr_captures_required
          && !is_black_king_attacked_by_non_king_too_often())
      {
        if (echecc(nbply,Black))
          stalemate_avoid_check(blmoves,whmoves-time,
                                blpcallowed-1,whpcallowed-nr_captures_required,
                                n);
        else if (slice_has_solution(slices[current_start_slice].u.fork.fork)
                 ==has_solution)
          stalemate_store_target_position(blmoves,whmoves-time,
                                          blpcallowed-1,whpcallowed-nr_captures_required,
                                          n);
        else
          stalemate_immobilise(blmoves,whmoves-time,
                               blpcallowed-1,whpcallowed-nr_captures_required,
                               n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_square_with_promoted_white_pawn(unsigned int blocker_index,
                                                            unsigned int blmoves,
                                                            unsigned int whmoves,
                                                            unsigned int blpcallowed,
                                                            unsigned int whpcallowed,
                                                            square to_be_blocked,
                                                            stip_length_type n)
{
  unsigned int const nr_moves_required = (to_be_blocked<=square_h7
                                          ? moves_to_prom[blocker_index]+1
                                          : moves_to_prom[blocker_index]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_moves_required);
  if (whmoves>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(pb,
                                                                   white[blocker_index].sq,
                                                                   pp,
                                                                   to_be_blocked);
      if (time<=whmoves && blpcallowed>=1)
      {
        SetPiece(pp,to_be_blocked,white[blocker_index].sp);
        if (!is_black_king_attacked_by_non_king_too_often())
        {
          if (echecc(nbply,Black))
            stalemate_avoid_check(blmoves,whmoves-time,
                                  blpcallowed-1,whpcallowed,
                                  n);
          else if (slice_has_solution(slices[current_start_slice].u.fork.fork)
                   ==has_solution)
            stalemate_store_target_position(blmoves,whmoves-time,
                                            blpcallowed-1,whpcallowed,
                                            n);
          else
            stalemate_immobilise(blmoves,whmoves-time,
                                 blpcallowed-1,whpcallowed,
                                 n);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_square_with_white_king(unsigned int blmoves,
                                                   unsigned int whmoves,
                                                   unsigned int blpcallowed,
                                                   unsigned int whpcallowed,
                                                   square to_be_blocked,
                                                   stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(roib,
                                                                 white[index_of_king].sq,
                                                                 roib,
                                                                 to_be_blocked);
    if (time<=whmoves)
    {
      SetPiece(roib,to_be_blocked,white[index_of_king].sp);
      king_square[White] = to_be_blocked;

      if (!are_kings_too_close() && !is_white_king_attacked_by_non_king())
      {
        if (echecc(nbply,Black))
          stalemate_avoid_check(blmoves,whmoves-time,
                                blpcallowed,whpcallowed,
                                n);
        else if (slice_has_solution(slices[current_start_slice].u.fork.fork)
                 ==has_solution)
          stalemate_store_target_position(blmoves,whmoves-time,
                                          blpcallowed,whpcallowed,
                                          n);
        else
          stalemate_immobilise(blmoves,whmoves-time,
                               blpcallowed,whpcallowed,
                               n);
      }

      king_square[White] = initsquare;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_square_with_white_officer(piece blocker_type,
                                                      unsigned int blocker_index,
                                                      unsigned int blmoves,
                                                      unsigned int whmoves,
                                                      unsigned int blpcallowed,
                                                      unsigned int whpcallowed,
                                                      square to_be_blocked,
                                                      stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                 white[blocker_index].sq,
                                                                 blocker_type,
                                                                 to_be_blocked);
    if (time<=whmoves)
    {
      SetPiece(blocker_type,to_be_blocked,white[blocker_index].sp);
      if (blpcallowed>=1
          && !is_black_king_attacked_by_non_king_too_often())
      {
        if (echecc(nbply,Black))
          stalemate_avoid_check(blmoves,whmoves-time,
                                blpcallowed-1,whpcallowed,
                                n);
        else if (slice_has_solution(slices[current_start_slice].u.fork.fork)
                 ==has_solution)
          stalemate_store_target_position(blmoves,whmoves-time,
                                          blpcallowed-1,whpcallowed,
                                          n);
        else
          stalemate_immobilise(blmoves,whmoves-time,
                               blpcallowed-1,whpcallowed,
                               n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_block_square_white(unsigned int blmoves,
                                         unsigned int whmoves,
                                         unsigned int blpcallowed,
                                         unsigned int whpcallowed,
                                         square to_be_blocked,
                                         stip_length_type n)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!white[index_of_king].used)
  {
    white[index_of_king].used = true;
    stalemate_block_square_with_white_king(blmoves,whmoves,
                                           blpcallowed,whpcallowed,
                                           to_be_blocked,
                                           n);
    white[index_of_king].used = false;
  }

  for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
    if (!white[blocker_index].used)
    {
      piece const blocker_type = white[blocker_index].p;

      white[blocker_index].used = true;

      if (blocker_type==pb)
      {
        stalemate_block_square_with_promoted_white_pawn(blocker_index,
                                                        blmoves,whmoves,
                                                        blpcallowed,whpcallowed,
                                                        to_be_blocked,
                                                        n);
        stalemate_block_square_with_unpromoted_white_pawn(blocker_index,
                                                          blmoves,whmoves,
                                                          blpcallowed,whpcallowed,
                                                          to_be_blocked,
                                                          n);
      }
      else
        stalemate_block_square_with_white_officer(blocker_type,blocker_index,
                                                  blmoves,whmoves,
                                                  blpcallowed,whpcallowed,
                                                  to_be_blocked,
                                                  n);

      white[blocker_index].used = false;
    }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean can_white_pin(unsigned int whmoves)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; i++)
    if (!(white[i].used
          || white[i].p==cb
          || (white[i].p==pb && whmoves<moves_to_prom[i])))
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
    if (!black[i].used)
      ++nr_unused_pieces[Black];

  if (nr_unused_pieces[Black]<nr_blocks_needed[Black])
    result = false;
  else
  {
    for (i = 0; i<MaxPiece[White]; ++i)
      if (!white[i].used)
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
 * @param blmoves number of remaining black moves
 * @param block_requirement blocking requirements for each square
 * @return * nullsquare more squares need to be blocked than Black can in the
 *                      blmoves remaining moves
 *         * initsquare no square is required to be blocked by Black
 *         * otherwise: most expensive square that must be blocked by Black
 */
static square find_most_expensive_square_to_be_blocked_by_black(unsigned int blmoves,
                                                                block_requirement_type const block_requirement[maxsquare+4])
{
  square result = initsquare;
  int max_number_black_moves_to_squares_to_be_blocked = -1;
  unsigned int total_number_black_moves_to_squares_to_be_blocked = 0;

  square const *bnp;
  for (bnp = boardnum; *bnp; ++bnp)
    if (block_requirement[*bnp]==black_block_needed_on_square)
    {
      int const nr_black_blocking_moves = MovesToBlock(*bnp,blmoves);
      total_number_black_moves_to_squares_to_be_blocked += nr_black_blocking_moves;
      if (total_number_black_moves_to_squares_to_be_blocked>blmoves)
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

static void stalemate_immobilise(unsigned int blmoves, unsigned int whmoves,
                                 unsigned int blpcallowed, unsigned int whpcallowed,
                                 stip_length_type n)
{
  immobilisation_state_type immobilisation_state = null_immobilisation_state;

  if (max_nr_solutions_found_in_phase())
    return;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
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
        && can_white_pin(whmoves))
      stalemate_immobilise_by_pin(blmoves,whmoves,blpcallowed,whpcallowed,immobilisation_state.position_of_trouble_maker,n);

    if (immobilisation_state.last_found_trouble_square_status<pin_required
        && can_we_block_all_necessary_squares(immobilisation_state.nr_blocks_needed))
    {
      square const most_expensive_square_to_be_blocked_by_black
        = find_most_expensive_square_to_be_blocked_by_black(blmoves,
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
          boolean const morethanonecheck = false;
          stalemate_block_square_black(blmoves,whmoves,
                                 blpcallowed,whpcallowed,
                                 immobilisation_state.last_found_trouble_square,
                                 morethanonecheck,n);
          stalemate_block_square_white(blmoves,whmoves,
                                 blpcallowed,whpcallowed,
                                 immobilisation_state.last_found_trouble_square,n);
          break;
        }

        default:
        {
          /* most_expensive_square_to_be_blocked_by_black is the most expensive
           * square among those that Black must block */
          boolean const morethanonecheck = false;
          stalemate_block_square_black(blmoves,whmoves,
                                 blpcallowed,whpcallowed,
                                 most_expensive_square_to_be_blocked_by_black,
                                 morethanonecheck,n);
          break;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* stalemate_immobilise */

static void stalemate_avoid_check_to_white_king(unsigned int blmoves, unsigned int whmoves,
                                                unsigned int blpcallowed, unsigned int whpcallowed,
                                                stip_length_type n)
{
  int checkdirs[8];
  unsigned int md = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = vec_queen_end; i>=vec_queen_start ; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[White]+vec_i]==vide)
      e[king_square[White]+vec_i] = dummyb;
  }

  for (i = vec_queen_end; i>=vec_queen_start ; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[White]+vec_i]==dummyb)
    {
      e[king_square[White]+vec_i] = vide;
      if (echecc(nbply,White))
      {
        checkdirs[md] = vec_i;
        ++md;
      }
      e[king_square[White]+vec_i] = dummyb;
    }
  }

  assert(md>0);

  for (i = vec_queen_end; i>=vec_queen_start ; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[White]+vec_i]==dummyb)
      e[king_square[White]+vec_i] = vide;
  }

  for (i = 0; i<md; i++)
  {
    square sq;
    for (sq = king_square[White]+checkdirs[i]; e[sq]==vide; sq += checkdirs[i])
    {
      stalemate_block_square_black(blmoves,whmoves,blpcallowed,whpcallowed,sq,md-1,n);
      stalemate_block_square_white(blmoves,whmoves,blpcallowed,whpcallowed,sq,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stalemate_avoid_check(unsigned int blmoves, unsigned int whmoves,
                                  unsigned int blpcallowed, unsigned int whpcallowed,
                                  stip_length_type n)
{
  int checkdirs[8];
  unsigned int md = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!is_white_king_attacked_by_non_king());

  for (i = vec_queen_end; i>=vec_queen_start; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[Black]+vec_i]==vide)
      e[king_square[Black]+vec_i] = dummyb;
  }

  for (i = vec_queen_end; i>=vec_queen_start; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[Black]+vec_i]==dummyb)
    {
      e[king_square[Black]+vec_i] = vide;
      if (echecc(nbply,Black))
      {
        checkdirs[md] = vec_i;
        ++md;
      }
      e[king_square[Black]+vec_i] = dummyb;
    }
  }

  assert(md>0);

  for (i = vec_queen_end; i>=vec_queen_start; --i)
  {
    numvec const vec_i = vec[i];
    if (e[king_square[Black]+vec_i]==dummyb)
      e[king_square[Black]+vec_i] = vide;
  }

  for (i = 0; i<md; ++i)
  {
    square sq;
    for (sq = king_square[Black]+checkdirs[i]; e[sq]==vide; sq += checkdirs[i])
    {
      stalemate_block_square_black(blmoves,whmoves,blpcallowed,whpcallowed,sq,md-1,n);
      stalemate_block_square_white(blmoves,whmoves,blpcallowed,whpcallowed,sq,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_promoted_black_pawn_on(unsigned int placed_index,
                                              square placed_on,
                                              unsigned int blmoves, unsigned int whmoves,
                                              unsigned int blpc, unsigned int whpc,
                                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = black[placed_index].sq/onerow - nr_of_slack_rows_below_board;
    if (time>5)
      time = 5;
    if (placed_on>=square_a2)
      ++time;

    if (time<=blmoves)
    {
      square const placed_from = black[placed_index].sq;
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      {
        unsigned int const time = count_nr_of_moves_from_to_no_check(pn,
                                                                     placed_from,
                                                                     -pp,
                                                                     placed_on);
        unsigned int diffcol = 0;
        if (pp==fn)
        {
          unsigned int const placed_from_file = placed_from%nr_files_on_board;
          square const promotion_square_on_same_file = square_a1+placed_from_file;
          if (SquareCol(placed_on)!=SquareCol(promotion_square_on_same_file))
            diffcol = 1;
        }

        if (diffcol<=blpc && time<=blmoves && !attacks_white_king(placed_on,pp))
        {
          SetPiece(pp,placed_on,black[placed_index].sp);
          mate_store_target_position(blmoves-time,whmoves,blpc-diffcol,whpc,n);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_unpromoted_black_pawn_on(unsigned int placed_index,
                                                square placed_on,
                                                unsigned int blmoves, unsigned int whmoves,
                                                unsigned int blpc, unsigned int whpc,
                                                stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const placed_from = black[placed_index].sq;
    unsigned int const time = count_nr_of_moves_from_to_no_check(pn,
                                                                 placed_from,
                                                                 pn,
                                                                 placed_on);
    if (time<=blmoves && !attacks_white_king(placed_on,pn))
    {
      unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);
      SetPiece(pn,placed_on,black[placed_index].sp);
      if (diffcol<=blpc)
        mate_store_target_position(blmoves-time,whmoves,blpc-diffcol,whpc,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_black_officer_on(unsigned int placed_index,
                                        piece placed_type, square placed_on,
                                        unsigned int blmoves, unsigned int whmoves,
                                        unsigned int blpc, unsigned int whpc,
                                        stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const placed_from = black[placed_index].sq;
    unsigned int const time = count_nr_of_moves_from_to_no_check(placed_type,
                                                                 placed_from,
                                                                 placed_type,
                                                                 placed_on);
    if (time<=blmoves && !attacks_white_king(placed_on,placed_type))
    {
      SetPiece(placed_type,placed_on,black[placed_index].sp);
      mate_store_target_position(blmoves-time,whmoves,blpc,whpc,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_any_black_piece_on(square placed_on,
                                          unsigned int blmoves, unsigned int whmoves,
                                          unsigned int blpc, unsigned int whpc,
                                          stip_length_type n)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
  {
    if (!black[placed_index].used)
    {
      piece const placed_type = black[placed_index].p;

      black[placed_index].used= true;

      if (placed_type==pn)
      {
        if (placed_on>=square_a2)
          mate_place_unpromoted_black_pawn_on(placed_index,placed_on,
                                              blmoves,whmoves,
                                              blpc,whpc,
                                              n);
        mate_place_promoted_black_pawn_on(placed_index,placed_on,
                                          blmoves,whmoves,
                                          blpc,whpc,
                                          n);
      }
      else
        mate_place_black_officer_on(placed_index,placed_type,placed_on,
                                    blmoves,whmoves,
                                    blpc,whpc,
                                    n);

      black[placed_index].used = false;
    }
  }

  e[placed_on] = vide;
  spec[placed_on] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_unpromoted_white_pawn_on(unsigned int placed_index,
                                                square placed_on,
                                                unsigned int blmoves, unsigned int whmoves,
                                                unsigned int blpc, unsigned int whpc,
                                                stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const placed_from = white[placed_index].sq;
    unsigned int const time = count_nr_of_moves_from_to_no_check(pb,
                                                                 placed_from,
                                                                 pb,
                                                                 placed_on);
    if (time<=whmoves)
    {
      SetPiece(pb,placed_on,white[placed_index].sp);
      if (!is_black_king_attacked_by_non_king_too_often())
      {
        unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);
        if (diffcol<=whpc)
          mate_store_target_position(blmoves,whmoves-time,blpc,whpc-diffcol,n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_promoted_white_pawn_on(unsigned int placed_index,
                                              square placed_on,
                                              unsigned int blmoves, unsigned int whmoves,
                                              unsigned int blpc, unsigned int whpc,
                                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    square const placed_from = white[placed_index].sq;
    unsigned int time = placed_from/onerow - nr_of_slack_rows_below_board;
    if (time>5)
      time= 5;
    if (placed_on<=square_h7)
      ++time;
    if (time<=whmoves)
    {
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      {
        unsigned int const time = count_nr_of_moves_from_to_no_check(pb,
                                                                     placed_from,
                                                                     pp,
                                                                     placed_on);
        unsigned int diffcol;
        if (pp==fb && SquareCol(placed_on)==SquareCol(placed_from%onerow))
          diffcol= 1;
        else
          diffcol= 0;

        if (diffcol<=whpc && time<=whmoves)
        {
          SetPiece(pp,placed_on,white[placed_index].sp);
          if (!is_black_king_attacked_by_non_king_too_often())
            mate_store_target_position(blmoves,whmoves-time,blpc,whpc-diffcol,n);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_white_officer_on(unsigned int placed_index,
                                        piece placed_type, square placed_on,
                                        unsigned int blmoves, unsigned int whmoves,
                                        unsigned int blpc, unsigned int whpc,
                                        stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const placed_from = white[placed_index].sq;
    unsigned int const time= count_nr_of_moves_from_to_no_check(placed_type,
                                                                placed_from,
                                                                placed_type,
                                                                placed_on);
    if (time<=whmoves)
    {
      Flags const placed_flags = white[placed_index].sp;
      SetPiece(placed_type,placed_on,placed_flags);
      if (!is_black_king_attacked_by_non_king_too_often())
        mate_store_target_position(blmoves,whmoves-time,blpc,whpc,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_place_any_white_piece_on(square placed_on,
                                          unsigned int blmoves, unsigned int whmoves,
                                          unsigned int blpc, unsigned int whpc,
                                          stip_length_type n)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
    if (!white[placed_index].used)
    {
      piece const placed_type = white[placed_index].p;

      white[placed_index].used = true;

      if (placed_type==pb)
      {
        if (placed_on<=square_h7)
          mate_place_unpromoted_white_pawn_on(placed_index,placed_on,
                                              blmoves,whmoves,
                                              blpc,whpc,
                                              n);
        mate_place_promoted_white_pawn_on(placed_index,placed_on,
                                          blmoves,whmoves,
                                          blpc,whpc,
                                          n);
      }
      else
        mate_place_white_officer_on(placed_index,placed_type,placed_on,
                                    blmoves,whmoves,
                                    blpc,whpc,
                                    n);

      white[placed_index].used = false;
    }

  e[placed_on]= vide;
  spec[placed_on]= EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_neutralise_guarding_pieces(unsigned int blmoves, unsigned int whmoves,
                                            unsigned int blpc, unsigned int whpc,
                                            stip_length_type n)
{
  square trouble = initsquare;
  square trto = initsquare;
#if !defined(NDEBUG)
  has_solution_type search_result;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpc);
  TraceFunctionParam("%u",whpc);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  init_legal_move_finder();

#if !defined(NDEBUG)
  search_result =
#endif
  slice_has_solution(slices[temporary_hack_legal_move_finder[Black]].u.fork.fork);
  assert(search_result==has_solution);
  if (legal_move_finder_departure==initsquare)
    FtlMsg(ErrUndef);
  trouble = legal_move_finder_departure;
  trto = legal_move_finder_arrival;

  fini_legal_move_finder();

  mate_pin_black_piece(trouble,blmoves,whmoves,blpc,whpc,n);

  if (is_rider(abs(e[trouble])))
  {
    int const dir = CheckDirQueen[trto-trouble];

    square sq;
    for (sq = trouble+dir; sq!=trto; sq+=dir)
    {
      mate_place_any_black_piece_on(sq,blmoves,whmoves,blpc,whpc,n);
      mate_place_any_white_piece_on(sq,blmoves,whmoves,blpc,whpc,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int MovesToBlock(square sq, unsigned int blmoves)
{
  int mintime = maxply+1;

  unsigned int i;
  for (i = 1; i<MaxPiece[Black]; ++i)
  {
    piece const p = black[i].p;

    if (p!=-Pawn || sq>=square_a2)
    {
      int const time = count_nr_of_moves_from_to_no_check(p,black[i].sq,p,sq);
      if (time<mintime)
        mintime = time;
    }

    /* pawn promotions */
    if (p==-Pawn)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int moves = black[i].sq>=square_a7 ? 5 : black[i].sq/onerow - nr_of_slack_rows_below_board;
      assert(moves<=5);

      if (sq>=square_a2)
        ++moves;

      if (blmoves>=moves)
      {
        piece pp;
        for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
        {
          int const time = count_nr_of_moves_from_to_no_check(p,black[i].sq,-pp,sq);
          if (time<mintime)
            mintime = time;
        }
      }
    }
  }

  return mintime;
} /* MovesToBlock */

static void finalise_blocking(unsigned int whmoves,
                              unsigned int blpcallowed, unsigned int whpcallowed,
                              unsigned int timetowaste,
                              stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",timetowaste);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (goal_to_be_reached==goal_stale)
  {
    assert(!are_kings_too_close());
    assert(king_square[White]==initsquare || !is_white_king_attacked_by_non_king());
    if (echecc(nbply,Black))
      stalemate_avoid_check(timetowaste,whmoves,blpcallowed,whpcallowed,n);
    else if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
      stalemate_store_target_position(timetowaste,whmoves,blpcallowed,whpcallowed,n);
    else
      stalemate_immobilise(timetowaste,whmoves,blpcallowed,whpcallowed,n);
  }
  else if (echecc(nbply,Black))
  {
    assert(!are_kings_too_close());
    assert(king_square[White]==initsquare || !is_white_king_attacked_by_non_king());
    mate_store_target_position(timetowaste,whmoves,blpcallowed,whpcallowed,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_flights(unsigned int whmoves,
                             unsigned int nr_flights,
                             square toblock[8],
                             unsigned int mintime[8],
                             unsigned int blpcallowed, unsigned int whpcallowed,
                             unsigned int timetowaste,
                             stip_length_type n);

static void block_one_flight_officer(square to_be_blocked,
                                     piece blocker_type,
                                     Flags blocker_flags,
                                     square blocks_from,
                                     unsigned int whmoves,
                                     unsigned int nr_flights,
                                     square toblock[8],
                                     unsigned int mintime[8],
                                     unsigned int blpcallowed,
                                     unsigned int whpcallowed,
                                     unsigned int timetowaste,
                                     stip_length_type n)
{
  unsigned int const current_flight = nr_flights-1;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",nr_flights);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",timetowaste);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,blocks_from,blocker_type,to_be_blocked);
    TraceValue("%u\n",mintime[current_flight]);
    if (time>=mintime[current_flight])
    {
      unsigned int const wasted = time-mintime[current_flight];
      if (wasted<=timetowaste && !attacks_white_king(to_be_blocked,blocker_type))
      {
        SetPiece(blocker_type,to_be_blocked,blocker_flags);
        block_flights(whmoves,
                      nr_flights-1,toblock,mintime,blpcallowed,
                      whpcallowed,timetowaste-wasted,n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_one_flight_pawn_no_prom(square to_be_blocked,
                                          Flags blocker_flags,
                                          square blocks_from,
                                          unsigned int whmoves,
                                          unsigned int nr_flights,
                                          square toblock[8],
                                          unsigned int mintime[8],
                                          unsigned int blpcallowed,
                                          unsigned int whpcallowed,
                                          unsigned int timetowaste,
                                          stip_length_type n)
{
  unsigned int const current_flight = nr_flights-1;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",nr_flights);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",timetowaste);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(pn,blocks_from,pn,to_be_blocked);
    TraceValue("%u\n",mintime[current_flight]);
    if (time>=mintime[current_flight])
    {
      unsigned int const wasted = time-mintime[current_flight];
      if (wasted<=timetowaste && !attacks_white_king(to_be_blocked,pn))
      {
        unsigned int const diffcol = abs(blocks_from%onerow - to_be_blocked%onerow);
        SetPiece(pn,to_be_blocked,blocker_flags);
        if (diffcol<=blpcallowed)
          block_flights(whmoves,nr_flights-1,
                        toblock,mintime,
                        blpcallowed-diffcol,whpcallowed,
                        timetowaste-wasted,n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_one_flight_with_prom(square to_be_blocked,
                                       square blocks_from,
                                       Flags blocker_flags,
                                       unsigned int whmoves,
                                       unsigned int nr_flights,
                                       square toblock[8],
                                       unsigned int mintime[8],
                                       unsigned int blpcallowed,
                                       unsigned int whpcallowed,
                                       unsigned int timetowaste,
                                       stip_length_type n)
{
  unsigned int const current_flight = nr_flights-1;
  unsigned int nr_moves_guesstimate = blocks_from/onerow - nr_of_slack_rows_below_board;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceSquare(blocks_from);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",nr_flights);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",timetowaste);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* A rough check whether it is worth thinking about promotions */
  if (nr_moves_guesstimate>5)
    nr_moves_guesstimate = 5;
  if (to_be_blocked>=square_a2)
    ++nr_moves_guesstimate;

  TraceValue("%u",nr_moves_guesstimate);
  TraceValue("%u\n",mintime[current_flight]);
  if (timetowaste+mintime[current_flight]>=nr_moves_guesstimate)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(pn,
                                                                   blocks_from,
                                                                   -pp,
                                                                   to_be_blocked);
      TraceValue("%u\n",mintime[current_flight]);
      if (time>=mintime[current_flight])
      {
        unsigned int const wasted = time-mintime[current_flight];
        unsigned int diffcol = 0;
        if (pp==fn)
        {
          unsigned int const placed_from_file = blocks_from%nr_files_on_board;
          square const promotion_square_on_same_file = square_a1+placed_from_file;
          if (SquareCol(to_be_blocked)!=SquareCol(promotion_square_on_same_file))
            diffcol = 1;
        }
        if (diffcol<=blpcallowed
            && wasted<=timetowaste
            && !attacks_white_king(to_be_blocked,pp))
        {
          SetPiece(pp,to_be_blocked,blocker_flags);
          block_flights(whmoves,
                        nr_flights-1,toblock,mintime,
                        blpcallowed-diffcol,whpcallowed,
                        timetowaste-wasted,n);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_last_flight(unsigned int whmoves, unsigned int nr_flights,
                              square toblock[8], unsigned int mintime[8],
                              unsigned int blpcallowed, unsigned int whpcallowed,
                              unsigned int timetowaste, stip_length_type n)
{
  unsigned int index_of_current_blocker;
  unsigned int const current_flight = nr_flights-1;
  square const to_be_blocked = toblock[current_flight];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",nr_flights);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",timetowaste);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceSquare(to_be_blocked);TraceText("\n");
  for (index_of_current_blocker = 1;
       index_of_current_blocker<MaxPiece[Black];
       index_of_current_blocker++)
    if (!black[index_of_current_blocker].used)
    {
      piece const blocker_type = black[index_of_current_blocker].p;
      square const blocks_from = black[index_of_current_blocker].sq;
      Flags const blocker_flags = black[index_of_current_blocker].sp;

      black[index_of_current_blocker].used = true;

      if (blocker_type==pn)
      {
        if (to_be_blocked>=square_a2)
          block_one_flight_pawn_no_prom(to_be_blocked,blocker_flags,blocks_from,
                                        whmoves,nr_flights,
                                        toblock,mintime,
                                        blpcallowed,whpcallowed,
                                        timetowaste,n);

        block_one_flight_with_prom(to_be_blocked,blocks_from,blocker_flags,
                                   whmoves,nr_flights,
                                   toblock,mintime,
                                   blpcallowed,whpcallowed,
                                   timetowaste,n);
      }
      else
        block_one_flight_officer(to_be_blocked,
                                 blocker_type,blocker_flags,blocks_from,
                                 whmoves,nr_flights,
                                 toblock,mintime,
                                 blpcallowed,whpcallowed,
                                 timetowaste,n);

      black[index_of_current_blocker].used = false;
    }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_flights(unsigned int whmoves,
                          unsigned int nr_flights,
                          square toblock[8], unsigned int mintime[8],
                          unsigned int blpcallowed, unsigned int whpcallowed,
                          unsigned int timetowaste, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",nr_flights);
  TraceFunctionParam("%u",blpcallowed);
  TraceFunctionParam("%u",whpcallowed);
  TraceFunctionParam("%u",timetowaste);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase())
  {
    /* nothing */
  }
  else if (nr_flights==0)
    finalise_blocking(whmoves,blpcallowed,whpcallowed,timetowaste,n);
  else
    block_last_flight(whmoves,nr_flights,
                      toblock,mintime,
                      blpcallowed,whpcallowed,
                      timetowaste,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int count_black_flights(square toblock[8],
                                        unsigned int min_nr_white_captures)
{
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_nr_white_captures);
  TraceFunctionParamListEnd();

  genmove(Black);
  while(encore() && min_nr_white_captures+result<MaxPiece[Black])
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      if (goal_to_be_reached==goal_stale)
        e[move_generation_stack[nbcou].departure] = obs;

      if (!echecc(nbply,Black))
      {
        if (pprise[nbply]==vide)
        {
          toblock[result] = move_generation_stack[nbcou].arrival;
          ++result;
        }
        else
          result = MaxPiece[Black];
      }
    }

    repcoup();
  }
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static int count_max_nr_allowed_black_pawn_captures(void)
{
  int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; ++i)
    if (!white[i].used)
      ++result;


  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_min_nr_black_moves_for_blocks(unsigned int blmoves,
                                                        unsigned int nr_flights,
                                                        square const toblock[8],
                                                        unsigned  int mintime[8])
{
  unsigned int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_flights && result<=blmoves; ++i)
  {
    mintime[i] = MovesToBlock(toblock[i],blmoves);
    result += mintime[i];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static void fix_white_king_on_diagram_square(unsigned int whmoves, unsigned int blmoves,
                                             unsigned int min_nr_white_captures,
                                             stip_length_type n)
{
  square const king_diagram_square = white[index_of_king].sq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",min_nr_white_captures);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && move_diff_code[abs(king_square[Black]-king_diagram_square)]>=3)
  {
    square toblock[8];
    unsigned int const nr_flights = count_black_flights(toblock,
                                                        min_nr_white_captures);
    if (min_nr_white_captures+nr_flights<MaxPiece[Black])
    {
      unsigned int mintime[8];
      unsigned int const mtba = count_min_nr_black_moves_for_blocks(blmoves,
                                                                    nr_flights,
                                                                    toblock,
                                                                    mintime);
      if (mtba<=blmoves)
      {
        unsigned int const blpcallowed = count_max_nr_allowed_black_pawn_captures();

        king_square[White] = king_diagram_square;
        SetPiece(roib,king_square[White],white[index_of_king].sp);
        white[index_of_king].used = true;
        block_flights(whmoves,
                      nr_flights,toblock,mintime,blpcallowed,
                      MaxPiece[Black]-1-min_nr_white_captures,blmoves-mtba,n);
        white[index_of_king].used = false;
        e[king_square[White]] = vide;
        spec[king_square[White]] = EmptySpec;
        king_square[White] = initsquare;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void FinaliseGuarding(unsigned int whmoves, unsigned int blmoves,
                             unsigned int min_nr_white_captures,
                             stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",min_nr_white_captures);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!white[index_of_king].used
      && white[index_of_king].sq!=square_e1
      && whmoves==0)
    fix_white_king_on_diagram_square(whmoves,blmoves,min_nr_white_captures,n);
  else
  {
    square toblock[8];
    unsigned int const nr_flights = count_black_flights(toblock,min_nr_white_captures);

    if (min_nr_white_captures+nr_flights<MaxPiece[Black])
    {
      unsigned int mintime[8];
      unsigned int const mtba = count_min_nr_black_moves_for_blocks(blmoves,
                                                                    nr_flights,
                                                                    toblock,
                                                                    mintime);
      if (mtba<=blmoves)
      {
        int const blpcallowed = count_max_nr_allowed_black_pawn_captures();
        block_flights(whmoves,
                      nr_flights,toblock,mintime,blpcallowed,
                      MaxPiece[Black]-1-min_nr_white_captures,blmoves-mtba,n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_non_king(unsigned int index_of_first_guarding_piece,
                                   unsigned int whmoves, unsigned int blmoves,
                                   unsigned int min_nr_white_captures,
                                   stip_length_type n);

static void guard_flight_unpromoted_pawn(unsigned int index,
                                         unsigned int whmoves, unsigned int blmoves,
                                         unsigned int min_nr_white_captures,
                                         stip_length_type n)
{
  Flags const pawn_flags = white[index].sp;
  square const guard_from = white[index].sq;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index);
  TraceValue("%u",whmoves);
  TraceValue("%u",blmoves);
  TraceValue("%u",min_nr_white_captures);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide)
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(pb,
                                                                   guard_from,
                                                                   pb,
                                                                   *bnp);
      if (time<=whmoves && guards_black_flight(pb,*bnp)
          && !(index<index_of_piece_delivering_check
               && guards(king_square[Black],pb,*bnp)))
      {
        SetPiece(pb,*bnp,pawn_flags);
        if (!is_black_king_attacked_by_non_king_too_often())
        {
          unsigned int const diffcol = abs(guard_from % onerow - *bnp % onerow);
          guard_flights_non_king(index+1,
                                 whmoves-time,blmoves,
                                 min_nr_white_captures+diffcol,n);
        }

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flight_promoted_pawn(unsigned int index,
                                       unsigned int whmoves, unsigned int blmoves,
                                       unsigned int min_nr_white_captures,
                                       stip_length_type n)
{
  Flags const pawn_flags = white[index].sp;
  square const guard_from = white[index].sq;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index);
  TraceValue("%u",whmoves);
  TraceValue("%u",blmoves);
  TraceValue("%u",min_nr_white_captures);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (*bnp<=square_h7
                                              ? moves_to_prom[index]+1
                                              : moves_to_prom[index]);
      if (whmoves>=min_nr_moves_by_p)
      {
        piece pp;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        {
          unsigned int const time = count_nr_of_moves_from_to_no_check(pb,
                                                                       guard_from,
                                                                       pp,
                                                                       *bnp);
          if (time<=whmoves
              && guards_black_flight(pp,*bnp)
              && !(index<index_of_piece_delivering_check
                   && guards(king_square[Black],pp,*bnp)))
          {
            SetPiece(pp,*bnp,pawn_flags);
            if (!is_black_king_attacked_by_non_king_too_often())
              guard_flights_non_king(index+1,
                                     whmoves-time,blmoves,
                                     min_nr_white_captures,n);
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

static void guard_flight_officer(unsigned int index,
                                 piece guard_type,
                                 unsigned int whmoves, unsigned int blmoves,
                                 unsigned int min_nr_white_captures,
                                 stip_length_type n)
{
  Flags const guard_flags = white[index].sp;
  square const guard_from = white[index].sq;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index);
  TracePiece(guard_type);
  TraceValue("%u",whmoves);
  TraceValue("%u",blmoves);
  TraceValue("%u",min_nr_white_captures);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    TraceSquare(*bnp);TraceText("\n");
    if (e[*bnp]==vide)
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(guard_type,
                                                                   guard_from,
                                                                   guard_type,
                                                                   *bnp);
      if (time<=whmoves && guards_black_flight(guard_type,*bnp)
          && !(index<index_of_piece_delivering_check
               && guards(king_square[Black],guard_type,*bnp)))
      {
        SetPiece(guard_type,*bnp,guard_flags);
        if (!is_black_king_attacked_by_non_king_too_often())
          guard_flights_non_king(index+1,
                                 whmoves-time,blmoves,
                                 min_nr_white_captures,n);

        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_non_king(unsigned int index_of_first_guarding_piece,
                                   unsigned int whmoves, unsigned int blmoves,
                                   unsigned int min_nr_white_captures,
                                   stip_length_type n)
{
  unsigned int index_of_current_guarding_piece;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_first_guarding_piece);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",min_nr_white_captures);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(index_of_first_guarding_piece>index_of_king);

  if (!max_nr_solutions_found_in_phase()
      && min_nr_white_captures<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
  {
    for (index_of_current_guarding_piece = index_of_first_guarding_piece;
         index_of_current_guarding_piece<MaxPiece[White];
         ++index_of_current_guarding_piece)
      if (index_of_current_guarding_piece!=index_of_piece_delivering_check)
      {
        piece const guard_type = white[index_of_current_guarding_piece].p;
        white[index_of_current_guarding_piece].used = true;

        if (guard_type==pb)
        {
          guard_flight_unpromoted_pawn(index_of_current_guarding_piece,
                                       whmoves,blmoves,
                                       min_nr_white_captures,n);
          guard_flight_promoted_pawn(index_of_current_guarding_piece,
                                     whmoves,blmoves,
                                     min_nr_white_captures,n);
        }
        else
          guard_flight_officer(index_of_current_guarding_piece,
                               guard_type,
                               whmoves,blmoves,
                               min_nr_white_captures,n);

        white[index_of_current_guarding_piece].used = false;
      }

    if (goal_to_be_reached==goal_stale || echecc(nbply,Black))
      FinaliseGuarding(whmoves,blmoves,min_nr_white_captures,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void guard_flights_king(unsigned int whmoves, unsigned int blmoves,
                               unsigned int min_nr_white_captures,
                               stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",min_nr_white_captures);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!max_nr_solutions_found_in_phase()
      && min_nr_white_captures<=MaxPiece[Black]-1
      && !hasMaxtimeElapsed())
  {
    square const guard_from = white[index_of_king].sq;
    square const *bnp;

    white[index_of_king].used = true;

    /* try using white king for guarding from every square */
    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (e[*bnp]==vide
          && move_diff_code[abs(king_square[Black]-*bnp)]>=3)
      {
        unsigned int const time = count_nr_of_moves_from_to_no_check(roib,
                                                                     guard_from,
                                                                     roib,
                                                                     *bnp);
        if (time<=whmoves && guards_black_flight(roib,*bnp))
        {
          king_square[White]= *bnp;
          SetPiece(roib,*bnp,white[index_of_king].sp);
          if (!are_kings_too_close())
            guard_flights_non_king(1,whmoves-time,blmoves,min_nr_white_captures,n);
          e[*bnp] = vide;
          spec[*bnp] = EmptySpec;
        }
      }

    king_square[White] = initsquare;
    white[index_of_king].used = false;

    /* try not using white king for guarding */
    guard_flights_non_king(1,whmoves,blmoves,min_nr_white_captures,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move_by_one_promoted_pawn(unsigned int whmoves,
                                                             unsigned int blmoves,
                                                             stip_length_type n)
{
  Flags const checker_flags = white[index_of_piece_delivering_check].sp;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const check_from = boardnum[i];
    TraceSquare(check_from);
    TracePiece(e[check_from]);
    TraceText("\n");
    if (e[check_from]==vide)
    {
      /* A rough check whether it is worth thinking about promotions */
      unsigned int const min_nr_moves_by_p = (check_from<=square_h7
                                              ? moves_to_prom[index_of_piece_delivering_check]+1
                                              : moves_to_prom[index_of_piece_delivering_check]);
      if (whmoves>=min_nr_moves_by_p)
      {
        piece pp;
        square const pawn_origin = white[index_of_piece_delivering_check].sq;
        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        {
          unsigned int const time= count_nr_of_moves_from_to_no_check(pb,
                                                                      pawn_origin,
                                                                      pp,
                                                                      check_from);
          if (time<=whmoves && guards(king_square[Black],pp,check_from))
          {
            piecechecking = pp;
            squarechecking = check_from;
            SetPiece(pp,check_from,checker_flags);
            guard_flights_king(whmoves-time,blmoves,0,n);
          }
        }
      }

      e[check_from] = vide;
      spec[check_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move_by_one_unpromoted_pawn(unsigned int whmoves,
                                                               unsigned int blmoves,
                                                               stip_length_type n)
{
  Flags const checker_flags = white[index_of_piece_delivering_check].sp;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const check_from = boardnum[i];
    TraceSquare(check_from);
    TracePiece(e[check_from]);
    TraceText("\n");
    if (e[check_from]==vide)
    {
      square const pawn_origin = white[index_of_piece_delivering_check].sq;
      unsigned int const time = count_nr_of_moves_from_to_checking(pb,
                                                                   pawn_origin,
                                                                   pb,
                                                                   check_from);
      if (time<=whmoves && guards(king_square[Black],pb,check_from))
      {
        int diffcol = pawn_origin%onerow - check_from%onerow;
        SetPiece(pb,check_from,checker_flags);
        piecechecking = pb;
        squarechecking = check_from;
        guard_flights_king(whmoves-time,blmoves,abs(diffcol),n);
      }

      e[check_from] = vide;
      spec[check_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move_by_one_officer(piece checker_type,
                                                       unsigned int whmoves,
                                                       unsigned int blmoves,
                                                       stip_length_type n)
{
  Flags const checker_flags = white[index_of_piece_delivering_check].sp;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TracePiece(checker_type);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const check_from = boardnum[i];
    TraceSquare(check_from);
    TracePiece(e[check_from]);
    TraceText("\n");
    if (e[check_from]==vide)
    {
      square const checker_origin = white[index_of_piece_delivering_check].sq;
      unsigned int const time = count_nr_of_moves_from_to_checking(checker_type,
                                                                   checker_origin,
                                                                   checker_type,
                                                                   check_from);
      if (time<=whmoves && guards(king_square[Black],checker_type,check_from))
      {
        SetPiece(checker_type,check_from,checker_flags);
        piecechecking = checker_type;
        squarechecking = check_from;
        guard_flights_king(whmoves-time,blmoves,0,n);
      }

      e[check_from] = vide;
      spec[check_from] = EmptySpec;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void mate_generate_checking_move(unsigned int whmoves,
                                        unsigned int blmoves,
                                        stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",whmoves);
  TraceFunctionParam("%u",blmoves);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (index_of_piece_delivering_check = 1;
       index_of_piece_delivering_check<MaxPiece[White];
       ++index_of_piece_delivering_check)
  {
    piece const checker_type = white[index_of_piece_delivering_check].p;

    TraceValue("%u",index_of_piece_delivering_check);
    TraceSquare(white[index_of_piece_delivering_check].sq);
    TracePiece(checker_type);
    TraceText("\n");

    white[index_of_piece_delivering_check].used = true;

    if (checker_type==pb)
    {
      mate_generate_checking_move_by_one_unpromoted_pawn(whmoves,blmoves,n);
      mate_generate_checking_move_by_one_promoted_pawn(whmoves,blmoves,n);
    }
    else
      mate_generate_checking_move_by_one_officer(checker_type,whmoves,blmoves,n);

    white[index_of_piece_delivering_check].used = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void GenerateBlackKing(stip_length_type n)
{
  int i;
  Flags const king_flags = black[index_of_king].sp;
  unsigned int const whmoves = MovesLeft[White];
  unsigned int const blmoves = MovesLeft[Black];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(black[index_of_king].p==roin);

  for (i = 0; i<nr_squares_on_board && !hasMaxtimeElapsed(); ++i)
  {
    square const king_pos = boardnum[i];
    TraceSquare(king_pos);TraceText("\n");
    if (e[king_pos]!=obs)
    {
      unsigned int const time = count_nr_of_moves_from_to_no_check(roin,black[index_of_king].sq,roin,king_pos);
      if (time<=blmoves)
      {
        SetPiece(roin,king_pos,king_flags);
        king_square[Black] = king_pos;
        if (goal_to_be_reached==goal_mate)
          mate_generate_checking_move(whmoves,blmoves-time,n);
        else
        {
          index_of_piece_delivering_check = 0;
          guard_flights_king(whmoves,blmoves-time,0,n);
        }
        e[king_pos] = vide;
        spec[king_pos] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void IntelligentRegulargoal_types(stip_length_type n)
{
  square const *bnp;
  piece p;

  deposebnp= boardnum;
  is_cast_supp= castling_supported;
  is_ep= ep[1]; is_ep2= ep2[1];
  castling_supported= false;

  SetIndex(spec[king_square[Black]],0);
  black[index_of_king].p= e[king_square[Black]];
  black[index_of_king].sp= spec[king_square[Black]];
  black[index_of_king].sq= king_square[Black];
  MaxPiece[Black]= 1;

  if (king_square[White]==initsquare)
    white[index_of_king].used = true;
  else
  {
    SetIndex(spec[king_square[White]],1);
    white[index_of_king].used = false;
    white[index_of_king].p = e[king_square[White]];
    white[index_of_king].sp = spec[king_square[White]];
    white[index_of_king].sq = king_square[White];
    assert(white[index_of_king].p==roib);
  }

  MaxPiece[White]= 1;
  MaxPieceAll = 2;

  for (bnp= boardnum; *bnp; bnp++)
    if (king_square[White]!=*bnp && e[*bnp]>obs)
    {
      SetIndex(spec[*bnp],MaxPieceAll);
      white[MaxPiece[White]].p= e[*bnp];
      white[MaxPiece[White]].sp= spec[*bnp];
      white[MaxPiece[White]].sq= *bnp;
      white[MaxPiece[White]].used= false;
      if (e[*bnp]==pb)
      {
        unsigned int moves= 15 - *bnp / onerow;
        square  sq = *bnp;
        if (moves > 5)
          moves= 5;

        /* a white piece that cannot move away */
        if (moves==5
            && moves==MovesLeft[White]
            && sq<=square_h2
            && (e[sq+dir_up]>vide || e[sq+2*dir_up]>vide))
          moves = maxply+1;

        /* a black pawn that needs a white sacrifice to move away */
        else if (MovesLeft[White]<7
                 && sq<=square_h2
                 && e[sq+dir_left]<=roib && e[sq+dir_right]<=roib
                 && (e[sq+dir_up]==pn
                     || (e[sq+dir_up+dir_left]<=roib
                         && e[sq+dir_up+dir_right]<=roib
                         && (ep[1]!=sq+dir_up+dir_left)
                         && (ep[1]!=sq+dir_up+dir_right)
                         && e[sq+2*dir_up]==pn)))
          ++moves;

        moves_to_prom[MaxPiece[White]] = moves;
      }
      ++MaxPiece[White];
      ++MaxPieceAll;
    }

  for (bnp= boardnum; *bnp; bnp++) {
    if ((king_square[Black] != *bnp) && (e[*bnp] < vide)) {
      SetIndex(spec[*bnp],MaxPieceAll);
      black[MaxPiece[Black]].p= e[*bnp];
      black[MaxPiece[Black]].sp= spec[*bnp];
      black[MaxPiece[Black]].sq= *bnp;
      black[MaxPiece[Black]].used= false;
      MaxPiece[Black]++;
      MaxPieceAll++;
    }
  }

  StorePosition();
  ep[1]= ep2[1]= initsquare;

  /* clear board */
  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] != obs) {
      e[*bnp]= vide;
      spec[*bnp]= EmptySpec;
    }
  }

  for (p= roib; p <= fb; p++) {
    nbpiece[-p]= nbpiece[p]= 2;
  }

  /* generate final positions */
  GenerateBlackKing(n);

  ResetPosition();

  if (OptFlag[movenbr]
      && !hasMaxtimeElapsed())
  {
    StdString("\n");
    sprintf(GlobalStr,"%lu %s %u+%u",
            MatesMax,GetMsgString(PotentialMates),
            MovesLeft[White],MovesLeft[Black]);
    StdString(GlobalStr);
    if (!flag_regression) {
      StdString("  (");
      PrintTime();
      StdString(")");
    }
  }

  castling_supported= is_cast_supp;
  ep[1]= is_ep; ep2[1]= is_ep2;
}

static void IntelligentProof(stip_length_type n)
{
  boolean const save_movenbr = OptFlag[movenbr];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  ProofInitialiseIntelligent(n);

  /* Proof games and a=>b are special because there is only 1 end
   * position to be reached. We therefore output move numbers as if
   * we were not in intelligent mode, and only if we are solving
   * full-length.
   */
  OptFlag[movenbr] = false;

  if (help(slices[current_start_slice].u.pipe.next,n)<=n)
    solutions_found = true;

  OptFlag[movenbr] = save_movenbr;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Calculate the number of moves of each side
 * @param si index of non-root slice
 * @param st address of structure defining traversal
 */
static void moves_left_move(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++MovesLeft[advers(slices[si].starter)];

  TraceValue("%u",MovesLeft[White]);
  TraceValue("%u\n",MovesLeft[Black]);

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void moves_left_zigzag(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves(slices[si].u.binary.op1,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Calculate the number of moves of each side, starting at the root
 * slice.
 * @param si identifies starting slice
 * @param n length of the solution(s) we are looking for (without slack)
 * @param full_length full length of the initial branch (without slack)
 */
static void init_moves_left(slice_index si,
                            stip_length_type n,
                            stip_length_type full_length)
{
  stip_moves_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",full_length);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  MovesLeft[Black] = 0;
  MovesLeft[White] = 0;

  stip_moves_traversal_init(&st,&n);
  st.context = stip_traversal_context_help;
  stip_moves_traversal_set_remaining(&st,n,full_length);
  stip_moves_traversal_override_single(&st,
                                       STGoalReachableGuardFilter,
                                       &moves_left_move);
  stip_moves_traversal_override_single(&st,STCheckZigzagJump,moves_left_zigzag);
  stip_traverse_moves(si,&st);

  TraceValue("%u",MovesLeft[White]);
  TraceValue("%u\n",MovesLeft[Black]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void goal_to_be_reached_goal(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(goal_to_be_reached==no_goal);
  goal_to_be_reached = slices[si].u.goal_tester.goal.type;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the variable holding the goal to be reached
 */
static void init_goal_to_be_reached(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  goal_to_be_reached = no_goal;

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &goal_to_be_reached_goal);
  stip_structure_traversal_override_single(&st,
                                           STTemporaryHackFork,
                                           &stip_traverse_structure_pipe);
  stip_traverse_structure(si,&st);

  TraceValue("%u",goal_to_be_reached);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a STGoalReachableGuardFilter slice
 * @return identifier of allocated slice
 */
static slice_index alloc_goalreachable_guard_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalReachableGuardFilter);

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
stip_length_type goalreachable_guard_help(slice_index si, stip_length_type n)
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

  if (isGoalReachable())
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
stip_length_type goalreachable_guard_can_help(slice_index si,
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

  if (isGoalReachable())
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_goalreachable_guard_filter();
    help_branch_insert_slices(si,&prototype,1);
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
static void stip_insert_goalreachable_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_intelligent_filter();
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
static void stip_insert_intelligent_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    intelligent_filters_inserters,
                                    nr_intelligent_filters_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean too_short(stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (OptFlag[restart])
  {
    stip_length_type min_length = 2*get_restart_number();
    if ((n-slack_length_help)%2==1)
      --min_length;
    result = n-slack_length_help<min_length;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean Intelligent(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  init_moves_left(si,n-slack_length_help,n-slack_length_help);

  if (MovesLeft[White]+MovesLeft[Black]>0)
  {
    current_start_slice = si;
    MatesMax = 0;
    solutions_found = false;

    if (goal_to_be_reached==goal_atob
        || goal_to_be_reached==goal_proofgame)
      IntelligentProof(n);
    else
    {
      intelligent_duplicate_avoider_init();
      if (!too_short(n))
        IntelligentRegulargoal_types(n);
      intelligent_duplicate_avoider_cleanup();
    }

    result = solutions_found;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean areCapturesPossible(void)
{
  CapturesLeft[nbply] = CapturesLeft[nbply-1];
  if (pprise[nbply]<vide)
    --CapturesLeft[nbply];
  return MovesLeft[White]>=CapturesLeft[nbply];
}

boolean isGoalReachable(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (goal_to_be_reached)
  {
    case goal_atob:
    case goal_proofgame:
      result = !(*alternateImpossible)();
      break;

    case goal_mate:
      result = isGoalReachableRegularGoals();
      break;

    case goal_stale:
      result = isGoalReachableRegularGoals() && areCapturesPossible();
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
        stip_insert_intelligent_filters(si);
        stip_insert_goalreachable_guards(si);
        init_goal_to_be_reached(si);
      }
      break;

    case intelligent_active_by_default:
      result = true;
      stip_insert_intelligent_filters(si);
      stip_insert_goalreachable_guards(si);
      init_goal_to_be_reached(si);
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

/* Determine whether intelligent mode overrides option movenum
 * @return true iff intelligent mode overrides option movenum
 */
boolean intelligent_mode_overrides_movenbr(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (goal_to_be_reached==goal_atob
      || goal_to_be_reached==goal_proofgame
      || goal_to_be_reached==no_goal)
    result = false;
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
