/******************** MODIFICATIONS to pyint.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/06/14 TLi  bug fix in function impact()
 **
 ** 2007/12/27 TLi  bug fix in function Immobilize()
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
#include "pybrah.h"
#include "pyproof.h"
#include "platform/maxtime.h"
#include "trace.h"

#if defined(DEBUG)
#include "pyoutput.h"
#endif

#define SetIndex(s, f)                                  \
  (s = (Flags)(s&((1<<DiaCirce)-1)) + (f<<DiaCirce))

#define GetIndex(s)                             \
  ((int)((s >> DiaCirce)))

typedef struct {
    square  sq;
    Flags   sp;
    piece   p;
} PIECE;

typedef struct {
    square  from;
    square  to;
    piece   prom;
} MOVE;

boolean isIntelligentModeActive;

static Goal goal_to_be_reached;

int MaxPieceAll, MaxPieceWhite, MaxPieceBlack;
int WhMovesLeft, BlMovesLeft;

long MatesMax;

PIECE white[nr_squares_on_board], black[nr_squares_on_board], final[nr_squares_on_board];
boolean whiteused[nr_squares_on_board], blackused[nr_squares_on_board], is_cast_supp;
square is_ep, is_ep2;
int moves_to_prom[nr_squares_on_board];
square squarechecking;
square const *deposebnp;
piece piecechecking;
int nbrchecking;

static MOVE **Sols;
static int SolMax;

PIECE Mate[nr_squares_on_board];
int IndxChP;

static slice_index current_start_slice;

#define SetPiece(P, SQ, SP) {e[SQ]= P; spec[SQ]= SP;}

static boolean guards(square bk, piece p, square sq)
{
  int diff = bk-sq;
  int dir= 0;

  switch (p)
  {
    case Pawn:
      return (sq>=square_a2
              && (diff==+dir_up+dir_left || diff==+dir_up+dir_right));

    case Knight:
      return (CheckDirKnight[diff] != 0);

    case Bishop:
      dir= CheckDirBishop[diff];
      break;

    case Rook:
      dir= CheckDirRook[diff];
      break;

    case Queen:
      dir= CheckDirBishop[diff];
      if (dir == 0)
        dir= CheckDirRook[diff];
      break;

    case King:
      return ((move_diff_code[abs(diff)]) < 3);

    default:
      break;
  }

  if (dir!=0)
  {
    square tmp = sq;
    do
    {
      tmp += dir;
      if (tmp==bk)
        return true;
    } while (e[tmp] == vide);
  }

  return false;
} /* guards */

static boolean IllegalCheck(Side camp)
{
  if (rb!=initsquare && move_diff_code[abs(rb-rn)]<3)
    return true;

  if (camp == Black)
  {
    int nrChecks= 0;
    numvec k;
    for (k = vec_rook_start; k<=vec_rook_end; k++)
      if (e[rn+vec[k]]==Rook || e[rn+vec[k]]==Queen)
        nrChecks++;

    for (k = vec_bishop_start; k<=vec_bishop_end; k++)
      if (e[rn+vec[k]]==Bishop || e[rn+vec[k]]==Queen)
        nrChecks++;

    for (k = vec_knight_start; k<=vec_knight_end; k++)
      if (e[rn+vec[k]] == Knight)
        nrChecks++;

    if (e[rn+dir_down+dir_right]==Pawn || e[rn+dir_down+dir_left]==Pawn)
      nrChecks++;

    return (nrChecks > (goal_to_be_reached==goal_stale ? 0: 1));
  }
  else
    return (rb!=initsquare
            && ((*checkfunctions[Pawn])( rb, pn, eval_ortho)
                || (*checkfunctions[Knight])( rb, cn, eval_ortho)
                || (*checkfunctions[Fers])( rb, fn, eval_ortho)
                || (*checkfunctions[Wesir])( rb, tn, eval_ortho)
                || (*checkfunctions[ErlKing])( rb, dn, eval_ortho)));
}

