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

/* TODO leaf_h_has_solution vs. leaf_h_is_solvable */

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
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u\n",side_at_move);

  if (side_at_move==White ? !flagwhitemummer : !flagblackmummer)
    empile_for_goal_of_leaf_slice = leaf;

  switch (goal)
  {
    case goal_mate:
    case goal_doublemate:
    case goal_check:
      TraceValue("%u\n",optim_orthomatingmoves);
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
  TraceFunctionParam("%u",just_moved);
  TraceFunctionParam("%u\n",leaf);
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceValue("%u\n",slices[leaf].u.leaf.goal);
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
  TraceFunctionResult("%u\n",result);
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
    Side const attacker = slices[leaf].u.leaf.starter;
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
  Side const defender = advers(slices[leaf].u.leaf.starter);
  boolean is_defender_immobile = true;
  boolean escape_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u\n",slices[leaf].u.leaf.goal);

  if (defender==Black ? flagblackmummer : flagwhitemummer)
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    genmove(defender);
    move_generation_mode = move_generation_optimized_by_killer_move;

    while (!escape_found
           && encore())
    {
      if (jouecoup(nbply,first_play)
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
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
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
  TraceValue("%u",escape_found);
  TraceValue("%u",is_defender_immobile);
  TraceFunctionResult("%u\n", !(escape_found || is_defender_immobile));
  return !(escape_found || is_defender_immobile);
}

/* Determine whether a side has an end in 1.
 * @param side_at_move
 * @param leaf slice index
 * @return true iff side_at_move can end in 1 move
 */
static boolean leaf_is_end_in_1_possible(Side side_at_move, slice_index leaf)
{
  boolean end_found = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u\n",leaf);

  generate_move_reaching_goal(leaf,side_at_move);

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore() && !end_found)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      end_found = true;
      coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",end_found);
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
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
    {
      Side const attacker = slices[leaf].u.leaf.starter;
      result = OptFlag[keepmating] && !is_a_mating_piece_left(attacker);
      break;
    }

    case ESelf:
    case EHelp:
    {
      Side const final = advers(slices[leaf].u.leaf.starter);
      result = OptFlag[keepmating] && !is_a_mating_piece_left(final);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect whether a move delivering countermate exists.
 * @param leaf identifies leaf slice
 * @return true iff such a move exists
 */
static boolean leaf_h_cmate_exists_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  if (goal_checker_mate(side_at_move))
  {
    generate_move_reaching_goal(leaf,other_side);

    while (encore() && !found_solution)
    {
      if (jouecoup(nbply,first_play))
        found_solution = leaf_is_goal_reached(other_side,leaf);

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Detect whether a help countermate is solvable
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution exists
 */
static boolean leaf_h_cmate_is_solvable(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;

  generate_move_reaching_goal(leaf,side_at_move);

  while (encore() && !found_solution)
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,side_at_move))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_h_cmate_exists_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_h_cmate_exists_final_move(leaf))
        found_solution = true;
    }

    repcoup();
  }

  finply();

  return found_solution;
}

/* Detect whether a move delivering doublemate exists
 * @param leaf identifies leaf slice
 * @return true iff such a move exists
 */
static boolean leaf_h_dmate_exists_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  if (!immobile(other_side))
  {
    generate_move_reaching_goal(leaf,other_side);

    while (encore() && !found_solution)
    {
      if (jouecoup(nbply,first_play))
        found_solution = leaf_is_goal_reached(other_side,leaf);

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Detect whether a help doublemate is solvable
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution exists
 */
static boolean leaf_h_dmate_is_solvable(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;

  genmove(side_at_move);

  while (encore() && !found_solution)
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,side_at_move))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_h_dmate_exists_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_h_dmate_exists_final_move(leaf))
        found_solution = true;
    }

    repcoup();
  }

  finply();

  return found_solution;
}

/* Detect whether a help leaf with goal other than
 * {double,counter}mate is solvable 
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution exists
 */
