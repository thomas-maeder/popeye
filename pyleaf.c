#include "pyleaf.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyoutput.h"

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


static unsigned int nr_ortho_mating_moves_generation_obstacles;

/* Reset the number of obstacles that might prevent
 * generate_move_reaching_goal() from optimising by only generating
 * orthodox moves
 */
void reset_ortho_mating_moves_generation_obstacles(void)
{
  nr_ortho_mating_moves_generation_obstacles = 0;
}

/* Add an obstacle.
 */
void add_ortho_mating_moves_generation_obstacle(void)
{
  ++nr_ortho_mating_moves_generation_obstacles;
}

/* Remove an obstacle.
 */
void remove_ortho_mating_moves_generation_obstacle(void)
{
  assert(nr_ortho_mating_moves_generation_obstacles>0);
  --nr_ortho_mating_moves_generation_obstacles;
}

static void generate_ortho_moves_reaching_goal(Goal goal, Side side_at_move)
{
  square square_a = square_a1;
  square const OpponentsKing = side_at_move==White ? rn : rb;
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();

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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for side side_at_move; optimise for moves reaching a
 * specific goal.
 * @param leaf leaf slice whose goal is to be reached by generated
 *             move(s)
 * @param side_at_move side for which to generate moves
 */
void generate_move_reaching_goal(slice_index leaf, Side side_at_move)
{
  Goal const goal = slices[leaf].u.leaf.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();

  if (side_at_move==White ? !flagwhitemummer : !flagblackmummer)
    empile_for_goal_of_leaf_slice = leaf;

  active_slice[nbply+1] = leaf;
  switch (goal)
  {
    case goal_mate:
    case goal_check:
      TraceValue("%u\n",nr_ortho_mating_moves_generation_obstacles);
      if (nr_ortho_mating_moves_generation_obstacles==0)
        generate_ortho_moves_reaching_goal(goal,side_at_move);
      else
        genmove(side_at_move);
      break;

    case goal_doublemate:
      if (immobile(side_at_move))
      {
        TraceText("attacker is immobile\n");
        nextply(nbply);
      }
      else if (nr_ortho_mating_moves_generation_obstacles==0)
        generate_ortho_moves_reaching_goal(goal,side_at_move);
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

    case goal_countermate:
      /* TODO can this be generalised to non-mate goals? */
      if (goal_checker_mate(advers(side_at_move))==goal_reached)
        /* TODO only generate king and ortho moves if there are no
         * obstacles?
         */
        genmove(side_at_move);
      else
        nextply(nbply);
      break;

    default:
      genmove(side_at_move);
      break;
  }

  empile_for_goal_of_leaf_slice = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* generate_move_reaching_goal */


/* Determine whether the mating side still has a piece that could
 * deliver the mate.
 * @return true iff the mating side has such a piece
 */
boolean is_a_mating_piece_left(Side mating_side)
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
 * @return whether camp has reached leaf's goal
 */
goal_checker_result_type leaf_is_goal_reached(Side just_moved,
                                              slice_index leaf)
{
  boolean result = goal_not_reached;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,just_moved,"");
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();
  assert(slices[leaf].starter!=no_side);

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
      result = goal_checker_proof(just_moved);
      break;

    case goal_mate_or_stale:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(goal_checker_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void leaf_impose_starter(slice_index si, stip_structure_traversal *st)
{
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
