#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Allocate a STOrthodoxMatingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_orthodox_mating_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOrthodoxMatingMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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

  k= CheckDir[Bishop][sq_king-sq_departure];
  if (k!=0)
    Battery=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else {
    k= CheckDir[Rook][sq_king-sq_departure];
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
      if (ep[parent_ply[nbply]]!=initsquare
          && trait[parent_ply[nbply]]!=trait[nbply]
          && (sq_departure+dir_up+dir_right==ep[parent_ply[nbply]]
              || sq_departure+dir_up+dir_left==ep[parent_ply[nbply]]))
      {
        if (nbply==2)    /* ep.-key  standard pawn */
          move_generation_stack[current_move[1]].arrival= ep[parent_ply[nbply]]+dir_down;
        empile(sq_departure,
               ep[parent_ply[nbply]],
               move_generation_stack[current_move[parent_ply[nbply]]].arrival);
      }

      /* single step */
      if (k!=dir_up) {
        /* suppress moves along a battery line */
        sq_arrival= sq_departure+dir_up;
        if (e[sq_arrival]==vide) {
          if (Battery
              || sq_arrival+dir_up+dir_left == sq_king
              || sq_arrival+dir_up+dir_right == sq_king
              || (PromSq(White,sq_arrival)
                  && (CheckDir[Queen][sq_king-sq_arrival]
                      || CheckDir[Knight][sq_king-sq_arrival])))
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
                && (CheckDir[Queen][sq_king-sq_arrival]
                    || CheckDir[Knight][sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);

      /* capture+dir_up+dir_right */
      sq_arrival= sq_departure+dir_up+dir_right;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left==sq_king
            || sq_arrival+dir_up+dir_right==sq_king
            || (PromSq(White,sq_arrival)
                && (CheckDir[Queen][sq_king-sq_arrival]
                    || CheckDir[Knight][sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    if (sq_departure>=square_a8)
      return;

    /* not last rank */
    /* ep captures */
    if (ep[parent_ply[nbply]]!=initsquare
        && trait[parent_ply[nbply]] != trait[nbply]
        && (sq_departure+dir_down+dir_left==ep[parent_ply[nbply]]
            || sq_departure+dir_down+dir_right==ep[parent_ply[nbply]]))
    {
      if (nbply==2)    /* ep.-key  standard pawn */
        move_generation_stack[current_move[1]].arrival= ep[parent_ply[nbply]]+dir_up;
      empile(sq_departure,
             ep[parent_ply[nbply]],
             move_generation_stack[current_move[parent_ply[nbply]]].arrival);
    }

    /* single step */
    if (k!=dir_up) {    /* suppress moves along a battery line */
      sq_arrival= sq_departure+dir_down;
      if (e[sq_arrival]==vide) {
        if (Battery
            || sq_arrival+dir_down+dir_right==sq_king
            || sq_arrival+dir_down+dir_left==sq_king
            || (PromSq(Black,sq_arrival)
                && (CheckDir[Queen][sq_king-sq_arrival]
                    || CheckDir[Knight][sq_king-sq_arrival])))
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
              && (CheckDir[Queen][sq_king-sq_arrival]
                  || CheckDir[Knight][sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }

    /* capture+dir_up+dir_right */
    sq_arrival= sq_departure+dir_down+dir_left;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(Black,sq_arrival)
              && (CheckDir[Queen][sq_king-sq_arrival]
                  || CheckDir[Knight][sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
} /* GenMatingPawn */

static void GenMatingKing(square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  ColourSpec const ColourCapturedPiece = advers(ColourMovingPiece);

  if (king_square[White]==king_square[Black])
  {
    numvec k;

    /* neutral kings */
    for (k= vec_queen_start; k<=vec_queen_end; k++)
    {
      square const sq_arrival = sq_departure+vec[k];
      /* they must capture to mate the opponent */
      if (e[sq_arrival]!=vide
          && TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else
  {
    boolean is_king_battery_front_piece = false;

    numvec k = CheckDir[Bishop][sq_king-sq_departure];
    if (k!=0)
      is_king_battery_front_piece = IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
    else
    {
      k = CheckDir[Rook][sq_king-sq_departure];
      if (k!=0)
        is_king_battery_front_piece = IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);
    }

    if (is_king_battery_front_piece)
    {
      numvec k2;
      for (k2 = vec_queen_start; k2<=vec_queen_end; k2++)
      {
        /* prevent the king from moving along the battery line*/
        if (k2!=k && k2!=-k)
        {
          square const sq_arrival = sq_departure+vec[k2];
          if ((e[sq_arrival]==vide
               || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
              && move_diff_code[abs(sq_king-sq_arrival)]>1+1) /* no contact */
            empile(sq_departure,sq_arrival,sq_arrival);
        }
      }
    }
  }

  /* castling */
  if (castling_supported)
    generate_castling(ColourMovingPiece);
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
  if ((k = CheckDir[Bishop][sq_king-sq_departure])!=0)
    Generate=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else if ((k = CheckDir[Rook][sq_king-sq_departure])!=0)
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
        if (Generate || CheckDir[Knight][sq_arrival-sq_king]!=0)
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
  k= CheckDir[Bishop][sq_king-sq_departure];
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

    k2= CheckDir[Rook][sq_king-sq_departure];
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
          k2= CheckDir[Rook][sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDir[Rook][sq_king-sq_arrival];
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
      k2= CheckDir[Queen][sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
      sq_arrival+= vec[k];
    }
    if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
      k2= CheckDir[Queen][sq_king-sq_arrival];
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
  k = CheckDir[Rook][sq_king-sq_departure];
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

    k2= CheckDir[Bishop][sq_king-sq_departure];
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
          k2= CheckDir[Bishop][sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDir[Bishop][sq_king-sq_arrival];
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

static void generate_move_reaching_goal()
{
  square square_a = square_a1;
  Side const side_at_move = trait[nbply];
  square const OpponentsKing = side_at_move==White ? king_square[Black] : king_square[White];
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

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
          switch (abs(p))
          {
            case King:
              GenMatingKing(sq_departure,OpponentsKing,side_at_move);
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type orthodox_mating_move_generator_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length+1);

  nextply();
  trait[nbply]= slices[si].starter;
  generate_move_reaching_goal();
  result = solve(slices[si].next1,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