static boolean impact(square bk, piece p, square sq) {
  int   i;
  boolean   ret= guards(bk, p, sq);

  e[bk]= vide;
  for (i= 8; i && !ret; i--) {
    if (e[bk+vec[i]] != obs && guards(bk+vec[i], p, sq)) {
      ret= true;
    }
  }
  e[bk]= roin;

  return ret;
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

static int FroTo(
  piece f_p,
  square    f_sq,
  piece t_p,
  square    t_sq,
  boolean genchk)
{
  int diffcol, diffrow, withcast;

  if (f_sq == t_sq && f_p == t_p) {
    if (genchk) {
      if (f_p == Pawn) {
        return maxply+1;
      }
      if (f_p == Knight) {
        return 2;
      }
      /* it's a rider */
      if (move_diff_code[abs(rn-t_sq)] < 3) {
        return 2;
      }
    }
    return 0;
  }

  switch (abs(f_p)) {
  case Knight:
    return ProofKnightMoves[abs(f_sq-t_sq)];

  case Rook:
    if (CheckDirRook[f_sq-t_sq])
      return 1;
    else
      return 2;

  case Queen:
    if (CheckDirRook[f_sq-t_sq] || CheckDirBishop[f_sq-t_sq])
      return 1;
    else
      return 2;

  case Bishop:
    if (SquareCol(f_sq) != SquareCol(t_sq))
      return maxply+1;
    if (CheckDirBishop[f_sq-t_sq])
      return 1;
    else
      return 2;

  case King:
  {
    int minmoves= FroToKing(f_sq, t_sq);
    /* castling */
    if (testcastling) {
      if (f_p == King) {
        /* white king */
        if (f_sq == square_e1) {
          if (TSTFLAGMASK(castling_flag[nbply],ra1_cancastle&no_castling)) {
            withcast= FroToKing(square_c1, t_sq);
            if (withcast < minmoves) {
              minmoves= withcast;
            }
          }
          if (TSTFLAGMASK(castling_flag[nbply],rh1_cancastle&no_castling)) {
            withcast= FroToKing(square_g1, t_sq);
            if (withcast < minmoves) {
              minmoves= withcast;
            }
          }
        }
      }
      else {
        /* black king */
        if (f_sq == square_e8) {
          if (TSTFLAGMASK(castling_flag[nbply],ra8_cancastle&no_castling)) {
            withcast= FroToKing(square_c8, t_sq);
            if (withcast < minmoves) {
              minmoves= withcast;
            }
          }
          if (TSTFLAGMASK(castling_flag[nbply],rh8_cancastle&no_castling)) {
            withcast= FroToKing(square_g8, t_sq);
            if (withcast < minmoves) {
              minmoves= withcast;
            }
          }
        }
      }
    }
    return minmoves;
  }

  case Pawn:
    if (f_p == t_p) {
      diffcol= f_sq % onerow - t_sq % onerow;
      if (diffcol < 0) {
        diffcol= -diffcol;
      }
      diffrow= f_sq / onerow - t_sq / onerow;
      if (f_p < vide) {
        /* black pawn */
        if (diffrow < diffcol) {
          /* if diffrow <= 0 then this test is true, since
             diffcol is always positive
          */
          return maxply+1;
        }
        if (f_sq>=square_a7 && diffrow > 1) {
          /* double step */
          if (diffrow-2 >= diffcol) {
            diffrow--;
          }
        }
        return diffrow;
      }
      else {
        /* white pawn */
        if (-diffrow < diffcol) {
          return maxply+1;
        }
        if (f_sq<=square_h2
            && diffrow < -1 && -diffrow-2 >= diffcol)
        {
          diffrow++;
        }
        return -diffrow;
      }
    }
    else
    {
      /* promotion */
      int minmoves, curmoves;
      square v_sq, start;

      minmoves= maxply+1;
      start= (f_p < vide) ? square_a1 : square_a8;

      for (v_sq= start; v_sq < start+8; v_sq++) {
        curmoves= FroTo(f_p, f_sq, f_p, v_sq, false)
          + FroTo(t_p, v_sq, t_p, t_sq, false);
        if (curmoves < minmoves) {
          minmoves= curmoves;
        }
      }
      return minmoves;
    }
  }
  return 1;
} /* FroTo */

void StoreSol(void) {
  ply cp;

  if (SolMax>0)
    Sols = (MOVE**)realloc(Sols, sizeof(MOVE*)*(SolMax+1));
  else
    Sols = (MOVE**)malloc(sizeof(MOVE*));

  if (Sols == NULL
      || (Sols[SolMax] = (MOVE*)malloc(sizeof(MOVE)*(nbply+1))) == NULL) {
    fprintf(stderr, "Cannot (re)allocate enough memory\n");
    exit(0);
  }

  for (cp= 2; cp <= nbply; cp++) {
    Sols[SolMax][cp].from= move_generation_stack[repere[cp+1]].departure;
    Sols[SolMax][cp].to= move_generation_stack[repere[cp+1]].arrival;
    Sols[SolMax][cp].prom= jouearr[cp];
  }

  SolMax++;

  ++sol_per_matingpos;
}

boolean SolAlreadyFound(void) {
  ply       cp;
  int   cs;
  boolean   found= false;

  repere[nbply+1]= nbcou;
  for (cs= 0; cs < SolMax && !found; cs++) {
    found= true;
    for (cp= 2; cp <= nbply && found; cp++) {
      found= Sols[cs][cp].from == move_generation_stack[repere[cp+1]].departure
        && Sols[cs][cp].to   == move_generation_stack[repere[cp+1]].arrival
        && Sols[cs][cp].prom == jouearr[cp];
    }
  }

#if defined(DEBUG)
  if (found) {
    isIntelligentModeActive= false;
    StdString("solution already found:");
    write_final_help_move(no_goal);
    isIntelligentModeActive= true;
  }
#endif

  return found;
}

int  CurMate;
int WhMovesRequired[maxply+1],
  BlMovesRequired[maxply+1],
  CapturesLeft[maxply+1];

static boolean isGoalReachableRegularGoals(void)
{
  int   whmoves, blmoves, index, time, captures;
  piece f_p;
  square    t_sq;

  captures= CapturesLeft[nbply-1];

  if (sol_per_matingpos>=maxsol_per_matingpos
      || maxtime_status==MAXTIME_TIMEOUT)
  {
    FlagMaxSolsReached= true;
    return false;
  }

  /* check if a piece has been captured that participates
     in the mate
  */
  if (pprise[nbply]) {
    index= GetIndex(pprispec[nbply]);
    if (Mate[index].sq != initsquare) {
      return false;
    }
  }

  if (nbply == 2
      || (testcastling
          && castling_flag[nbply] != castling_flag[nbply-1]))
  {
    square const *bnp;
    whmoves= blmoves= 0;
    for (bnp= boardnum; *bnp; bnp++) {
      square f_sq= *bnp;
      if ( (f_p= e[f_sq]) != vide
           && (f_p != obs))
      {
        index= GetIndex(spec[f_sq]);
        if ((t_sq= Mate[index].sq) != initsquare) {
          if (WhMovesLeft && index == IndxChP) {
            square _rn= rn;
            rn= Mate[GetIndex(spec[rn])].sq;
            time= FroTo(f_p,
                        f_sq, Mate[index].p, t_sq, true);
            rn= _rn;
          }
          else {
            time= FroTo(f_p,
                        f_sq, Mate[index].p, t_sq, false);
          }
          if (f_p > vide)
            whmoves += time;
          else
            blmoves += time;
        }
      }
    }
  }
  else {
    index= GetIndex(jouespec[nbply]);
    t_sq= Mate[index].sq;
    whmoves= WhMovesRequired[nbply-1];
    blmoves= BlMovesRequired[nbply-1];
    if (t_sq != initsquare) {
      /* old time */
      if (index == IndxChP) {
        square _rn= rn;
        rn= Mate[GetIndex(spec[rn])].sq;
        time= -FroTo(pjoue[nbply], move_generation_stack[nbcou].departure,
                     Mate[index].p, t_sq, true);
        rn= _rn;
      }
      else {
        time= -FroTo(pjoue[nbply],
                     move_generation_stack[nbcou].departure, Mate[index].p, t_sq, false);
      }

      /* new time */
      if (index == IndxChP && WhMovesLeft) {
        square _rn= rn;
        rn= Mate[GetIndex(spec[rn])].sq;
        time += FroTo(e[move_generation_stack[nbcou].arrival], move_generation_stack[nbcou].arrival,
                      Mate[index].p, t_sq, true);
        rn= _rn;
      }
      else {
        time += FroTo(e[move_generation_stack[nbcou].arrival],
                      move_generation_stack[nbcou].arrival, Mate[index].p, t_sq, false);
      }
      if (trait[nbply] == White)
        whmoves += time;
      else
        blmoves += time;
    }
  }

  if (goal_to_be_reached == goal_stale) {
    if (pprise[nbply] < vide) {
      captures--;
    }
    if (WhMovesLeft < captures) {
      return false;
    }
  }

  if (whmoves > WhMovesLeft || blmoves > BlMovesLeft)
    return false;

  WhMovesRequired[nbply]= whmoves;
  BlMovesRequired[nbply]= blmoves;
  CapturesLeft[nbply]= captures;

  return true;
} /* isGoalReachableRegularGoals */

/* declarations */
static void ImmobilizeByBlBlock(
    int, int, int, int, square, boolean, stip_length_type n);
static void DeposeBlPiece(int, int, int, int, stip_length_type n);
static void Immobilize(int, int, int, int, stip_length_type n);
static void AvoidCheckInStalemate(int, int, int, int, stip_length_type n);
static int MovesToBlock(square, int);
static void DeposeWhKing(int, int, int, int, stip_length_type n);
static void NeutralizeMateGuardingPieces(int, int, int, int, stip_length_type n);
static void BlackPieceTo(square, int, int, int, int, stip_length_type n);
static void WhitePieceTo(square, int, int, int, int, stip_length_type n);
static void AvoidWhKingInCheck(int, int, int, int, stip_length_type n);

static void StaleStoreMate(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  stip_length_type n)
{
  int   i, index, unused= 0;
  square const *bnp;
  square _rb, _rn;
  Flags sp;

  if (blpcallowed < 0
      || whpcallowed < 0
      || maxtime_status==MAXTIME_TIMEOUT)
  {
    return;
  }

  if (   (rb == initsquare)
         && (white[0].sq != initsquare)
         && (white[0].sq != square_e1)
         && (whmoves == 0))
  {
    DeposeWhKing(blmoves, whmoves, blpcallowed, whpcallowed, n);
    return;
  }

  for (i= 1; i < MaxPieceBlack; i++) {
    if (!blackused[i]) {
      unused++;
    }
  }

  if (unused) {
#if defined(DETAILS)
    WritePosition();
    sprintf(GlobalStr, "unused= %d\n", unused);
    StdString(GlobalStr);
#endif
    DeposeBlPiece(blmoves, whmoves, blpcallowed, whpcallowed, n);
  }
#if defined(DEBUG)
  sprintf(GlobalStr,
          "unused: %d, WhMovesLeft: %d\n", unused, WhMovesLeft);
  StdString(GlobalStr);
#endif

  if (unused > WhMovesLeft)
    return;

  /* checks against the wKing should be coped with earlier !!! */
  if (echecc(nbply,White)) {
    AvoidWhKingInCheck(blmoves, whmoves, blpcallowed, whpcallowed,
                       n); 
  }

  CapturesLeft[1]= unused;

  MatesMax++;

#if defined(DETAILS)
  sprintf(GlobalStr, "mate no. %d\n", MatesMax);
  StdString(GlobalStr);
  WritePosition();
#endif

  for (i= 0; i < MaxPieceAll; i++) {
    Mate[i].sq= initsquare;
  }

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

  IndxChP= nbrchecking == -1
    ? -1
    : GetIndex(white[nbrchecking].sp);
  _rb= rb;
  _rn= rn;

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
          m= FroTo(e[*bnp],
                   *bnp, Mate[index].p, Mate[index].sq, 0);
          if (e[*bnp] < vide)
            blm+= m;
          else
            whm+= m;
          sprintf(GlobalStr, "(%d)  ", m);
          StdString(GlobalStr);
        }
      }
    sprintf(GlobalStr,
            "\nblack moves: %d, white moves: %d\n", blm, whm);
    StdString(GlobalStr);
  }
#endif

  sol_per_matingpos= 0;

  closehash();
  inithash();

  {
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    slice_root_solve_in_n(current_start_slice,n);
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

  for (i= King; i <= Bishop; i++) {
    nbpiece[-i]= nbpiece[i]= 2;
  }

  rb= _rb;
  rn= _rn;

  ep[1]= ep2[1]= initsquare;

  castling_supported= false;
} /* StaleStoreMate */

void write_indentation(void)
{
}

void DeposeBlPiece(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  stip_length_type n)
{
  square const *bnp;
  square const * const isbnp = deposebnp;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "DeposeBlPiece(%d,%d,%d), *deposebnp=%d\n",
          blmoves, whmoves, blpcallowed, *deposebnp);
  StdString(GlobalStr);
#endif

  for (bnp= deposebnp; *bnp; bnp++) {
    if (e[*bnp] == vide) {
#if defined(DEBUG)
      StdString("deposing piece on ");
      WriteSquare(*bnp);
      StdString(" ");
#endif
      deposebnp= bnp;
      ImmobilizeByBlBlock(blmoves,
                          whmoves, blpcallowed, whpcallowed, *bnp, false, n);
    }
  }

  deposebnp= isbnp;