static boolean leaf_h_regulargoals_is_solvable(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  genmove(side_at_move);

  if (side_at_move==Black)
    BlMovesLeft--;
  else
    WhMovesLeft--;

  while (encore() && !found_solution)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && (!OptFlag[keepmating] || is_a_mating_piece_left(other_side)))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_is_end_in_1_possible(other_side,leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_is_end_in_1_possible(other_side,leaf))
        found_solution = true;
    }

    repcoup();
  }
    
  if (side_at_move==Black)
    BlMovesLeft++;
  else
    WhMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
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
  TraceFunctionParam("%u\n",leaf);

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
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Look up the current position in the hash table
 * @param leaf slice index of leaf slice
 * @param hb contains encoded position
 * @return if found in hash table: DirSucc or DirNoSucc
 *         otherwise: nr_hashwhat
 */
static hashwhat leaf_d_hash_lookup(slice_index leaf, HashBuffer *hb)
{
  /* It is more likely that a position has no solution. */
  /*    Therefore let's check for "no solution" first. TLi */
  if (inhash(leaf,DirNoSucc,1,hb))
  {
    assert(!inhash(leaf,DirSucc,0,hb));
    return DirNoSucc;
  }
  else if (inhash(leaf,DirSucc,0,hb))
    return DirSucc;
  else
    return nr_hashwhat;
}

/* Update/insert hash table entry for current position
 * @param leaf slice index of leaf slice
 * @param hb contains encoded position
 * @param h DirSucc for writing DirSucc; other values for writing
 *          DirNoSucc
 */
static void leaf_d_hash_update(slice_index leaf, HashBuffer *hb, hashwhat h)
{
  if (h==DirSucc)
    addtohash(leaf,DirSucc,0,hb);
  else
    addtohash(leaf,DirNoSucc,1,hb);
}

/* Determine whether there is a solution in a direct leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker can end in 1 move
 */
