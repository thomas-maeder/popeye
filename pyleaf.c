#include "pyleaf.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pymsg.h"
#include "pyint.h"
#include "pyoutput.h"
#include "pyproof.h"

#include <assert.h>
#include <stdlib.h>

typedef Flags ColourSpec;

static boolean IsABattery(square KingSquare,
                          square FrontSquare,
                          numvec Direction,
                          ColourSpec ColourMovingPiece,
                          piece BackPiece1,
                          piece BackPiece2)
{
  square sq;
  piece p;

  /* is the line between king and front piece empty? */
  EndOfLine(FrontSquare, Direction, sq);
  if (sq == KingSquare) {
    /* the line is empty, but is there really an appropriate back
    ** battery piece? */
    EndOfLine(FrontSquare, -Direction, sq);
    p= e[sq];
    if (p < vide)
      p= -p;
    if ((p == BackPiece1 || p == BackPiece2)
        && TSTFLAG(spec[sq], ColourMovingPiece))
    {
      /* So, it is a battery. */
      return true;
    }
  }
  return false;
} /* IsABattery */

static void GenMatingPawn(square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  boolean Battery = false;
  numvec k;
  square sq_arrival;

  k= CheckDirBishop[sq_king-sq_departure];
  if (k!=0)
    Battery=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else {
    k= CheckDirRook[sq_king-sq_departure];
    if (k!=0)
      Battery=
        IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);
  }

  /* if the pawn is not the front piece of a battery reset k,
     otherwise normalise it to be positiv. This is necessary to
     avoid moves along the battery line subsequently.
  */
  if (Battery) {
    if (k<0)
      k= -k;
  }
  else
    k= 0;

  if (ColourMovingPiece==White) {
    if (sq_departure<=square_h1)
      return;
    else {
      /* not first rank */
      /* ep captures */
      if (ep[nbply-1]!=initsquare
          && trait[nbply-1]!=trait[nbply]
          && (sq_departure+dir_up+dir_right==ep[nbply-1]
              || sq_departure+dir_up+dir_left==ep[nbply-1]))
      {
        if (nbply==2)    /* ep.-key  standard pawn */
          move_generation_stack[repere[2]].arrival= ep[nbply-1]+dir_down;
        empile(sq_departure,
               ep[nbply-1],
               move_generation_stack[repere[nbply]].arrival);
      }

      /* single step */
      if (k!=24) {
        /* suppress moves along a battery line */
        sq_arrival= sq_departure+dir_up;
        if (e[sq_arrival]==vide) {
          if (Battery
              || sq_arrival+dir_up+dir_left == sq_king
              || sq_arrival+dir_up+dir_right == sq_king
              || (PromSq(White,sq_arrival)
                  && (CheckDirQueen[sq_king-sq_arrival]
                      || CheckDirKnight[sq_king-sq_arrival])))
            empile(sq_departure,sq_arrival,sq_arrival);

          /* double step */
          if (sq_departure<=square_h2) {
            sq_arrival+= onerow;
            if (e[sq_arrival]==vide
                && (Battery
                    || sq_arrival+dir_up+dir_left==sq_king
                    || sq_arrival+dir_up+dir_right==sq_king))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }

      /* capture+dir_up+dir_left */
      sq_arrival= sq_departure+dir_up+dir_left;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left == sq_king
            || sq_arrival+dir_up+dir_right == sq_king
            || (PromSq(White,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
      
      /* capture+dir_up+dir_right */
      sq_arrival= sq_departure+dir_up+dir_right;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left==sq_king
            || sq_arrival+dir_up+dir_right==sq_king
            || (PromSq(White,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    if (sq_departure>=square_a8)
      return;

    /* not last rank */
    /* ep captures */
    if (ep[nbply-1]!=initsquare
        && trait[nbply-1] != trait[nbply]
        && (sq_departure+dir_down+dir_left==ep[nbply-1]
            || sq_departure+dir_down+dir_right==ep[nbply-1]))
    {
      if (nbply==2)    /* ep.-key  standard pawn */
        move_generation_stack[repere[2]].arrival= ep[nbply-1]+dir_up;
      empile(sq_departure,
             ep[nbply-1],
             move_generation_stack[repere[nbply]].arrival);
    }

    /* single step */
    if (k!=24) {    /* suppress moves along a battery line */
      sq_arrival= sq_departure+dir_down;
      if (e[sq_arrival]==vide) {
        if (Battery
            || sq_arrival+dir_down+dir_right==sq_king
            || sq_arrival+dir_down+dir_left==sq_king
            || (PromSq(Black,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);

        /* double step */
        if (sq_departure>=square_a7) {
          sq_arrival-= onerow;
          if (e[sq_arrival] == vide
              && (Battery
                  || sq_arrival+dir_down+dir_right==sq_king
                  || sq_arrival+dir_down+dir_left==sq_king))
            empile(sq_departure,sq_arrival,sq_arrival);
        }
      }
    }

    /* capture+dir_up+dir_left */
    sq_arrival= sq_departure+dir_down+dir_right;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(Black,sq_arrival)
              && (CheckDirQueen[sq_king-sq_arrival]
                  || CheckDirKnight[sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }

    /* capture+dir_up+dir_right */
    sq_arrival= sq_departure+dir_down+dir_left;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(Black,sq_arrival)
              && (CheckDirQueen[sq_king-sq_arrival]
                  || CheckDirKnight[sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
} /* GenMatingPawn */

static void GenMatingKing(Goal goal,
                          square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  numvec    k, k2;
  boolean   Generate = false;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  if (rb==rn) {
    /* neutral kings */
    for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
      sq_arrival= sq_departure+vec[k2];
      /* they must capture to mate the opponent */
      if (e[sq_arrival]!=vide
          && TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    /* check if the king is the front piece of a battery
       that can fire */
    k= CheckDirBishop[sq_king-sq_departure];
    if (k!=0)
      Generate=
        IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
    else {
      k= CheckDirRook[sq_king-sq_departure];
      if (k!=0)
        Generate= IsABattery(sq_king,sq_departure,k,ColourMovingPiece,
                             Rook,Queen);
    }

    if (Generate)
      for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
        /* prevent the king from moving along the battery line*/
        if (k2==k || k2==-k)
          continue;
        sq_arrival= sq_departure+vec[k2];
        if ((e[sq_arrival]==vide
             || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
            && move_diff_code[abs(sq_king-sq_arrival)]>1+1) /* no contact */
          empile(sq_departure,sq_arrival,sq_arrival);
      }

    if (CondFlag[ColourCapturedPiece==White ? whiteedge : blackedge]
        || goal==goal_doublemate)
      for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
        sq_arrival= sq_departure + vec[k2];
        if ((e[sq_arrival]==vide
             || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
            && move_diff_code[abs(sq_king-sq_arrival)]<=1+1)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
  }

  /* castling */
  if (castling_supported) {
    if (ColourMovingPiece==White)
      /* white to play */
      genrb_cast();
    else
      /* black to play */
      genrn_cast();
  }
}

static void GenMatingKnight(square sq_departure,
                            square sq_king,
                            ColourSpec ColourMovingPiece)
{
  numvec    k;
  boolean   Generate = false;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  /* check if the knight is the front piece of a battery that can
     fire
  */
  if ((k = CheckDirBishop[sq_king-sq_departure])!=0)
    Generate=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else if ((k = CheckDirRook[sq_king-sq_departure])!=0)
    Generate= IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);

  k= abs(sq_king-sq_departure);
  if (Generate
      || (SquareCol(sq_departure) == SquareCol(sq_king)
          && move_diff_code[k]<=move_diff_code[square_a3-square_e1]
          && move_diff_code[k]!=move_diff_code[square_a3-square_c1]))
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_arrival= sq_departure+vec[k];
      if (e[sq_arrival]==vide
          || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        if (Generate || CheckDirKnight[sq_arrival-sq_king]!=0)
          empile(sq_departure,sq_arrival,sq_arrival);
    }
}

static void GenMatingRook(square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  square    sq2;
  numvec    k, k2;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  /* check if the rook is the front piece of a battery that can fire
   */
  k= CheckDirBishop[sq_king-sq_departure];
  if (k != 0
      && IsABattery(sq_king, sq_departure, k, ColourMovingPiece, Bishop, Queen))
  {
    for (k= vec_rook_start; k<=vec_rook_end; k++) {
      sq_arrival= sq_departure+vec[k];
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= vec[k];
      }
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    int OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];

    k2= CheckDirRook[sq_king-sq_departure];
    if (k2!=0) {
      /* the rook is already on a line with the king */
      EndOfLine(sq_departure,k2,sq_arrival);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
    else {
      for (k= vec_rook_start; k<=vec_rook_end; k++) {
        sq_arrival= sq_departure+vec[k];
        if (e[sq_arrival]==obs)
          continue;
        if (move_diff_code[abs(sq_arrival-sq_king)]<OriginalDistance) {
          /* The rook must move closer to the king! */
          k2= CheckDirRook[sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDirRook[sq_king-sq_arrival];
          }

          /* We are at the end of the line or in checking
             distance
          */
          if (k2==0)
            continue;
          if (e[sq_arrival]==vide
              || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
          {
            EndOfLine(sq_arrival,k2,sq2);
            if (sq2==sq_king)
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
}

static void GenMatingQueen(square sq_departure,
                           square sq_king,
                           ColourSpec ColourMovingPiece)
{
  square sq2;
  numvec  k, k2;
  ColourSpec ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  for (k= vec_queen_start; k<=vec_queen_end; k++) {
    sq_arrival= sq_departure+vec[k];
    while (e[sq_arrival]==vide) {
      k2= CheckDirQueen[sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
      sq_arrival+= vec[k];
    }
    if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
      k2= CheckDirQueen[sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void GenMatingBishop(square sq_departure,
                            square sq_king,
                            ColourSpec ColourMovingPiece)
{
  square    sq2;
  numvec    k, k2;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  /* check if the bishop is the front piece of a battery that can
     fire
  */
  k = CheckDirRook[sq_king-sq_departure];
  if (k!=0
      && IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen))
  {
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      sq_arrival= sq_departure+vec[k];
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= vec[k];
      }
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else if (SquareCol(sq_departure)==SquareCol(sq_king)) {
    int OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];

    k2= CheckDirBishop[sq_king-sq_departure];
    if (k2) {
      /* the bishop is already on a line with the king */
      EndOfLine(sq_departure,k2,sq_arrival);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
    else {
      for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
        sq_arrival= sq_departure+vec[k];
        if (e[sq_arrival]==obs)
          continue;
        if (move_diff_code[abs(sq_arrival-sq_king)]
            <OriginalDistance) {
          /* The bishop must move closer to the king! */
          k2= CheckDirBishop[sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDirBishop[sq_king-sq_arrival];
          }

          /* We are at the end of the line or in checking
             distance */
          if (k2==0)
            continue;
          if (e[sq_arrival]==vide
              || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
          {
            EndOfLine(sq_arrival,k2,sq2);
            if (sq2==sq_king)
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
} /* GenMatingBishop */


/* Generate moves for side side_at_move; optimise for moves reaching a
 * specific goal.
 * @param leaf leaf slice whose goal is to be reached by generated
 *             move(s)
 * @param side_at_move side for which to generate moves
 */
static void generate_move_reaching_goal(slice_index leaf, Side side_at_move)
{
  Goal const goal = slices[leaf].u.leaf.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",leaf);
  TraceFunctionParam("%d\n",side_at_move);

  if (side_at_move==White ? !flagwhitemummer : !flagblackmummer)
    empile_for_goal_of_leaf_slice = leaf;

  switch (goal)
  {
    case goal_mate:
    case goal_doublemate:
    case goal_check:
      TraceValue("%d\n",optim_orthomatingmoves);
      if (optim_orthomatingmoves)
      {
        square square_a = square_a1;
        square const OpponentsKing = side_at_move==White ? rn : rb;
        int i;

        nextply(nbply);
        trait[nbply]= side_at_move;
        init_move_generation_optimizer();

        /* Don't try to "optimize" by hand. The double-loop is tested as
         * the fastest way to compute (due to compiler-optimizations!) */
        for (i = nr_rows_on_board; i>0; i--, square_a += onerow)
        {
          square sq_departure = square_a;
          int j;
          for (j = nr_files_on_board; j>0; j--, sq_departure += dir_right)
          {
            piece const p = e[sq_departure];
            if (p!=vide && TSTFLAG(spec[sq_departure],side_at_move))
            {
              if (CondFlag[gridchess]
                  && !GridLegal(sq_departure,OpponentsKing))
              {
                if (side_at_move==White)
                  gen_wh_piece(sq_departure,p);
                else
                  gen_bl_piece(sq_departure,p);
              }
              else
              {
                switch(abs(p))
                {
                  case King:
                    GenMatingKing(goal,sq_departure,OpponentsKing,side_at_move);
                    break;

                  case Pawn:
                    GenMatingPawn(sq_departure,OpponentsKing,side_at_move);
                    break;

                  case Knight:
                    TraceText("Knight\n");
                    GenMatingKnight(sq_departure,OpponentsKing,side_at_move);
                    break;

                  case Rook:
                    GenMatingRook(sq_departure,OpponentsKing,side_at_move);
                    break;

                  case Queen:
                    GenMatingQueen(sq_departure,OpponentsKing,side_at_move);
                    break;

                  case Bishop:
                    GenMatingBishop(sq_departure,OpponentsKing,side_at_move);
                    break;
                }
              }
            }
          }
        }

        finish_move_generation_optimizer();
      }
      else
        genmove(side_at_move);
      break;

    case goal_ep:
      if (ep[nbply]==initsquare && ep2[nbply]==initsquare)
        nextply(nbply);
      else
        /* TODO only generate pawn moves? */
        genmove(side_at_move);
      break;

    case goal_castling:
      if (side_at_move==White
          ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)<=ke1_cancastle
          : TSTFLAGMASK(castling_flag[nbply],bl_castlings)<=ke8_cancastle)
        nextply(nbply);
      else
        /* TODO only generate king moves? */
        genmove(side_at_move);
      break;

    default:
      genmove(side_at_move);
      break;
  }

  empile_for_goal_of_leaf_slice = no_slice;

  TraceFunctionExit(__func__);
  TraceText("\n");
} /* generate_move_reaching_goal */


/* Determine whether the mating side still has a piece that could
 * deliver the mate.
 * @return true iff the mating side has such a piece
 */
static boolean is_a_mating_piece_left(Side mating_side)
{
  boolean const is_white_mating = mating_side==White;

  piece p = roib+1;
  while (p<derbla && nbpiece[is_white_mating ? p : -p]==0)
    p++;

  return p<derbla;
}

/* Determine whether a side has reached the goal of a leaf slice.
 * @param camp side
 * @param leaf slice index of leaf slice
 * @return true iff camp has reached leaf's goal
 */
boolean leaf_is_goal_reached(Side just_moved, slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",just_moved);
  TraceFunctionParam("%d\n",leaf);
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceValue("%d\n",slices[leaf].u.leaf.goal);
  switch (slices[leaf].u.leaf.goal)
  {
    case goal_mate:
      if (CondFlag[blackultraschachzwang]
          || CondFlag[whiteultraschachzwang])
        result = goal_checker_mate_ultraschachzwang(just_moved);
      else
        result = goal_checker_mate(just_moved);
      break;

    case goal_stale:
      result = goal_checker_stale(just_moved);
      break;

    case goal_dblstale:
      result = goal_checker_dblstale(just_moved);
      break;

    case goal_target:
      assert(slices[leaf].u.leaf.target!=initsquare);
      result = goal_checker_target(just_moved,slices[leaf].u.leaf.target);
      break;

    case goal_check:
      result = goal_checker_check(just_moved);
      break;

    case goal_capture:
      result = goal_checker_capture(just_moved);
      break;

    case goal_steingewinn:
      result = goal_checker_steingewinn(just_moved);
      break;

    case goal_ep:
      result = goal_checker_ep(just_moved);
      break;

    case goal_doublemate:
    case goal_countermate:
      result = goal_checker_doublemate(just_moved);
      break;

    case goal_castling:
      result = goal_checker_castling(just_moved);
      break;

    case goal_autostale:
      result = goal_checker_autostale(just_moved);
      break;

    case goal_circuit:
      result = goal_checker_circuit(just_moved);
      break;

    case goal_exchange:
      result = goal_checker_exchange(just_moved);
      break;

    case goal_circuitB:
      result = goal_checker_circuitB(just_moved);
      break;

    case goal_exchangeB:
      result = goal_checker_exchangeB(just_moved);
      break;

    case goal_any:
      result = goal_checker_any(just_moved);
      break;

    case goal_proof:
    case goal_atob:
      result = ProofIdentical();
      break;

    case goal_mate_or_stale:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Generate (piece by piece) candidate moves for the last move of a s#
 * or r#. Do *not* generate moves for the piece on square
 * initiallygenerated; this piece has already been taken care of.
 */
/* TODO Find out whether selflastencore() is really more efficient
 * than the usual genmove() */
static boolean selflastencore(square const **selfbnp,
                              square initiallygenerated,
                              slice_index leaf)
{
  if (encore())
    return true;
  else
  {
    Side const attacker = slices[leaf].starter;
    Side const defender = advers(attacker);
    square curr_square = **selfbnp;

    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(attacker);

    while (curr_square!=initsquare)
    {
      if (curr_square!=initiallygenerated)
      {
        piece p= e[curr_square];
        if (p!=vide)
        {
          if (TSTFLAG(spec[curr_square],Neutral))
            p = -p;
          if (defender==White)
          {
            if (p>obs)
              gen_wh_piece(curr_square,p);
          }
          else
          {
            if (p<vide)
              gen_bl_piece(curr_square,p);
          }
        }
      }
    
      ++*selfbnp;
      curr_square = **selfbnp;

      if (encore())
        return true;
    }

    return false;
  }
} /* selflastencore */

/* Determine whether the side at move must end in 1.
 * @return true iff side_at_move can end in 1 move
 */
static boolean leaf_is_end_in_1_forced(slice_index leaf)
{
  Side const defender = advers(slices[leaf].starter);
  boolean is_defender_immobile = true;
  boolean escape_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",leaf);
  TraceFunctionParam("%d\n",slices[leaf].u.leaf.goal);

  if (defender==Black ? flagblackmummer : flagwhitemummer)
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    genmove(defender);
    move_generation_mode = move_generation_optimized_by_killer_move;

    while (!escape_found
           && encore())
    {
      if (jouecoup(nbply)
          && !echecc(nbply,defender))
      {
        is_defender_immobile = false;
        /* TODO this checks for echecc(nbply,defender) again (in most cases
         * anyway); optimise? */
        escape_found = !leaf_is_goal_reached(defender,leaf);
        if (escape_found)
          coupfort();
      }

      repcoup();
    }
    finply();
  }
  else if (slices[leaf].u.leaf.goal==goal_ep
           && ep[nbply]==initsquare
           && ep2[nbply]==initsquare)
  {
    /* a little optimization if end "state" is en passant capture,
     * but no en passant capture is possible */
    /* TODO Should we play the same trick for castling? Other end
     * states? */
  }
  else
  {
    piece p;
    square const *selfbnp = boardnum;
    square initiallygenerated = initsquare;
    Side const attacker = advers(defender);

    nextply(nbply);
    init_move_generation_optimizer();
    trait[nbply]= defender;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(attacker);

    p = e[current_killer_state.move.departure];
    if (p!=vide)
    {
      if (TSTFLAG(spec[current_killer_state.move.departure], Neutral))
        p = -p;
      if (defender==White)
      {
        if (p>obs)
        {
          initiallygenerated = current_killer_state.move.departure;
          gen_wh_piece(initiallygenerated,p);
        }
      }
      else
      {
        if (p<-obs)
        {
          initiallygenerated = current_killer_state.move.departure;
          gen_bl_piece(initiallygenerated,p);
        }
      }
    }
    finish_move_generation_optimizer();
    while (!escape_found
           && selflastencore(&selfbnp,initiallygenerated,leaf))
    {
      if (jouecoup(nbply) && TraceCurrentMove()
          && !echecc(nbply,defender))
      {
        is_defender_immobile = false;
        /* TODO this checks for echecc(nbply,defender) again (in most cases
         * anyway); optimise? */
        if (!leaf_is_goal_reached(defender,leaf))
        {
          TraceText("escape_found\n");
          escape_found = true;
          coupfort();
        }
      }
      repcoup();
    }
    finply();
  }

  TraceFunctionExit(__func__);
  TraceValue("%d",escape_found);
  TraceValue("%d",is_defender_immobile);
  TraceFunctionResult("%d\n", !(escape_found || is_defender_immobile));
  return !(escape_found || is_defender_immobile);
}

/* Determine whether a side has an end in 1.
 * @param side_at_move
 * @param leaf slice index
 * @return true iff side_at_move can end in 1 move
 */
boolean leaf_is_end_in_1_possible(Side side_at_move, slice_index leaf)
{
  boolean end_found = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u ",side_at_move);
  TraceFunctionParam("%u\n",leaf);

  if (OptFlag[keepmating] && !is_a_mating_piece_left(side_at_move))
    return false;

  if (slices[leaf].u.leaf.goal==goal_mate)
    generate_move_reaching_goal(leaf,side_at_move);
  else
    genmove(side_at_move);

  while (encore()
         && !end_found)
  {
    if (jouecoup(nbply) && TraceCurrentMove()
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      TraceText("goal reached\n");
      end_found = true;
      coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",end_found);
  return end_found;
}

/* Detect a priori unsolvability of a leaf (e.g. because of a forced
 * reflex mate)
 * @param leaf leaf's slice index
 * @return true iff leaf is a priory unsolvable
 */
boolean leaf_is_unsolvable(slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
    {
      Side const attacker = slices[leaf].starter;
      result = OptFlag[keepmating] && !is_a_mating_piece_left(attacker);
      break;
    }

    case EReflex:
    {
      Side const attacker = slices[leaf].starter;
      Side const defender = advers(attacker);
      result = (leaf_is_end_in_1_possible(attacker,leaf)
                || (OptFlag[keepmating]
                    && !is_a_mating_piece_left(defender)));
      break;
    }

    case ESelf:
    case ESemireflex:
    {
      Side const defender = advers(slices[leaf].starter);
      result = OptFlag[keepmating] && !is_a_mating_piece_left(defender);
      break;
    }

    case EHelp:
    {
      Side const final = advers(slices[leaf].starter);
      result = OptFlag[keepmating] && !is_a_mating_piece_left(final);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean leaf_h_cmate_is_solvable(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;
  Side const other_side = advers(side_at_move);

  generate_move_reaching_goal(leaf,side_at_move);

  while (encore() && !found_solution)
  {
    if (jouecoup(nbply)
        && !echecc(nbply,side_at_move))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
      {
        if (goal_checker_mate(side_at_move))
        {
          generate_move_reaching_goal(leaf,other_side);

          while (encore() && !found_solution)
          {
            if (jouecoup(nbply))
              found_solution = leaf_is_goal_reached(other_side,leaf);

            repcoup();
          }

          finply();
        }

        if (!found_solution)
          addtohash(leaf,HelpNoSuccOdd,1,&hb);
      }
    }

    repcoup();
  }

  finply();

  return found_solution;
}

static boolean leaf_h_dmate_is_solvable(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;
  Side const other_side = advers(side_at_move);

  genmove(side_at_move);

  while (encore() && !found_solution)
  {
    if (jouecoup(nbply)
        && !echecc(nbply,side_at_move))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
      {
        if (!immobile(other_side))
        {
          generate_move_reaching_goal(leaf,other_side);

          while (encore() && !found_solution)
          {
            if (jouecoup(nbply))
              found_solution = leaf_is_goal_reached(other_side,leaf);

            repcoup();
          }

          finply();
        }

        if (!found_solution)
          addtohash(leaf,HelpNoSuccOdd,1,&hb);
      }
    }

    repcoup();
  }

  finply();

  return found_solution;
}

static boolean leaf_h_exists_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = advers(slices[leaf].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  TraceValue("%d\n",side_at_move);

  generate_move_reaching_goal(leaf,side_at_move);

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore() && !final_move_found)
  {
    if (jouecoup(nbply) && TraceCurrentMove())
    {
      if (isIntelligentModeActive && !isGoalReachable())
        TraceText("isIntelligentModeActive && !isGoalReachable()\n");
      else if (!leaf_is_goal_reached(side_at_move,leaf))
        TraceText("!leaf_is_goal_reached(side_at_move,leaf)\n");
      else
        final_move_found = true;
    }

    repcoup();
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",final_move_found);
  return final_move_found;
}

static boolean leaf_h_regulargoals_is_solvable(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(side_at_move);

  if (side_at_move==Black)
    BlMovesLeft--;
  else
    WhMovesLeft--;

  while (encore() && !found_solution)
  {
    if (jouecoup(nbply) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && !leaf_is_unsolvable(leaf))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
      {
        if (leaf_h_exists_final_move(leaf))
          found_solution = true;
        else
          addtohash(leaf,HelpNoSuccOdd,1,&hb);
      }
    }

    repcoup();
  }
    
  if (side_at_move==Black)
    BlMovesLeft++;
  else
    WhMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Determine whether a leaf has >=1 help play solution.
 * @param leaf slice index
 * @return true iff the leaf has >=1 solution 
 */
static boolean leaf_h_is_solvable(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_h_cmate_is_solvable(leaf);
      break;

    case goal_doublemate:
      result = leaf_h_dmate_is_solvable(leaf);
      break;

    default:
      result = leaf_h_regulargoals_is_solvable(leaf);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether there is >= 1 solution for the leaf
 * @param leaf slice index of leaf slice
 * @return true iff side_at_move has >=1 solution
 */
boolean leaf_is_solvable(slice_index leaf)
{
  boolean result = false;
  
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);
  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
    case ESelf:
    case EReflex:
    case ESemireflex:
      if (leaf_is_unsolvable(leaf))
        ; /* intentionally nothing */
      else if (d_leaf_has_defender_lost(leaf))
        result = true;
      else
        result = d_leaf_does_attacker_win(leaf);
      break;

    case EHelp:
      result = leaf_h_is_solvable(leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has directly won with his move just
 * played. 
 * Assumes that there is no short win for the defending side.
 * @param leaf slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_leaf_has_defender_won(slice_index leaf)
{
  return leaf_is_unsolvable(leaf);
}

/* Determine whether the defender has lost with his move just played.
 * Assumes that there is no short win for the defending side.
 * @param si slice identifier
 * @return whether there is a short win or loss
 */
boolean d_leaf_has_defender_lost(slice_index leaf)
{
  Side const defender = advers(slices[leaf].starter);
  boolean result = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",defender);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      result = leaf_is_goal_reached(defender,leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether a side has just solved a leaf slice in direct play.
 * @param attacker attacking side of predecessor slice, has just moved
 * @param leaf slice index
 * @return true iff attacker has just solved leaf
 */
boolean d_leaf_is_solved(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;

  assert(slices[leaf].type==STLeaf);
  assert(attacker!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return leaf_is_goal_reached(attacker,leaf);

    case ESelf:
      return leaf_is_end_in_1_forced(leaf);

    case EReflex:
    case ESemireflex:
    {
      Side const defender = advers(attacker);
      return leaf_is_end_in_1_possible(defender,leaf);
    }

    default:
      assert(0);
      return false;
  }
}

/* Determine and write forced end moves in 1 by the attacker in reflex
 * stipulations; we know that at least 1 exists.
 * @param leaf leaf's slice index
 */
static void d_leaf_r_solve_forced_keys(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  Goal const goal = slices[leaf].u.leaf.goal;

  generate_move_reaching_goal(leaf,attacker);
  active_slice[nbply] = leaf;

  while(encore())
  {
    if (jouecoup(nbply)
        && leaf_is_goal_reached(attacker,leaf))
    {
      write_attack(goal,attack_regular);
      write_attack_conclusion(attack_without_zugzwang);
    }

    repcoup();
  }

  finply();
}

/* Write a priori unsolvability (if any) of a leaf in direct play
 * (e.g. forced reflex mates)
 * @param leaf leaf's slice index
 */
void d_leaf_write_unsolvability(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EReflex:
      d_leaf_r_solve_forced_keys(leaf);
      break;

    default:
      break;
  }
}

/* Determine and write keys if the end is direct
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  generate_move_reaching_goal(leaf,attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply) && TraceCurrentMove()
        && leaf_is_goal_reached(attacker,leaf))
    {
      solution_found = true;
      write_attack(slices[leaf].u.leaf.goal,attack_key);
      write_attack_conclusion(attack_without_zugzwang);
      write_end_of_solution();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Determine and write all final moves of a self/reflex variation.
 * @param leaf slice index of the leaf slice
 * @return true iff >=1 solution was found
 */
static boolean leaf_sr_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const defender = advers(slices[leaf].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  generate_move_reaching_goal(leaf,defender);
  active_slice[nbply] = leaf;

  while(encore())
  {
    if (jouecoup(nbply)
        && leaf_is_goal_reached(defender,leaf))
    {
      found_solution = true;
      write_defense(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Determine whether the defender is not forced to end in 1 in a
 * self stipulation.
 * @param leaf slice identifier
 * @return true iff defender wins
 */
static boolean d_leaf_s_does_defender_win(slice_index leaf)
{
  Side const defender = advers(slices[leaf].starter);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  result = ((OptFlag[keepmating] && !is_a_mating_piece_left(defender))
            || !leaf_is_end_in_1_forced(leaf));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender is not forced to end in 1 in a
 * reflex stipulation.
 * @param leaf slice identifier
 * @return true iff defender wins
 */
static boolean d_leaf_r_does_defender_win(slice_index leaf)
{
  boolean result = true;
  Side const defender = advers(slices[leaf].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  if (!(OptFlag[keepmating] && !is_a_mating_piece_left(defender)))
    result = !leaf_is_end_in_1_possible(defender,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defending side wins
 * @param leaf slice identifier
 * @return true iff defender wins
 */
static boolean d_leaf_does_defender_win(slice_index leaf)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
    {
      result = !leaf_is_goal_reached(slices[leaf].starter,leaf);
      break;
    }

    case ESelf:
      result = d_leaf_s_does_defender_win(leaf);
      break;

    case EReflex:
    case ESemireflex:
      result = d_leaf_r_does_defender_win(leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write solutions in a self stipulation in 1 move
 * @param leaf slice index of the leaf slice
 * @return true iff >=1 key was found and written
 */
static boolean leaf_s_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  boolean found_solution = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply)
        && !echecc(nbply,attacker)
        && !d_leaf_s_does_defender_win(leaf))
    {
      found_solution = true;

      write_attack(no_goal,attack_key);
      write_attack_conclusion(attack_without_zugzwang);

      marge += 4;
      leaf_sr_solve_final_move(leaf);
      marge -= 4;
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Determine and write the final move pair in a semireflex
 * leaf.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_semir_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && !d_leaf_r_does_defender_win(leaf))
    {
      found_solution = true;

      write_attack(no_goal,attack_key);
      write_attack_conclusion(attack_without_zugzwang);

      marge += 4;
      leaf_sr_solve_final_move(leaf);
      marge -= 4;
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Determine and write the final move pair in a reflex leaf.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_r_solve(slice_index leaf)
{
  boolean result = false;
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  if (!leaf_is_end_in_1_possible(attacker,leaf))
    result = leaf_semir_solve(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write the final move pair in help countermate.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_cmate_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;
  Side const other_side = advers(side_at_move);

  generate_move_reaching_goal(leaf,side_at_move);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply)
        && !echecc(nbply,side_at_move))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
      {
        if (goal_checker_mate(side_at_move))
        {
          generate_move_reaching_goal(leaf,other_side);
          active_slice[nbply] = leaf;

          while (encore())
          {
            if (jouecoup(nbply)
                && leaf_is_goal_reached(other_side,leaf))
            {
              found_solution = true;
              write_defense(goal_countermate);
            }
            repcoup();
          }

          finply();
        }

        if (!found_solution)
          addtohash(leaf,HelpNoSuccOdd,1,&hb);
      }
    }

    repcoup();
  }

  finply();

  return found_solution;
}

/* Determine and write the final move pair in help doublemate.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_dmate_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;
  Side const other_side = advers(side_at_move);

  genmove(side_at_move);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply)
        && !echecc(nbply,side_at_move))
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
      {
        if (!immobile(other_side))
        {
          generate_move_reaching_goal(leaf,other_side);
          active_slice[nbply] = leaf;

          while (encore())
          {
            if (jouecoup(nbply)
                && leaf_is_goal_reached(other_side,leaf))
            {
              found_solution = true;
              write_defense(goal_doublemate);
            }

            repcoup();
          }

          finply();
        }

        if (!found_solution)
          addtohash(leaf,HelpNoSuccOdd,1,&hb);
      }
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  return found_solution;
}

#if !defined(DATABASE)
/* Determine and find final moves of a help leaf
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean leaf_h_solve_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = advers(slices[leaf].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  TraceValue("%d\n",side_at_move);

  generate_move_reaching_goal(leaf,side_at_move);
  active_slice[nbply] = leaf;

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore())
  {
    if (jouecoup(nbply) && TraceCurrentMove())
    {
      if (isIntelligentModeActive && !isGoalReachable())
        TraceText("isIntelligentModeActive && !isGoalReachable()\n");
      else if (!leaf_is_goal_reached(side_at_move,leaf))
        TraceText("!leaf_is_goal_reached(side_at_move,leaf)\n");
      else
      {
        final_move_found = true;
        write_defense(leaf);
      }
    }

    repcoup();
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",final_move_found);
  return final_move_found;
}
#endif

/* Determine and write the final move pair in help stipulation with
 * "regular" goal. 
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_regulargoals_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(side_at_move);
  active_slice[nbply] = leaf;

  if (side_at_move==Black)
    BlMovesLeft--;
  else
    WhMovesLeft--;

  while (encore())
  {
    if (jouecoup(nbply) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && !leaf_is_unsolvable(leaf))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_h_solve_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_h_solve_final_move(leaf))
        found_solution = true;
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }
    
  if (side_at_move==Black)
    BlMovesLeft++;
  else
    WhMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Determine and write the solution of a help leaf slice.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_h_cmate_solve(leaf);
      break;

    case goal_doublemate:
      result = leaf_h_dmate_solve(leaf);
      break;

    default:
      result = leaf_h_regulargoals_solve(leaf);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* In "directoid" leaf slices, solve the final defender move
 * @param leaf slice index
 */
static void d_leaf_solve_final_defender_move(slice_index leaf)
{
  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      /* nothing */
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      leaf_sr_solve_final_move(leaf);
      break;
    
    default:
      assert(0);
      break;
  }
}

/* Write the key and solve the remainder of a leaf in direct play
 * @param refutations table containing the refutations
 * @param leaf slice index
 * @param type type of attack
 */
void d_leaf_write_key_solve_postkey(int refutations,
                                    slice_index leaf,
                                    attack_type type)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      write_attack(slices[leaf].u.leaf.goal,type);
      write_attack_conclusion(attack_without_zugzwang);
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      write_attack(no_goal,type);
      write_attack_conclusion(attack_without_zugzwang);
      marge += 4;

      if (OptFlag[solvariantes])
      {
        marge += 4;
        leaf_sr_solve_final_move(leaf);
        marge -= 4;
      }

      marge -= 4;
      break;

    default:
      assert(0);
      break;
  }

  marge += 4;
  write_refutations(refutations);
  marge -= 4;
}

/* Determine whether the attacking side has directly lost by the move
 * that it has just played.
 * @param defender defending side
 * @param leaf slice index
 * @return true iff the attacking side has directly lost
 */
boolean d_leaf_has_attacker_lost(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return false;

    case ESelf:
    case EReflex:
    case ESemireflex:
      return (OptFlag[keepmating]
              && !is_a_mating_piece_left(slices[leaf].starter));

    default:
      assert(0);
      return false;
  }
}

/* Determine whether the attacking side has directly won by the move
 * that it has just played.
 * @param defender defending side
 * @param leaf slice index
 * @return true iff the attacking side has directly won
 */
boolean d_leaf_has_attacker_won(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return leaf_is_goal_reached(slices[leaf].starter,leaf);

    case ESelf:
      return false;

    case EReflex:
    case ESemireflex:
      return leaf_is_end_in_1_possible(advers(slices[leaf].starter),leaf);

    default:
      assert(0);
      return false;
  }
}

/* Determine whether attacker can end in 1 move in direct play.
 * @param attacker attacking side (i.e. side attempting to reach the
 * end)
 * @param leaf slice index of leaf slice
 * @return true iff attacker can end in 1 move
 */
static boolean d_leaf_d_does_attacker_win(slice_index leaf)
{
  boolean end_found = false;
  HashBuffer hb;
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  /* In move orientated stipulations (%, z, x etc.) it's less
   * expensive to compute an end in 1. TLi
   */
  if (!FlagMoveOrientatedStip)
  {
    /* It is more likely that a position has no solution. 
     * Therefore let's check for "no solution" first.  TLi
     */
    (*encode)(&hb);
    if (inhash(leaf,DirNoSucc,1,&hb))
    {
      TraceText("DirNoSucc\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%d\n",false);
      return false;
    }
    if (inhash(leaf,DirSucc,0,&hb))
    {
      TraceText("DirSucc\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%d\n",true);
      return true;
    }
  }

  if (OptFlag[keepmating] && !is_a_mating_piece_left(attacker))
  {
    TraceText("!is_a_mating_piece_left\n");
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",false);
    return false;
  }

  if (slices[leaf].u.leaf.goal==goal_doublemate
      && immobile(attacker))
  {
    TraceText("attacker is immobile\n");
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",false);
    return false;
  }

  generate_move_reaching_goal(leaf,attacker);

  while (encore() && !end_found)
  {
    if (jouecoup(nbply) && TraceCurrentMove())
    {
      end_found = leaf_is_goal_reached(attacker,leaf);
      if (end_found)
      {
        TraceText("wins\n");
        coupfort();
      }
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  if (!FlagMoveOrientatedStip)
  {
    if (end_found)
      addtohash(leaf,DirSucc,0,&hb);
    else
      addtohash(leaf,DirNoSucc,1,&hb);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",end_found);
  return end_found;
}

/* Determine whether the attacker wins in a self/reflex leaf slice
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
static boolean d_leaf_sr_does_attacker_win(slice_index leaf)
{
  boolean win_found = false;
  HashBuffer hb;
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  /* It is more likely that a position has no solution. */
  /*    Therefore let's check for "no solution" first. TLi */
  (*encode)(&hb);
  if (inhash(leaf,DirNoSucc,1,&hb))
  {
    assert(!inhash(leaf,DirSucc,0,&hb));
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",false);
    return false;
  }
  if (inhash(leaf,DirSucc,0,&hb))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",true);
    return true;
  }

  genmove(attacker);

  while (!win_found
         && encore())
  {
    if (jouecoup(nbply) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && !d_leaf_does_defender_win(leaf))
    {
      TraceText("wins\n");
      win_found = true;
      coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  if (win_found)
    addtohash(leaf,DirSucc,0,&hb);
  else
    addtohash(leaf,DirNoSucc,1,&hb);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",win_found);
  return win_found;
}

/* Determine whether the attacker wins in a direct/self/reflex
 * stipulation
 * @param attacker attacking side (at move)
 * @param leaf slice index
 * @return true iff attacker wins
 */
boolean d_leaf_does_attacker_win(slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      result = d_leaf_d_does_attacker_win(leaf);
      break;

    case ESelf:
    case ESemireflex:
    case EReflex:
      result = d_leaf_sr_does_attacker_win(leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write all set play of a self/reflex stipulation.
 * @param leaf slice index of the leaf slice
 */
static void d_leaf_sr_solve_setplay(slice_index leaf)
{
  Side const defender = advers(slices[leaf].starter);

  generate_move_reaching_goal(leaf,defender);
  active_slice[nbply] = leaf;

  while(encore())
  {
    if (jouecoup(nbply)
        && leaf_is_goal_reached(defender,leaf))
    {
      write_defense(slices[leaf].u.leaf.goal);
      if (OptFlag[maxsols]) 
        solutions++;
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();
}

/* Find and write defender's set play in self/reflex play
 * @param leaf slice index
 */
void d_leaf_solve_setplay(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].type)
  {
    case STLeaf:
      switch (slices[leaf].u.leaf.end)
      {
        case EDirect:
          /* nothing */
          break;

        case ESelf:
        case EReflex:
        case ESemireflex:
          d_leaf_sr_solve_setplay(leaf);
          break;

        default:
          assert(0);
          break;
      }
      break;

    default:
      assert(0);
      break;
  }
}

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean d_leaf_solve_complete_set(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case ESelf:
      if (!d_leaf_s_does_defender_win(leaf))
      {
        d_leaf_sr_solve_setplay(leaf);
        return true;
      }
      else
        break;

    case EReflex:
    case ESemireflex:
      if (!d_leaf_r_does_defender_win(leaf))
      {
        d_leaf_sr_solve_setplay(leaf);
        return true;
      }
      else
        break;

    default:
      break;
  }

  return false;
}

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param leaf slice index
 */
void d_leaf_solve_variations(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  /* We have just written an attacking move without knowing whether we
   * have to signal Zugzwang in the output.
   * (The occurence of this situation is the reason why write_attack()
   * and write_attack_conclusion() are separate functions).
   *
   * When solving a leaf, we never signal Zugzwang:
   */
  write_attack_conclusion(attack_without_zugzwang);

  d_leaf_solve_final_defender_move(leaf);
}

/* Find and write continuations (i.e. mating moves).
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
static void d_leaf_d_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  generate_move_reaching_goal(leaf,attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    /* TODO optimise echecc() check into d_leaf_is_solved? */
    if (jouecoup(nbply) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && d_leaf_is_solved(leaf))
    {
      write_attack(slices[leaf].u.leaf.goal,attack_regular);
      write_attack_conclusion(attack_without_zugzwang);
      pushtabsol(solutions);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write continuations (i.e. final move pairs)
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
static void d_leaf_sr_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  genmove(attacker);

  active_slice[nbply] = leaf;

  while (encore())
  {
    /* TODO optimise echecc() check into d_leaf_is_solved? */
    if (jouecoup(nbply) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && d_leaf_is_solved(leaf))
    {
      write_attack(no_goal,attack_regular);
      write_attack_conclusion(attack_without_zugzwang);

      marge += 4;
      d_leaf_solve_final_defender_move(leaf);
      marge -= 4;

      pushtabsol(solutions);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write continuations (i.e. mating moves or final move pairs).
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
void d_leaf_solve_continuations(int solutions, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      d_leaf_d_solve_continuations(solutions,leaf);
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      d_leaf_sr_solve_continuations(solutions,leaf);
      break;
    
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find the final (ending) move of a reflex leaf
 * @param leaf slice index
 * @return true iff >=1 ending move was found
 */
static boolean leaf_r_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  if (!d_leaf_r_does_defender_win(leaf))
    found_solution = leaf_sr_solve_final_move(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Solve the set play in a help stipulation
 * @param leaf slice index
 * @return true iff >=1 set play was found
 */
boolean h_leaf_solve_setplay(slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case ESelf:
      result = (!d_leaf_s_does_defender_win(leaf)
                && leaf_sr_solve_final_move(leaf));
      break;

    case EReflex:
    case ESemireflex:
      result = leaf_r_solve_final_move(leaf);
      break;

    case EHelp:
      result = leaf_h_solve_final_move(leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write the solution of a leaf slice
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
boolean leaf_solve(slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",leaf);

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      result = leaf_d_solve(leaf);
      break;

    case EHelp:
      result = leaf_h_solve(leaf);
      break;

    case ESelf:
      result = leaf_s_solve(leaf);
      break;

    case EReflex:
      result = leaf_r_solve(leaf);
      break;

    case ESemireflex:
      result = leaf_semir_solve(leaf);
      break;

    default:
      TraceValue("(unexpected value):%d\n",slices[leaf].u.leaf.end);
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param is_duplex is this for duplex?
 */
void leaf_detect_starter(slice_index leaf, boolean is_duplex)
{
  slices[leaf].starter = no_side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",leaf);
  TraceFunctionParam("%d\n",is_duplex);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      /* normally White, but Black in reci-h# */
      break;

    case ESelf:
    case EReflex:
    case ESemireflex:
      slices[leaf].starter = is_duplex ? Black : White;
      break;
          
    case EHelp:
      slices[leaf].starter = is_duplex ? White : Black;
      break;

    default:
      assert(0);
      break;
  }

  TraceValue("%d\n",slices[leaf].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a leaf. 
 * @param leaf identifies leaf
 * @param s starting side of leaf
 */
void leaf_impose_starter(slice_index leaf, Side s)
{
  slices[leaf].starter = s;
}