#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "leaving DeposeBlPiece, deposebnp=%d\n", *deposebnp);
  StdString(GlobalStr);
#endif

} /* DeposeBlPiece */

static void PreventCheckAgainstWhK(
  int   blmoves,
  int   whmoves,
  int   blpc,
  int   whpc,
  stip_length_type n)
{
  square trouble= initsquare;
  boolean fbm= flagblackmummer;

  flagblackmummer= false;
  genmove(Black);
  flagblackmummer= fbm;

  while(encore() && (trouble == initsquare)) {
    if (move_generation_stack[nbcou].arrival == rb) {
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
    int dir= CheckDirQueen[rb-trouble];

    for (sq= trouble+dir; sq != rb; sq+=dir) {
      BlackPieceTo(sq, blmoves, whmoves, blpc, whpc, n);
      WhitePieceTo(sq, blmoves, whmoves, blpc, whpc, n);
    }
  }

  return;
}

static boolean Redundant(void)
{
  square const *bnp;
  square sq;
  piece p;
  Flags sp;
  boolean   flag;

  /* check for redundant white pieces */
  for (bnp= boardnum; *bnp; bnp++)
  {
    if (e[sq= *bnp] > obs)
    {
      if (sq == rb)
        continue;

      /* remove piece */
      p= e[sq]; sp= spec[sq];
      e[sq]= vide; spec[sq]= EmptySpec;

      flag= echecc(nbply,Black) && immobile(Black);

      /* restore piece */
      e[sq]= p; spec[sq]= sp;
      if (flag)
        return true;
    }
  }

  return false;
} /* Redundant */

static void StoreMate(
  int   blmoves,
  int   whmoves,
  int   blpc,
  int   whpc,
  stip_length_type n)
{
  int i, index;
  square const *bnp;
  square _rb, _rn;
  Flags sp;

  if (!immobile(Black)) {
    NeutralizeMateGuardingPieces(blmoves, whmoves, blpc, whpc, n);
    return;
  }

  if (Redundant())
    return;

  if (   (rb == initsquare)
         && (white[0].sq != initsquare)
         && (white[0].sq != square_e1)
         && (whmoves == 0))
  {
    if (e[white[0].sq] != vide) {
      return;
    }
  }

  if (echecc(nbply,White))
    PreventCheckAgainstWhK(blmoves, whmoves, blpc, whpc, n);

  MatesMax++;

#if defined(DETAILS)
  sprintf(GlobalStr, "mate no. %d\n", MatesMax);
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

  IndxChP= nbrchecking == -1
    ? -1
    : GetIndex(white[nbrchecking].sp);
  _rb= rb;
  _rn= rn;

  /* solve the problem */
  ResetPosition();
  castling_supported= is_cast_supp;

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

  sol_per_matingpos= 0;

  closehash();
  inithash();

  {
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    slice_root_solve_in_n(current_start_slice,n);
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

  for (i= King; i <= Bishop; i++) {
    nbpiece[-i]= nbpiece[i]= 2;
  }

  rb= _rb;
  rn= _rn;

  ep[1]= ep2[1]= initsquare;

  castling_supported= false;
} /* StoreMate */

static void PinBlPiece(
  square    topin,
  int   blmoves,
  int   whmoves,
  int   blpc,
  int   whpc,
  stip_length_type n)
{
  square    sq= topin;
  int   dir, time, i;
  boolean   diagonal;
  piece f_p;

  dir= sq-rn;
  diagonal= SquareCol(sq) == SquareCol(rn);
  while (e[sq+=dir] == vide) {
    for (i= 1; i < MaxPieceWhite; i++) {
      if (!whiteused[i] && (f_p= white[i].p) != Knight) {
        if (f_p == (diagonal ? Rook : Bishop)) {
          continue;
        }
        whiteused[i]= true;
        if (f_p == Pawn) {
          if (diagonal) {
            time= FroTo(f_p,
                        white[i].sq, Bishop, sq, false);
            if (time <= whmoves) {
              SetPiece(Bishop, sq, white[i].sp);
              StoreMate(blmoves,
                        whmoves-time, blpc, whpc,n);
            }
          }
          else {
            time=
              FroTo(f_p, white[i].sq, Rook, sq, false);
            if (time <= whmoves) {
              SetPiece(Rook, sq, white[i].sp);
              StoreMate(blmoves,
                        whmoves-time, blpc, whpc,n);
            }
          }
          time= FroTo(f_p, white[i].sq, Queen, sq, false);
          if (time <= whmoves) {
            SetPiece(Queen, sq, white[i].sp);
            StoreMate(blmoves,
                      whmoves-time, blpc, whpc,n);
          }
        }
        else {
          time= FroTo(f_p, white[i].sq, f_p, sq, false);
          if (time <= whmoves) {
            SetPiece(f_p, sq, white[i].sp);
            StoreMate(blmoves, whmoves-time, blpc, whpc, n);
          }
        }
        whiteused[i]= false;
      }
    }
    e[sq]= vide;
    spec[sq]= EmptySpec;
  }
}

static void ImmobilizeByPin(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  square    topin,
  stip_length_type n)
{
  int   dir, time, i;
  boolean   diagonal;
  square    sq;
  piece f_p;

  dir= CheckDirQueen[topin-rn];
  diagonal= SquareCol(rn+dir) == SquareCol(rn);

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
  sq= rn;
  while (e[(sq+=dir)]==vide)
    ;
  if (sq != topin) {
    return;
  }

  sq= topin;
  while (e[sq+=dir] == vide) {
    for (i= 1; i < MaxPieceWhite; i++) {
      if (!whiteused[i] && ((f_p= white[i].p) != Knight)) {
        if (f_p == (diagonal ? Rook : Bishop))
          continue;

        whiteused[i]= true;
        if (f_p == Pawn) {
          if (diagonal) {
            time=
              FroTo(f_p, white[i].sq, Bishop, sq, false);
            if (time <= whmoves) {
              SetPiece(Bishop, sq, white[i].sp);
              if (immobile(Black)) {
                StaleStoreMate(blmoves, whmoves-time,
                               blpcallowed-1, whpcallowed, n);
              }
              else {
                Immobilize(blmoves,
                           whmoves-time, blpcallowed-1,
                           whpcallowed, n);
              }
            }
          }
          else {
            time= FroTo(f_p, white[i].sq, Rook, sq, false);
            if (time <= whmoves) {
              SetPiece(Rook, sq, white[i].sp);
              if (immobile(Black)) {
                StaleStoreMate(blmoves, whmoves-time,
                               blpcallowed-1, whpcallowed, n);
              }
              else {
                Immobilize(blmoves, whmoves-time,
                           blpcallowed-1, whpcallowed, n);
              }
            }
          }
          time= FroTo(f_p, white[i].sq, Queen, sq, false);
          if (time <= whmoves) {
            SetPiece(Queen, sq, white[i].sp);
            if (immobile(Black)) {
              StaleStoreMate(blmoves,
                             whmoves-time, blpcallowed-1,
                             whpcallowed, n);
            }
            else {
              Immobilize(blmoves,
                         whmoves-time, blpcallowed-1,
                         whpcallowed, n);
            }
          }
        }
        else {
          time= FroTo(f_p, white[i].sq, f_p, sq, false);
          if (time <= whmoves) {
            SetPiece(f_p, sq, white[i].sp);
            if (immobile(Black)) {
              StaleStoreMate(
                blmoves, whmoves-time, blpcallowed-1,
                whpcallowed, n);
            }
            else {
              Immobilize(blmoves,
                         whmoves-time, blpcallowed-1,
                         whpcallowed, n);
            }
          }
        }
        whiteused[i]= false;
      }
    }
    e[sq]= vide;
    spec[sq]= EmptySpec;
  }
} /* ImmobilizeByPin */

static boolean BlIllegalCheck(square from, piece p) {
  int const dir = from-rb;
  switch(p)
  {
    case -Queen:
      return CheckDirQueen[dir] == dir;

    case -Knight:
      return CheckDirKnight[rb-from] != 0;

    case -Pawn:
      return (dir ==+dir_up+dir_right) || (dir==+dir_up+dir_left);

    case -Bishop:
      return CheckDirBishop[dir] == dir;

    case -Rook:
      return CheckDirRook[dir] == dir;

    default:
      return false;
  }
}

void DeposeWhKing(int   blmoves,
                  int   whmoves,
                  int   blpcallowed,
                  int   whpcallowed,
                  stip_length_type n)
{
  piece f_p;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering DeposeWhKing(%d,%d,%d,%d)\n",
          blmoves, whmoves, blpcallowed, whpcallowed);
  StdString(GlobalStr);
#endif

  rb= white[0].sq;
  if (e[rb] != vide) {
    rb= initsquare;
    return;
  }
  f_p= white[0].p;
  whiteused[0]= true;
  SetPiece(f_p, rb, white[0].sp);
  if (!IllegalCheck(Black) && !IllegalCheck(White)) {
    if (echecc(nbply,Black)) {
      AvoidCheckInStalemate(blmoves, whmoves,
                            blpcallowed, whpcallowed, n);
    }
    else {
      if (immobile(Black)) {
        StaleStoreMate(blmoves,
                       whmoves, blpcallowed, whpcallowed, n);
      }
      else {
        Immobilize(blmoves, whmoves, blpcallowed, whpcallowed, n);
      }
    }
  }
  e[rb]= vide;
  spec[rb]= EmptySpec;
  whiteused[0]= false;
  rb= initsquare;

#if defined(DEBUG)
  write_indentation();StdString("leaving DeposeWhKing\n");
#endif
}

void ImmobilizeByBlBlock(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  square    toblock,
  boolean   morethanonecheck,
  stip_length_type n)
{
  int i, time, pcreq;
  piece f_p;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering ImmobilizeByBlBlock(%d,%d,%d)\n",
          blmoves, whmoves, toblock);
  StdString(GlobalStr);
#endif

  for (i= 1; i < MaxPieceBlack; i++) {
    if (!blackused[i]) {
      f_p= black[i].p;
      blackused[i]= true;

      /* promotions */
      if (f_p == -Pawn) {
        /* A rough check whether it is worth thinking about
           promotions.
        */
        int moves= black[i].sq / onerow - 8;
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
          while (pp != vide) {
            time= FroTo(f_p,
                        black[i].sq, -pp, toblock, false);
            if ( time <= blmoves
                 && (rb == initsquare
                     || !BlIllegalCheck(toblock, pp)))
            {
              SetPiece(pp, toblock, black[i].sp);
              if (morethanonecheck) {
                AvoidCheckInStalemate(blmoves-time,
                                      whmoves, blpcallowed, whpcallowed-1,
                                      n);
              }
              else {
                if (immobile(Black)) {
                  StaleStoreMate(blmoves-time,
                                 whmoves,
                                 blpcallowed, whpcallowed-1, n);
                }
                else {
                  Immobilize(blmoves-time, whmoves,
                             blpcallowed, whpcallowed-1,
                             n);
                }
              }
            }
            pp= -getprompiece[-pp];
          }
        }
        pcreq= black[i].sq%onerow - toblock%onerow;
        if (pcreq < 0)
          pcreq= -pcreq;
      }
      else {
        pcreq= 0;
      }

      if (f_p!=-Pawn || toblock>=square_a2) {
        time= FroTo(f_p, black[i].sq, f_p, toblock, false);
        if ( time <= blmoves
             && pcreq <= blpcallowed
             && (rb == initsquare
                 || !BlIllegalCheck(toblock, f_p)))
        {
          SetPiece(f_p, toblock, black[i].sp);
          if (morethanonecheck) {
            AvoidCheckInStalemate(blmoves-time, whmoves,
                                  blpcallowed-pcreq, whpcallowed-1, n);
          }
          else {
            if (immobile(Black)) {
              StaleStoreMate(blmoves-time, whmoves,
                             blpcallowed-pcreq, whpcallowed-1, n);
            }
            else {
              Immobilize(blmoves-time, whmoves,
                         blpcallowed-pcreq, whpcallowed-1, n);
            }
          }
        }
      }
      blackused[i]= false;
    }
  }
  e[toblock]= vide;
  spec[toblock]= EmptySpec;