static boolean leaf_d_has_solution(slice_index leaf)
{
  hashwhat result = nr_hashwhat;
  HashBuffer hb;
  Side const attacker = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  /* In move orientated stipulations (%, z, x etc.) it's less
   * expensive to compute an end in 1. TLi
   */
  if (!FlagMoveOrientatedStip)
  {
    (*encode)(&hb);
    result = leaf_d_hash_lookup(leaf,&hb);
  }

  if (result==nr_hashwhat)
  {
    if (OptFlag[keepmating] && !is_a_mating_piece_left(attacker))
    {
      TraceText("!is_a_mating_piece_left\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",false);
      return false;
    }

    if (slices[leaf].u.leaf.goal==goal_doublemate
        && immobile(attacker))
    {
      TraceText("attacker is immobile\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",false);
      return false;
    }

    generate_move_reaching_goal(leaf,attacker);

    while (encore() && result!=DirSucc)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && leaf_is_goal_reached(attacker,leaf))
        {
          result = DirSucc;
          coupfort();
        }

      repcoup();

      if (maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    finply();

    if (!FlagMoveOrientatedStip)
      leaf_d_hash_update(leaf,&hb,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result==DirSucc);
  return result==DirSucc;
}

/* Determine whether there is a solution in a help leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
static boolean leaf_h_has_solution(slice_index leaf)
{
  boolean result = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  genmove(side_at_move);

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && !(OptFlag[keepmating] && !is_a_mating_piece_left(other_side)))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_is_end_in_1_possible(other_side,leaf))
          {
            result = true;
            coupfort();
          }
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_is_end_in_1_possible(other_side,leaf))
      {
        result = true;
        coupfort();
      }
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there is a solution in a self leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
static boolean leaf_s_has_solution(slice_index leaf)
{
  hashwhat result;
  HashBuffer hb;
  Side const attacker = slices[leaf].u.leaf.starter;
  Side const defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  (*encode)(&hb);
  result = leaf_d_hash_lookup(leaf,&hb);

  if (result==nr_hashwhat)
  {
    genmove(attacker);

    while (result!=DirSucc
           && encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !echecc(nbply,attacker)
          && !(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
          && leaf_is_end_in_1_forced(leaf))
      {
        result = DirSucc;
        coupfort();
      }

      repcoup();

      if (maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    finply();

    leaf_d_hash_update(leaf,&hb,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result==DirSucc);
  return result==DirSucc;
}

/* Determine whether there is >= 1 solution for the leaf
 * @param leaf slice index of leaf slice
 * @return true iff side_at_move has >=1 solution
 */
boolean leaf_is_solvable(slice_index leaf)
{
  boolean result = false;
  
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);
  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
    {
      Side const attacker = slices[leaf].u.leaf.starter;
      if (OptFlag[keepmating] && !is_a_mating_piece_left(attacker))
        ; /* intentionally nothing */
      else
        result = leaf_d_has_solution(leaf);
      break;
    }

    case ESelf:
    {
      Side const defender = advers(slices[leaf].u.leaf.starter);
      if (OptFlag[keepmating] && !is_a_mating_piece_left(defender))
        ; /* intentionally nothing */
      else
        result = leaf_s_has_solution(leaf);
      break;
    }

    case EHelp:
    {
      Side const defender = advers(slices[leaf].u.leaf.starter);
      if (OptFlag[keepmating] && !is_a_mating_piece_left(defender))
        ; /* intentionally nothing */
      else
        result = leaf_h_is_solvable(leaf);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_has_non_starter_solved(slice_index leaf)
{
  Side const defender = advers(slices[leaf].u.leaf.starter);
  boolean result = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",defender);
  TraceFunctionParam("%u\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      break;

    case ESelf:
    case EHelp:
      result = leaf_is_goal_reached(defender,leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the starter 
 * @param leaf slice index
 * @return true iff the leaf's starter has just solved leaf
 */
boolean leaf_has_starter_solved(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;

  assert(slices[leaf].type==STLeaf);
  assert(attacker!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return leaf_is_goal_reached(attacker,leaf);

    case ESelf:
      return leaf_is_end_in_1_forced(leaf);

    case EHelp:
    {
      Side const defender = advers(attacker);
      return (!(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
              && leaf_is_end_in_1_possible(defender,leaf));
    }

    default:
      assert(0);
      return false;
  }
}

/* Write a priori unsolvability (if any) of a leaf (e.g. forced reflex
 * mates)
 * @param leaf leaf's slice index
 */
void leaf_write_unsolvability(slice_index leaf)
{
}

/* Determine and write keys if the end is direct
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  generate_move_reaching_goal(leaf,attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && leaf_is_goal_reached(attacker,leaf))
    {
      solution_found = true;
      write_attack(slices[leaf].u.leaf.goal,attack_key);
      output_start_leaf_variation_level();
      output_end_leaf_variation_level();
      write_end_of_solution();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
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
  Side const side_at_move = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceValue("%u\n",side_at_move);

  generate_move_reaching_goal(leaf,side_at_move);
  active_slice[nbply] = leaf;

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !(isIntelligentModeActive && !isGoalReachable())
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      final_move_found = true;
      write_final_help_move(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",final_move_found);
  return final_move_found;
}
#endif

/* Determine and find final moves of a self leaf
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean leaf_s_solve_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceValue("%u\n",side_at_move);

  generate_move_reaching_goal(leaf,side_at_move);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      final_move_found = true;
      write_final_defense(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",final_move_found);
  return final_move_found;
}

/* Determine and write solutions in a self stipulation in 1 move
 * @param leaf slice index of the leaf slice
 * @return true iff >=1 key was found and written
 */
static boolean leaf_s_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const attacker = slices[leaf].u.leaf.starter;
  Side const defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  genmove(attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && !(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
        && leaf_is_end_in_1_forced(leaf))
    {
      found_solution = true;

      write_attack(no_goal,attack_key);
      output_start_postkey_level();
      leaf_s_solve_final_move(leaf);
      output_end_postkey_level();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Solve the final move for a countermate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_cmate_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(slices[leaf].u.leaf.starter);

  if (goal_checker_mate(side_at_move))
  {
    generate_move_reaching_goal(leaf,other_side);
    active_slice[nbply] = leaf;

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(other_side,leaf))
      {
        found_solution = true;
        write_final_help_move(goal_countermate);
      }
      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the final move pair in help countermate.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_cmate_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;

  generate_move_reaching_goal(leaf,side_at_move);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,side_at_move))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_h_cmate_solve_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_h_cmate_solve_final_move(leaf))
        found_solution = true;
    }

    repcoup();
  }

  finply();

  return found_solution;
}

/* Solve the final move for a doublemate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_dmate_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const final_side = advers(slices[leaf].u.leaf.starter);

  if (!immobile(final_side))
  {
    generate_move_reaching_goal(leaf,final_side);
    active_slice[nbply] = leaf;

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(final_side,leaf))
      {
        found_solution = true;
        write_final_help_move(goal_doublemate);
      }

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the final move pair in help doublemate.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_dmate_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;

  genmove(side_at_move);
  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,side_at_move))
    {
      if (compression_counter==0)
      {
        HashBuffer hb;
        (*encode)(&hb);
        if (!inhash(leaf,HelpNoSuccOdd,1,&hb))
        {
          if (leaf_h_dmate_solve_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1,&hb);
        }
      }
      else if (leaf_h_dmate_solve_final_move(leaf))
        found_solution = true;
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  return found_solution;
}

/* Determine and write the final move pair in help stipulation with
 * "regular" goal. 
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_regulargoals_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  genmove(side_at_move);
  active_slice[nbply] = leaf;

  if (side_at_move==Black)
    BlMovesLeft--;
  else
    WhMovesLeft--;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && (!OptFlag[keepmating] || is_a_mating_piece_left(other_side)))
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
  TraceFunctionResult("%u\n",found_solution);
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
  TraceFunctionParam("%u\n",leaf);

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
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key and solve the remainder of a leaf in direct play
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_root_write_key_solve_postkey(slice_index leaf, attack_type type)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      write_attack(slices[leaf].u.leaf.goal,type);
      output_start_leaf_variation_level();
      output_end_leaf_variation_level();
      break;

    case ESelf:
    case EHelp:
      write_attack(no_goal,type);
      output_start_leaf_variation_level();
      if (OptFlag[solvariantes])
        leaf_h_solve_final_move(leaf);
      output_end_leaf_variation_level();
      break;

    default:
      assert(0);
      break;
  }

  /* TODO why here? */
  Message(NewLine);
}

/* Determine whether the starting side has lost with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_has_starter_lost(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return false;

    case ESelf:
    case EHelp:
      return (OptFlag[keepmating]
              && !is_a_mating_piece_left(slices[leaf].u.leaf.starter));

    default:
      assert(0);
      return false;
  }
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_has_starter_won(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      return leaf_is_goal_reached(slices[leaf].u.leaf.starter,leaf);

    case ESelf:
      return false;

    case EHelp:
    {
      Side const defender = advers(slices[leaf].u.leaf.starter);
      return (!(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
              && leaf_is_end_in_1_possible(defender,leaf));
    }

    default:
      assert(0);
      return false;
  }
}

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index
 * @return true iff leaf has >=1 solution
 */
boolean leaf_has_solution(slice_index leaf)
{
  boolean result = false;

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      result = leaf_d_has_solution(leaf);
      break;

    case ESelf:
      result = leaf_s_has_solution(leaf);
      break;

    case EHelp:
      result = leaf_h_has_solution(leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write set play of a self/reflex stipulation.
 * @param leaf slice index of the leaf slice
 */
static boolean leaf_root_sr_solve_setplay(slice_index leaf)
{
  boolean result = false;
  Side const defender = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceValue("%u\n",defender);

  generate_move_reaching_goal(leaf,defender);
  active_slice[nbply] = leaf;

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && leaf_is_goal_reached(defender,leaf))
    {
      result = true;
      write_final_defense(slices[leaf].u.leaf.goal);
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

  return result;
}

/* Find and write defender's set play
 * @param leaf slice index
 */
boolean leaf_root_solve_setplay(slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      /* nothing */
      break;

    case ESelf:
      result = leaf_root_sr_solve_setplay(leaf);
      break;

    case EHelp:
      result = leaf_h_solve_final_move(leaf);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write set play provided every set move leads to end
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean leaf_root_solve_complete_set(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case ESelf:
    {
      Side const defender = advers(slices[leaf].u.leaf.starter);
      if (!(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
          && leaf_is_end_in_1_forced(leaf))
      {
        leaf_root_sr_solve_setplay(leaf);
        return true;
      }
      else
        break;
    }

    case EHelp:
    {
      Side const defender = advers(slices[leaf].u.leaf.starter);
      if (!(OptFlag[keepmating] && !is_a_mating_piece_left(defender)))
        return leaf_root_sr_solve_setplay(leaf);
      else
        break;
    }

    default:
      break;
  }

  return false;
}

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param leaf slice index
 */
void leaf_solve_variations(slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      /* nothing */
      break;

    case ESelf:
    case EHelp:
      output_start_leaf_variation_level();
      leaf_h_solve_final_move(leaf);
      output_end_leaf_variation_level();
      break;
    
    default:
      assert(0);
      break;
  }
}

/* Find and write continuations (i.e. mating moves).
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
static void leaf_d_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  generate_move_reaching_goal(leaf,attacker);
  active_slice[nbply] = leaf;

  while (encore())
  {
    /* TODO optimise echecc() check into leaf_is_goal_reached? */
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && leaf_is_goal_reached(attacker,leaf))
    {
      write_attack(slices[leaf].u.leaf.goal,attack_regular);
      output_start_leaf_variation_level();
      output_end_leaf_variation_level();
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
static void leaf_s_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  genmove(attacker);

  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && leaf_is_end_in_1_forced(leaf))
    {
      write_attack(no_goal,attack_regular);
      output_start_postkey_level();
      leaf_s_solve_final_move(leaf);
      output_end_postkey_level();
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
static void leaf_h_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;
  Side const defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  genmove(attacker);

  active_slice[nbply] = leaf;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && !(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
        && leaf_is_end_in_1_possible(defender,leaf))
    {
      write_attack(no_goal,attack_regular);
      output_start_postkey_level();
      leaf_h_solve_final_move(leaf);
      output_end_postkey_level();
      pushtabsol(solutions);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write continuations (i.e. mating moves or final move pairs).
 * @param solutions table where to append continuations found and
 *                  written
 * @param leaf slice index
 */
void leaf_solve_continuations(int solutions, slice_index leaf)
{
  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  switch (slices[leaf].u.leaf.end)
  {
    case EDirect:
      leaf_d_solve_continuations(solutions,leaf);
      break;

    case ESelf:
      leaf_s_solve_continuations(solutions,leaf);
      break;
    
    case EHelp:
      leaf_h_solve_continuations(solutions,leaf);
      break;
    
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the solution of a leaf slice
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
boolean leaf_solve(slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].type==STLeaf);
  assert(slices[leaf].u.leaf.starter!=no_side);

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

    default:
      TraceValue("(unexpected value):%u\n",slices[leaf].u.leaf.end);
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param is_duplex is this for duplex?
 */
void leaf_detect_starter(slice_index leaf, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);

  TraceFunctionParam("%u\n",is_duplex);

  if (slices[leaf].u.leaf.starter==no_side)
    switch (slices[leaf].u.leaf.end)
    {
      case EDirect:
        /* normally White, but Black in reci-h */
        break;

      case ESelf:
        slices[leaf].u.leaf.starter = is_duplex ? Black : White;
        break;
          
      case EHelp:
        slices[leaf].u.leaf.starter = is_duplex ? White : Black;
        break;

      default:
        assert(0);
        break;
    }

  TraceValue("%u\n",slices[leaf].u.leaf.starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a leaf. 
 * @param leaf identifies leaf
 * @param s starting side of leaf
 */
void leaf_impose_starter(slice_index leaf, Side s)
{
  slices[leaf].u.leaf.starter = s;
}