#if defined(DEBUG)
  write_indentation();StdString("leaving ImmobilizeByblBlock\n");
#endif
} /* ImmobilizeByBlBlock */

static void ImmobilizeByWhBlock(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  square    toblock,
  stip_length_type n)
{
  int    i, time, pcreq;
  piece f_p;

  if (blpcallowed < 0) {
    StdString("hu-hu!\n");
  }

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering ImmobilizeByWhBlock(%d,%d,%d)\n",
          blmoves, whmoves, toblock);
  StdString(GlobalStr);
#endif

  for (i= 0; i < MaxPieceWhite; i++) {
    /* take care of wKing in check/absence !!! */
    if (whiteused[i])
      continue;

    f_p= white[i].p;
    whiteused[i]= true;
    if (f_p == Pawn) {
      /* A rough check whether it is worth thinking about
         promotions.
      */
      if (whmoves
          >= (toblock<=square_h7 ? moves_to_prom[i]+1 : moves_to_prom[i]))
      {
        piece pp= getprompiece[vide];
        while (pp != vide) {
          time= FroTo(f_p, white[i].sq, pp, toblock, false);
          if (time <= whmoves) {
            SetPiece(pp, toblock, white[i].sp);
            if (!IllegalCheck(Black)) {
              if (echecc(nbply,Black)) {
                AvoidCheckInStalemate(blmoves,
                                      whmoves-time, blpcallowed-1,
                                      whpcallowed, n);
              }
              else {
                if (immobile(Black)) {
                  StaleStoreMate(blmoves,
                                 whmoves-time,
                                 blpcallowed-1, whpcallowed, n);
                }
                else {
                  Immobilize(blmoves, whmoves-time,
                             blpcallowed-1, whpcallowed, n);
                }
              }
            }
          }
          pp= getprompiece[pp];
        }
      }
      pcreq= white[i].sq%onerow - toblock%onerow;
      if (pcreq < 0) {
        pcreq= -pcreq;
      }
    }
    else {
      pcreq= 0;
    }

    time= FroTo(f_p, white[i].sq, f_p, toblock, false);
    if (time <= whmoves) {
      int decpc= i ? 1 : 0;
      SetPiece(f_p, toblock, white[i].sp);
      if (i == 0) {
        rb= toblock;
      }
      if (!IllegalCheck(Black)
          && (i > 0 || !IllegalCheck(White)))
      {
        if (echecc(nbply,Black)) {
          AvoidCheckInStalemate(blmoves,
                                whmoves-time, blpcallowed-decpc,
                                whpcallowed-pcreq, n);
        }
        else {
          if (immobile(Black)) {
            StaleStoreMate(blmoves, whmoves-time,
                           blpcallowed-decpc, whpcallowed-pcreq, n);
          }
          else {
            Immobilize(blmoves, whmoves-time,
                       blpcallowed-decpc, whpcallowed-pcreq, n);
          }
        }
      }
    }
    whiteused[i]= false;
    if (i == 0) {
      rb= initsquare;
    }
  }
  e[toblock]= vide;
  spec[toblock]= EmptySpec;

#if defined(DEBUG)
  write_indentation();StdString("leaving ImmobilizeByWhBlock\n");
#endif
} /* ImmobilizeByWhBlock */

void Immobilize(int blmoves,
                int whmoves,
                int blpcallowed,
                int whpcallowed,
                stip_length_type n)
{
  square    trouble, block, blblock;
  square const *bnp;
  int   i, blockwhite, blockblack, bpl, wpl, mtba, weight;
  boolean   nopinpossible, pinnecessary;
  unsigned int toblock[maxsquare+4];

  if (OptFlag[maxsols] && solutions>=maxsolutions)
    return;

  VARIABLE_INIT(block);

  trouble= initsquare;
  blockwhite= blockblack= bpl= wpl= 0;
  nopinpossible= true;
  pinnecessary= false;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering Immobilize(%d, %d,%d,%d)\n",
          blmoves, whmoves, blpcallowed, whpcallowed);
  StdString(GlobalStr);
#endif

  if (blpcallowed < 0 || whpcallowed<0)
    return;

  /* determine number of white pinning pieces available */
  for (i = 1; i<MaxPieceWhite && nopinpossible; i++)
    nopinpossible = (whiteused[i]
                     || white[i].p==Knight
                     || (white[i].p==Pawn && whmoves<moves_to_prom[i]));

  for (bnp = boardnum; *bnp; bnp++)
    toblock[*bnp] = 0;

  genmove(Black);
  while (encore() && trouble!=rn && !pinnecessary)
  {
    if (jouecoup(nbply,first_play) && !echecc(nbply,Black))
    {
      trouble = move_generation_stack[nbcou].departure;
      switch (-e[move_generation_stack[nbcou].arrival])
      {
        case Knight:
        case King:
          block= move_generation_stack[nbcou].arrival;
          break;
        default:
          block = (move_generation_stack[nbcou].departure
                   + CheckDirQueen[(move_generation_stack[nbcou].arrival
                                    -move_generation_stack[nbcou].departure)]);
      }
      pinnecessary= (move_generation_stack[nbcou].arrival==block
                     && pprise[nbply]!=vide);
      switch (toblock[block])
      {
        case 1:
          if (pjoue[nbply]!=-Pawn)
          {
            toblock[block]= 2;
            blockwhite--;
            blockblack++;
          }
          break;

        case 0:
          if (pjoue[nbply]==-Pawn)
          {
            toblock[block] = 1;
            blockwhite++;
          }
          else
          {
            toblock[block] = 2;
            blockblack++;
          }
          break;
      }
    }
    repcoup();
  }
  finply();

  if (trouble == rn)
    nopinpossible= true;

#if defined(DEBUG)
  if (trouble == initsquare) {
    StdString("something is wrong\n");
  }
#endif

  /* pin the trouble maker */
  if (!nopinpossible) {
    ImmobilizeByPin(blmoves,
                    whmoves, blpcallowed, whpcallowed, trouble, n);
  }

  /* block the trouble maker */
  if (!pinnecessary) {
    blblock= block;

    if (nopinpossible) {
      for (i= 1; i < MaxPieceBlack; i++) {
        if (!blackused[i]) {
          bpl++;
        }
      }

      if (bpl < blockblack) {
        return;
      }

      for (i= 0; i < MaxPieceWhite; i++) {
        if (!whiteused[i]) {
          wpl++;
        }
      }

      if (wpl + bpl < blockblack + blockwhite) {
        return;
      }

      mtba= 0;
      weight= 0;
      for (bnp= boardnum; *bnp && (mtba <= blmoves); bnp++) {
        if (toblock[*bnp]>1) {
          int nw= MovesToBlock(*bnp, blmoves);
          mtba += nw;
          if (nw > weight) {
            weight= nw;
            blblock= *bnp;
          }
        }
      }

      if (mtba > blmoves) {
        return;
      }
    }

    ImmobilizeByBlBlock(blmoves,
                        whmoves, blpcallowed, whpcallowed, blblock, false,
                        n);

    if ((blblock == block) && (-e[trouble] == Pawn)) {
      ImmobilizeByWhBlock(blmoves,
                          whmoves, blpcallowed, whpcallowed, block, n);
    }
  }

#if defined(DEBUG)
  write_indentation();StdString("leaving Immobilize\n");
#endif
} /* Immobilize */

void AvoidWhKingInCheck(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  stip_length_type n)
{
  int checkdirs[8], md= 0, i;

  if (blpcallowed < 0 || whpcallowed < 0) {
    return;
  }

  for (i= 8; i ; i--) {
    if (e[rb+vec[i]] == vide) {
      e[rb+vec[i]] = dummyb;
    }
  }

  for (i= 8; i ; i--) {
    if (e[rb+vec[i]] == dummyb) {
      e[rb+vec[i]] = vide;
      if (echecc(nbply,White)) {
        checkdirs[md++]= vec[i];
      }
      e[rb+vec[i]]= dummyb;
    }
  }

  for (i= 8; i ; i--) {
    if (e[rb+vec[i]] == dummyb) {
      e[rb+vec[i]] = vide;
    }
  }

#if defined(DEBUG)
  if (md == 0) {
    StdString("something's wrong\n");
    WritePosition();
  }
  sprintf(GlobalStr,"md=%d\n", md); StdString(GlobalStr);
#endif

  for (i= 0; i < md; i++) {
    square sq= rn;
    while (e[sq+=checkdirs[i]] == vide) {
      ImmobilizeByBlBlock(blmoves,
                          whmoves, blpcallowed, whpcallowed, sq, md-1, n);
      ImmobilizeByWhBlock(blmoves,
                          whmoves, blpcallowed, whpcallowed, sq, n);
    }
  }
} /* AvoidWhKingInCheck */


void AvoidCheckInStalemate(
  int   blmoves,
  int   whmoves,
  int   blpcallowed,
  int   whpcallowed,
  stip_length_type n)
{
  int checkdirs[8], md= 0, i;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering AvoidCheckInStaleMate(%d, %d)\n",
          blmoves, whmoves);
  StdString(GlobalStr);
#endif
  if (blpcallowed < 0 || whpcallowed < 0) {
    return;
  }

#if defined(DEBUG)
  if ( (*checkfunctions[Knight])(rn, cb, eval_ortho)
       || (*checkfunctions[Pawn])(rn, pb, eval_ortho)
       || (*checkfunctions[Fers])(rn, fb, eval_ortho)
       || (*checkfunctions[Wesir])(rn, tb, eval_ortho)
       || (*checkfunctions[ErlKing])(rn, db, eval_ortho))
  {
    printf("this message should not appear\n");
    return;
  }
#endif

  for (i= 8; i ; i--) {
    if (e[rn+vec[i]] == vide) {
      e[rn+vec[i]] = dummyb;
    }
  }

  for (i= 8; i ; i--) {
    if (e[rn+vec[i]] == dummyb) {
      e[rn+vec[i]] = vide;
      if (echecc(nbply,Black)) {
        checkdirs[md++]= vec[i];
      }
      e[rn+vec[i]]= dummyb;
    }
  }

  for (i= 8; i ; i--) {
    if (e[rn+vec[i]] == dummyb) {
      e[rn+vec[i]] = vide;
    }
  }

#if defined(DEBUG)
  if (md == 0) {
    StdString("something's wrong\n");
    WritePosition();
  }
  sprintf(GlobalStr,"md=%d\n", md); StdString(GlobalStr);
#endif

  for (i= 0; i < md; i++) {
    square sq= rn;
    while (e[sq+=checkdirs[i]] == vide) {
      ImmobilizeByBlBlock(blmoves,
                          whmoves, blpcallowed, whpcallowed, sq, md-1, n);
      ImmobilizeByWhBlock(blmoves,
                          whmoves, blpcallowed, whpcallowed, sq, n);
    }
  }
#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,"leaving AvoidCheckInStalemate\n");
  StdString(GlobalStr);
#endif
} /* AvoidCheckInStalemate */

void BlackPieceTo(
  square    sq,
  int   blmoves,
  int   whmoves,
  int   blpc,
  int   whpc,
  stip_length_type n)
{
  int time, actpbl;

  VARIABLE_INIT(time);

  for (actpbl= 1; actpbl < MaxPieceBlack; actpbl++) {
    if (!blackused[actpbl]) {
      piece p;

      p= black[actpbl].p;
      blackused[actpbl]= true;

      if (p != -Pawn || sq>=square_a2) {
        time= FroTo(p, black[actpbl].sq, p, sq, false);
        if ( (time <= blmoves)
             && ((rb == initsquare) || !BlIllegalCheck(sq, p)))
        {
          Flags sp= black[actpbl].sp;
          SetPiece(p, sq, sp);
          if (p == -Pawn) {
            int diffcol= black[actpbl].sq%onerow - sq%onerow;
            if (diffcol < 0) {
              diffcol= -diffcol;
            }
            if (diffcol <= blpc) {
              StoreMate(blmoves-time,
                        whmoves, blpc-diffcol, whpc, n);
            }
          }
          else {
            StoreMate(blmoves-time, whmoves, blpc, whpc, n);
          }
        }
      }

      /* pawn promotions */
      if (p == -Pawn) {
        /* A rough check whether it is worth thinking about
           promotions.
        */
        time= black[actpbl].sq / onerow - 8;
        if (time > 5) {
          time= 5;
        }
        if (sq>=square_a2)
          time++;

        if (time <= blmoves) {
          piece pp= -getprompiece[vide];
          while (pp != vide) {
            int diffcol;
            time= FroTo(p,
                        black[actpbl].sq, -pp, sq, false);
            /* black piece */
            if (pp == -Bishop
                && SquareCol(sq)
                != SquareCol(black[actpbl].sq%onerow+192))
            {
              diffcol= 1;
            }
            else {
              diffcol= 0;
            }
            if ( (diffcol <= blpc && time <= blmoves)
                 && (rb == initsquare
                     || !BlIllegalCheck(sq, pp)))
            {
              Flags sp= black[actpbl].sp;
              SetPiece(pp, sq, sp);
              StoreMate(blmoves-time,
                        whmoves, blpc-diffcol, whpc, n);
            }

            /* get next promotion piece */
            pp= -getprompiece[-pp];
          }
        }
      }
      blackused[actpbl]= false;
    }
  }
  e[sq]= vide;
  spec[sq]= EmptySpec;
} /* BlackPieceTo */

void WhitePieceTo(
  square    sq,
  int   blmoves,
  int   whmoves,
  int   blpc,
  int   whpc,
  stip_length_type n)
{
  int time, actpwh;

  VARIABLE_INIT(time);

  for (actpwh= 1; actpwh < MaxPieceWhite; actpwh++) {
    piece p;
    if (whiteused[actpwh]) {
      continue;
    }

    p= white[actpwh].p;
    whiteused[actpwh]= true;

    if (p != Pawn || sq < 360) {
      time= FroTo(p, white[actpwh].sq, p, sq, false);
      if (time <= whmoves) {
        Flags sp= white[actpwh].sp;
        SetPiece(p, sq, sp);
        if (IllegalCheck(Black)) {
          continue;
        }
        if (p == Pawn) {
          int diffcol= white[actpwh].sq%onerow - sq%onerow;
          if (diffcol < 0) {
            diffcol= -diffcol;
          }
          if (diffcol <= whpc) {
            StoreMate(blmoves,
                      whmoves-time, blpc, whpc-diffcol, n);
          }
        }
        else {
          StoreMate(blmoves, whmoves-time, blpc, whpc, n);
        }
      }
    }

    /* pawn promotions */
    if (p == Pawn) {
      /* A rough check whether it is worth thinking about
         promotions.
      */
      time= white[actpwh].sq / onerow - 8;
      if (time > 5) {
        time= 5;
      }
      if (sq < 360) {
        time++;
      }
      if (time <= whmoves) {
        piece pp= getprompiece[vide];
        while (pp != vide) {
          int diffcol;
          time= FroTo(p, white[actpwh].sq, pp, sq, false);
          if (pp == Bishop
              && SquareCol(sq)
              == SquareCol(white[actpwh].sq%onerow+192))
          {
            diffcol= 1;
          }
          else {
            diffcol= 0;
          }
          if (diffcol <= whpc && time <= whmoves) {
            Flags sp= white[actpwh].sp;
            SetPiece(pp, sq, sp);
            if (!IllegalCheck(Black)) {
              StoreMate(blmoves,
                        whmoves-time, blpc, whpc-diffcol, n);
            }
          }
          /* get next promotion piece */
          pp= getprompiece[pp];
        }
      }
    }
    whiteused[actpwh]= false;
  }
  e[sq]= vide;
  spec[sq]= EmptySpec;
} /* WhitePieceTo */

void NeutralizeMateGuardingPieces(
  int   blmoves,
  int   whmoves,
  int   blpc,
  int   whpc,
  stip_length_type n)
{
  square trouble, trto;

  VARIABLE_INIT(trto);
  trouble= initsquare;
  genmove(Black);

  while(encore() && (trouble == initsquare)) {
    if (jouecoup(nbply,first_play) && !echecc(nbply,Black)) {
      trouble= move_generation_stack[nbcou].departure;
      trto= move_generation_stack[nbcou].arrival;
    }
    repcoup();
  }

  finply();

  if (trouble == initsquare) {
    FtlMsg(ErrUndef);
  }

  PinBlPiece(trouble, blmoves, whmoves, blpc, whpc, n);
  if (is_rider(abs(e[trouble]))) {
    square   sq;
    int dir= CheckDirQueen[trto-trouble];

    for (sq= trouble+dir; sq != trto; sq+=dir) {
      BlackPieceTo(sq, blmoves, whmoves, blpc, whpc, n);
      WhitePieceTo(sq, blmoves, whmoves, blpc, whpc, n);
    }
  }
}

int MovesToBlock(square sq, int blmoves) {
  int i;
  int mintime= maxply+1;

  for (i= 1; i < MaxPieceBlack; i++) {
    int  time;
    piece     p= black[i].p;

    if (p!=-Pawn || sq>=square_a2) {
      time=  FroTo(p, black[i].sq, p, sq, false);
      if (time < mintime) {
        mintime= time;
      }
    }

    /* pawn promotions */
    if (p == -Pawn) {
      /* A rough check whether it is worth thinking about promotions. */
      int moves= black[i].sq / onerow - 8;
      if (moves > 5)
        moves= 5;

      if (sq>=square_a2)
        moves++;

      if (blmoves >= moves) {
        piece pp= -getprompiece[vide];
        while (pp != vide) {
          time= FroTo(p, black[i].sq, -pp, sq, false);
          if (time < mintime) {
            mintime= time;
          }
          pp= -getprompiece[-pp];
        }
      }
    }
  }

  return mintime;
} /* MovesToBlock */

static void GenerateBlocking(
  int   whmoves,
  int   nbrfl,
  square    *toblock,
  int   *mintime,
  int   blpcallowed,
  int   whpcallowed,
  int   timetowaste,
  stip_length_type n)
{
  int   actpbl, wasted;
  square    sq;

  if (OptFlag[maxsols] && solutions>=maxsolutions)
    return;

  if (nbrfl == 0) {
    /* check for stipulation */
    if (goal_to_be_reached == goal_stale || echecc(nbply,Black)) {
#if defined(DEBUG)
      if (IllegalCheck(White)) {
        StdString("oops!\n");
        exit(0);
      }
#endif
      if (goal_to_be_reached==goal_stale) {
        if (echecc(nbply,Black)) {
          AvoidCheckInStalemate(timetowaste,
                                whmoves, blpcallowed, whpcallowed, n);
        }
        else {
          if (immobile(Black)) {
            StaleStoreMate(timetowaste,
                           whmoves, blpcallowed, whpcallowed, n);
          }
          else {
            Immobilize(timetowaste,
                       whmoves, blpcallowed, whpcallowed, n);
          }
        }
      }
      else {
        StoreMate(timetowaste,
                  whmoves, blpcallowed, whpcallowed, n);
      }
    }
  }
  else {
    sq= toblock[--nbrfl];
    for (actpbl= 1; actpbl < MaxPieceBlack; actpbl++) {
      if (!blackused[actpbl]) {
        piece p= black[actpbl].p;

        blackused[actpbl]= true;

        if (p!=-Pawn || sq>=square_a2) {
          wasted= FroTo(p, black[actpbl].sq, p, sq, false)
            - mintime[nbrfl];
          if ((wasted <= timetowaste)
              && ((rb == initsquare) || !BlIllegalCheck(sq, p)))
          {
            Flags sp= black[actpbl].sp;
            SetPiece(p, sq, sp);
            if (p == -Pawn) {
              int diffcol;
              diffcol= black[actpbl].sq%onerow - sq%onerow;
              if (diffcol < 0) {
                diffcol= -diffcol;
              }
              if (diffcol <= blpcallowed) {
                GenerateBlocking(whmoves,
                                 nbrfl, toblock, mintime,
                                 blpcallowed-diffcol, whpcallowed,
                                 timetowaste-wasted, n);
              }
            }
            else {
              GenerateBlocking(whmoves,
                               nbrfl, toblock, mintime, blpcallowed,
                               whpcallowed, timetowaste-wasted, n);
            }
          }
        }
        /* pawn promotions */
        if (p == -Pawn) {
          /* A rough check whether it is worth thinking about
             promotions. */
          int moves= black[actpbl].sq / onerow - 8;
          if (moves > 5)
            moves= 5;

          if (sq>=square_a2)
            moves++;

          if (timetowaste >= moves-mintime[nbrfl]) {
            piece pp= -getprompiece[vide];
            while (pp != vide) {
              int diffcol;
              wasted= FroTo(p,
                            black[actpbl].sq, -pp, sq, false)
                - mintime[nbrfl];
              /* black piece */
              if (pp == -Bishop
                  && SquareCol(sq)
                  != SquareCol(black[actpbl].sq%onerow+192))
              {
                diffcol= 1;
              }
              else {
                diffcol= 0;
              }
              if ((diffcol <= blpcallowed
                   && wasted <= timetowaste)
                  && (rb == initsquare
                      || !BlIllegalCheck(sq, pp)))
              {
                Flags sp= black[actpbl].sp;
                SetPiece(pp, sq, sp);
                GenerateBlocking(whmoves,
                                 nbrfl, toblock, mintime,
                                 blpcallowed-diffcol, whpcallowed,
                                 timetowaste-wasted, n);
              }
              /* get next promotion piece */
              pp= -getprompiece[-pp];
            }
          }
        }
        blackused[actpbl]= false;
      }
    }
    e[sq]= vide;
    spec[sq]= EmptySpec;
  }
} /* GenerateBlocking */

static void GenerateGuarding(
  int   actpwh,
  int   whmoves,
  int   blmoves,
  int   whcaptures,
  stip_length_type n)
{
  square const *bnp;
  square toblock[8];
  int   flights;
  boolean   unblockable= false;

#if defined(DEBUG)
  sprintf(GlobalStr,
          "GenerateGuarding(%d, %d, %d, %d)\n",
          actpwh, whmoves, blmoves, whcaptures);
  StdString(GlobalStr);
#endif

  if (OptFlag[maxsols] && solutions>=maxsolutions)
    return;

  if (whcaptures > MaxPieceBlack-1
      || maxtime_status==MAXTIME_TIMEOUT) {
    return;
  }

  if (actpwh == nbrchecking) {
    actpwh++;
  }

  if (actpwh == MaxPieceWhite) {
    int i, blpcallowed, mtba, mintime[8];
    boolean set_king= false;
    flights= 0;

    /* check for check */
    if (goal_to_be_reached == goal_mate && !echecc(nbply,Black)) {
      return;
    }

    if (    (rb == initsquare)
            && (white[0].sq != initsquare)
            && (white[0].sq != square_e1)
            && (whmoves == 0))
    {
      if (    (e[white[0].sq] != vide)
              || (move_diff_code[abs(rn-white[0].sq)] < 3))
      {
        return;
      }
      set_king= true;
    }

    /* determine flights */
    genmove(Black);
    while(encore() && !unblockable) {
      if (jouecoup(nbply,first_play)
          && goal_to_be_reached==goal_stale)
      {
        e[move_generation_stack[nbcou].departure]= obs;
      }

      if (!echecc(nbply,Black)) {
        toblock[flights++]= move_generation_stack[nbcou].arrival;
        if (pprise[nbply] != vide) {
          unblockable= true;
        }
      }
      repcoup();
    }
    finply();

    if (unblockable || (flights > MaxPieceBlack-1)) {
      return;
    }

#if defined(DEBUG)
    WritePosition();
    sprintf(GlobalStr, "flights: %d, ", flights);
    StdString(GlobalStr);
    sprintf(GlobalStr, "whcaptures: %d\n", whcaptures);
    StdString(GlobalStr);
#endif
    if (whcaptures+flights > MaxPieceBlack-1) {
      return;
    }

    mtba= 0;
    for (i= 0; i < flights && mtba <= blmoves; i++) {
      mtba += mintime[i]= MovesToBlock(toblock[i], blmoves);
    }

    if (mtba > blmoves) {
      return;
    }

    /* count black pawn captures allowed */
    blpcallowed= 0;
    for (i= 1; i < MaxPieceWhite; i++) {
      if (!whiteused[i]) {
        blpcallowed++;
      }
    }

#if defined(DEBUG)
    sprintf(GlobalStr,
            "GenerateBlocking with timetowaste=%d\n", blmoves-mtba);
    StdString(GlobalStr);
#endif

    if (set_king) {
      rb= white[0].sq;
      SetPiece(white[0].p, rb, white[0].sp);
      whiteused[0]= true;
      GenerateBlocking(whmoves,
                       flights, toblock, mintime, blpcallowed,
                       MaxPieceBlack-1-whcaptures, blmoves-mtba, n);
      whiteused[0]= false;
      e[rb]= vide;
      spec[rb]= EmptySpec;
      rb= initsquare;
    }
    else {
      GenerateBlocking(whmoves, flights, toblock, mintime,
                       blpcallowed, MaxPieceBlack-1-whcaptures, blmoves-mtba,
                       n);
    }
  }
  else {
    piece   p= white[actpwh].p;
    Flags   sp= white[actpwh].sp;
    square  sq= white[actpwh].sq;

    whiteused[actpwh]= true;

    for (bnp= boardnum; *bnp; bnp++) {
      int time;
      if (e[*bnp] != vide)
        continue;

      /* set piece */
      time= FroTo(p, sq, p, *bnp, false);
      if (actpwh == 0) {
        if (move_diff_code[abs(rn-*bnp)] < 3) {
          continue;
        }
        rb= *bnp;
      }
      /* white piece */
      if (time <= whmoves && impact(rn, p, *bnp)) {
        if (guards(rn, p, *bnp)
            && actpwh < nbrchecking)
        {
          continue;
        }
        SetPiece(p, *bnp, sp);
        if (!IllegalCheck(Black)) {
          if (p == Pawn) {
            int diffcol= sq % onerow - *bnp % onerow;
            GenerateGuarding(actpwh+1, whmoves-time,
                             blmoves, whcaptures+abs(diffcol), n);
          }
          else {
            GenerateGuarding(actpwh+1, whmoves-time,
                             blmoves, whcaptures, n);
          }
        }
      }
      /* pawn promotions */
      if (p == Pawn) {
        /* A rough check whether it is worth thinking about
           promotions.
        */
        if (whmoves
            >= (*bnp<=square_h7
                ? moves_to_prom[actpwh]+1
                : moves_to_prom[actpwh]))
        {
          piece pp= getprompiece[vide];
          while (pp != vide) {
            time= FroTo(p, sq, pp, *bnp, false);
            if (impact(rn, pp, *bnp) && time <= whmoves) {
              if (!(guards(rn, pp, *bnp)
                    && actpwh < nbrchecking))
              {
                SetPiece(pp, *bnp, sp);
                if (!IllegalCheck(Black))
                  GenerateGuarding(actpwh+1,
                                   whmoves-time, blmoves,
                                   whcaptures,
                                   n);
              }
            }
            /* get next promotion piece */
            pp= getprompiece[pp];
          }
        }
      }
      e[*bnp]= vide;
      spec[*bnp]= EmptySpec;
    }

    /* captured piece */
    if (actpwh == 0) {
      rb= initsquare;
    }
    whiteused[actpwh]= false;
    GenerateGuarding(actpwh+1, whmoves, blmoves, whcaptures, n);
  }
} /* GenerateGuarding */

static void GenerateChecking(int whmoves, int blmoves,
                             stip_length_type n)
{
  int   i, j, time;
  square    sq;

#if defined(DEBUG)
  sprintf(GlobalStr, "GenerateChecking(%d, %d)\n", whmoves, blmoves);
  StdString(GlobalStr);
#endif

  for (j= 1; j < MaxPieceWhite; j++) {
    piece   p= white[j].p;
    Flags   sp= white[j].sp;

    whiteused[j]= true;
    nbrchecking= j;
    for (i= 0; i < nr_squares_on_board; i++) {
      sq= boardnum[i];
      if (e[sq] != vide) {
        continue;
      }
      /* set piece */
      time= FroTo(p, white[j].sq, p, sq, true);
      /* white piece */
      if (time <= whmoves && guards(rn, p, sq)) {
        SetPiece(p, sq, sp);
        piecechecking= p;
        squarechecking= sq;
        if (p == Pawn) {
          int diffcol= white[j].sq % onerow - sq % onerow;
          GenerateGuarding(0,
                           whmoves-time, blmoves, abs(diffcol), n);
        }
        else {
          GenerateGuarding(0, whmoves-time, blmoves, 0, n);
        }
      }
      /* pawn promotions */
      if (p == Pawn) {
        /* A rough check whether it is worth thinking about
           promotions.
        */
        if (whmoves
            >= (sq<=square_h7 ? moves_to_prom[j]+1 : moves_to_prom[j]))
        {
          piece pp= getprompiece[vide];
          while (pp != vide) {
            time= FroTo(p, white[j].sq, pp, sq, false);
            if (time <= whmoves && guards(rn, pp, sq)) {
              piecechecking= pp;
              squarechecking= sq;
              SetPiece(pp, sq, sp);
              GenerateGuarding(0,
                               whmoves-time, blmoves, 0, n);
            }
            /* get next promotion piece */
            pp= getprompiece[pp];
          }
        }
      }
      e[sq]= vide;
      spec[sq]= EmptySpec;
    }
    whiteused[j]= false;
  }
} /* GenerateChecking */

static void GenerateBlackKing(int whmoves, int blmoves, stip_length_type n)
{
  int   i, time;
  square    sq;
  piece p= black[0].p;
  Flags sp= black[0].sp;

#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,
          "entering GenerateBlackKing(%d, %d)\n",
          whmoves, blmoves);
  StdString(GlobalStr);
#endif

  for (i= 0; i < nr_squares_on_board; i++) {
    sq= boardnum[i];
    if (e[sq] == obs)
      continue;

    /* set piece */
    time= FroTo(p, black[0].sq, p, sq, false);
    if (time <= blmoves) {
      SetPiece(p, sq, sp);
      rn= sq;
#if defined(DEBUG)
      WriteSpec(spec[rn], false);
      WritePiece(p);
      WriteSquare(sq);
      StdString("\n");
#endif
      if (goal_to_be_reached==goal_mate) {
        GenerateChecking(whmoves, blmoves-time, n);
      }
      else {
        /* stale mate */
        nbrchecking= -1;
        GenerateGuarding(0, whmoves, blmoves-time, 0, n);
      }
      e[sq]= vide;
      spec[sq]= EmptySpec;
    }
    if (maxtime_status==MAXTIME_TIMEOUT) {
      break;
    }
  }
#if defined(DEBUG)
  write_indentation();
  sprintf(GlobalStr,"leaving GenerateBlackKing\n");
  StdString(GlobalStr);
#endif
} /* GenerateBlackKing */

static void InitSols(void)
{
  SolMax = 0;
}

static boolean CleanupSols(void)
{
  if (SolMax>0)
  {
    int i;
    for (i = 0; i<SolMax; i++)
      free(Sols[i]);

    free(Sols);

    return true;
  }
  else
    return false;
}

static void IntelligentRegularGoals(stip_length_type n)
{
  square const *bnp;
  piece p;

  deposebnp= boardnum;
  is_cast_supp= castling_supported;
  is_ep= ep[1]; is_ep2= ep2[1];
  castling_supported= false;

  SetIndex(spec[rn], 0);
  black[0].p= e[rn];
  black[0].sp= spec[rn];
  black[0].sq= rn;
  MaxPieceBlack= 1;

  SetIndex(spec[rb], 1);
  white[0].p= e[rb];
  white[0].sp= spec[rb];
  white[0].sq= rb;
  MaxPieceWhite= 1;
  if (rb == initsquare)
    whiteused[0]= true;

  MaxPieceAll= 2;

  for (bnp= boardnum; *bnp; bnp++)
    if ((rb != *bnp) && (e[*bnp] > obs)) {
      SetIndex(spec[*bnp], MaxPieceAll);
      white[MaxPieceWhite].p= e[*bnp];
      white[MaxPieceWhite].sp= spec[*bnp];
      white[MaxPieceWhite].sq= *bnp;
      whiteused[MaxPieceWhite]= false;
      if (e[*bnp] == Pawn) {
        int moves= 15 - *bnp / onerow;
        square  sq= *bnp;
        if (moves > 5)
          moves= 5;

        /* a white piece that cannot move away */
        if (moves == 5
            && moves == WhMovesLeft
            && (sq<=square_h2 && (e[sq+dir_up]>vide || e[sq+2*dir_up]>vide)))
          moves= maxply+1;

        /* a black pawn that needs a white sacrifice to move away */
        else if (WhMovesLeft < 7
                 && sq<=square_h2
                 && e[sq+dir_left] <= King && e[sq+dir_right] <= King
                 && (e[sq+dir_up] == -Pawn
                     || (e[sq+dir_up+dir_left] <= King
                         && e[sq+dir_up+dir_right] <= King
                         && (ep[1] != sq+dir_up+dir_left)
                         && (ep[1] != sq+dir_up+dir_right)
                         && e[sq+2*dir_up] == -Pawn)))
        {
          moves++;
        }
        moves_to_prom[MaxPieceWhite]= moves;
      }
      MaxPieceWhite++;
      MaxPieceAll++;
    }
  for (bnp= boardnum; *bnp; bnp++) {
    if ((rn != *bnp) && (e[*bnp] < vide)) {
      SetIndex(spec[*bnp], MaxPieceAll);
      black[MaxPieceBlack].p= e[*bnp];
      black[MaxPieceBlack].sp= spec[*bnp];
      black[MaxPieceBlack].sq= *bnp;
      blackused[MaxPieceBlack]= false;
      MaxPieceBlack++;
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

  for (p= King; p <= Bishop; p++) {
    nbpiece[-p]= nbpiece[p]= 2;
  }

  /* generate final positions */
  GenerateBlackKing(WhMovesLeft, BlMovesLeft, n);

  ResetPosition();

  if (OptFlag[movenbr]
      && maxtime_status!=MAXTIME_TIMEOUT)
  {
    sprintf(GlobalStr, "%ld %s %d+%d",
            MatesMax, GetMsgString(PotentialMates), WhMovesLeft, BlMovesLeft);
    StdString(GlobalStr);
    if (!flag_regression) {
      StdString("  (");
      PrintTime();
      StdString(")");
    }
    StdString("\n");
  }

  castling_supported= is_cast_supp;
  ep[1]= is_ep; ep2[1]= is_ep2;
}

static void IntelligentProof(stip_length_type n)
{
  boolean const save_movenbr = OptFlag[movenbr];

  ProofInitialiseIntelligent();

  /* Proof games and a=>b are special because there is only 1 end
   * position to be reached. We therefore output move numbers as if
   * we were not in intelligent mode, and only if we are solving
   * full-length.
   * If n is smaller, temporarily disable move number output:
   */
  if (n<slices[current_start_slice].u.branch.length)
    OptFlag[movenbr] = false;
    
  slice_root_solve_in_n(current_start_slice,n);

  OptFlag[movenbr] = save_movenbr;
}

/* Calculate the number of moves of each side, continuing at a non-root
 * slice.
 * @param si index of non-root slice
 */
static void init_moves_left_non_root(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);

  switch (slices[si].type)
  {
    case STBranchHelp:
    {
      stip_length_type const n = slices[si].u.branch.length;
      BlMovesLeft += (n-slack_length_help)/2;
      WhMovesLeft += (n-slack_length_help)/2;
      if (n%2==1)
      {
        if (branch_h_starter_in_n(si,n)==White)
          ++WhMovesLeft;
        else
          ++BlMovesLeft;
      }
      init_moves_left_non_root(slices[si].u.branch.next);
      break;
    }

    case STBranchSeries:
    {
      stip_length_type const n = slices[si].u.branch.length;
      if (slices[si].u.branch.starter==White)
        WhMovesLeft += n-slack_length_series;
      else
        BlMovesLeft += n-slack_length_series;
      init_moves_left_non_root(slices[si].u.branch.next);
      break;
    }

    case STLeafDirect:
      assert(goal_to_be_reached==no_goal);
      goal_to_be_reached = slices[si].u.leaf.goal;
      ++WhMovesLeft;
      break;

    case STLeafHelp:
      assert(goal_to_be_reached==no_goal);
      goal_to_be_reached = slices[si].u.leaf.goal;
      if (slices[si].u.leaf.starter==White)
        ++WhMovesLeft;
      else
        ++BlMovesLeft;
      break;

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      init_moves_left_non_root(next);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Calculate the number of moves of each side, starting at the root
 * slice.
 * @param n length of the solution(s) we are looking for
 */
static void init_moves_left_root(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",slices[si].type);

  goal_to_be_reached = no_goal;

  switch (slices[si].type)
  {
    case STBranchHelp:
    {
      BlMovesLeft = (n-slack_length_help)/2;
      WhMovesLeft = BlMovesLeft;
      if (n%2==1)
      {
        if (branch_h_starter_in_n(si,n)==White)
          ++WhMovesLeft;
        else
          ++BlMovesLeft;
      }

      init_moves_left_non_root(slices[si].u.branch.next);
      break;
    }

    case STBranchSeries:
    {
      if (slices[si].u.branch.starter==White)
      {
        WhMovesLeft = n-slack_length_series;
        BlMovesLeft = 0;
      }
      else
      {
        WhMovesLeft = 0;
        BlMovesLeft = n-slack_length_series;
      }

      init_moves_left_non_root(slices[si].u.branch.next);
      break;
    }

    case STMoveInverter:
      BlMovesLeft = 0;
      WhMovesLeft = 0;
      init_moves_left_root(slices[si].u.move_inverter.next,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceValue("%u",WhMovesLeft);
  TraceValue("%u\n",BlMovesLeft);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

boolean Intelligent(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",n);

  current_start_slice = si;

  init_moves_left_root(si,n);
     
  MatesMax = 0;

  InitSols();

  if (goal_to_be_reached==goal_atob
      || goal_to_be_reached==goal_proof)
    IntelligentProof(n);
  else
    IntelligentRegularGoals(n);

  result = CleanupSols();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

boolean isGoalReachable(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (goal_to_be_reached==goal_atob
      || goal_to_be_reached==goal_proof)
    result = !(*alternateImpossible)();
  else
    result = isGoalReachableRegularGoals();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

static support_for_intelligent_mode
stip_supports_intelligent_rec(slice_index si)
{
  support_for_intelligent_mode result = intelligent_not_supported;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchHelp:
      if (slices[si].u.branch.length<slack_length_help)
        result = intelligent_not_supported;
      else
        result = stip_supports_intelligent_rec(slices[si].u.branch.next);
      break;

    case STBranchSeries:
      if (slices[si].u.branch.length<slack_length_series)
        result = intelligent_not_supported;
      else
        result = stip_supports_intelligent_rec(slices[si].u.branch.next);
      break;

    case STLeafDirect:
    case STLeafHelp:
      switch (slices[si].u.leaf.goal)
      {
        case goal_proof:
        case goal_atob:
          result = intelligent_active_by_default;
          break;

        case goal_mate:
        case goal_stale:
          result = intelligent_not_active_by_default;
          break;

        default:
          result = intelligent_not_supported;
          break;
      }

      break;

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      result = stip_supports_intelligent_rec(next);
      break;
    }

    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.quodlibet.op1;
      slice_index const op2 = slices[si].u.quodlibet.op2;
      result = (stip_supports_intelligent_rec(op1)
                && stip_supports_intelligent_rec(op2));
      break;
    }

    default:
      /* nothing */
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

support_for_intelligent_mode stip_supports_intelligent(void)
{
  support_for_intelligent_mode result;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  result = stip_supports_intelligent_rec(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
