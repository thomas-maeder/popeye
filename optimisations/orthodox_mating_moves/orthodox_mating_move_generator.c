#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "pydata.h"
#include "pyproc.h"
#include "pieces/pawns/promotion.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "solving/move_effect_journal.h"
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

static boolean IsABattery(square KingSquare,
                          square FrontSquare,
                          numvec Direction,
                          Side side,
                          PieNam RearPiece1,
                          PieNam RearPiece2)
{
  square sq_target;
  EndOfLine(FrontSquare, Direction, sq_target);
  if (sq_target==KingSquare)
  {
    square sq_rear;
    EndOfLine(FrontSquare, -Direction,sq_rear);

    {
      PieNam const pi_rear = abs(e[sq_rear]);
      if ((pi_rear==RearPiece1 || pi_rear==RearPiece2)
          && TSTFLAG(spec[sq_rear],side))
        return true;
    }
  }

  return false;
}

static numvec detect_directed_battery(square sq_departure, square sq_king,
                                      Side side,
                                      PieNam rider)
{
  numvec const dir_battery = CheckDir[rider][sq_king-sq_departure];
  numvec result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceEnumerator(Side,side,"");
  TracePiece(rider);
  TraceFunctionParamListEnd();

  if (dir_battery!=0
      && IsABattery(sq_king,sq_departure,dir_battery,side,rider,Queen))
    result = abs(dir_battery);
  else
    result = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static numvec detect_battery(square sq_departure, square sq_king, Side side)
{
  return (detect_directed_battery(sq_departure,sq_king,side,Bishop)
          +detect_directed_battery(sq_departure,sq_king,side,Rook));
}

static void pawn_ep_try_direction(square sq_departure, Side side, numvec dir)
{
  square const sq_arrival = sq_departure+dir;

  if (en_passant_is_capture_possible_to(side,sq_arrival))
  {
    square const pos_capturee = en_passant_find_capturee();
    if (pos_capturee!=initsquare)
    {
      empile(sq_departure,sq_arrival,pos_capturee);
      move_generation_stack[current_move[nbply]].auxiliary = sq_arrival;
    }
  }
}

static void pawn_ep(square sq_departure, Side side)
{
  if (trait[parent_ply[nbply]]!=trait[nbply])
  {
    numvec const dir_forward = side==White ? dir_up : dir_down;
    pawn_ep_try_direction(sq_departure,side,dir_forward+dir_right);
    pawn_ep_try_direction(sq_departure,side,dir_forward+dir_left);
  }
}

static void pawn_no_capture(square sq_departure, numvec dir_battery, square sq_king, Side side)
{
  numvec const dir_forward = side==White ? dir_up : dir_down;
  square const sq_arrival_singlestep = sq_departure+dir_forward;

  if (e[sq_arrival_singlestep]==vide)
  {
    if (dir_battery!=0
        || sq_arrival_singlestep+dir_forward+dir_left == sq_king
        || sq_arrival_singlestep+dir_forward+dir_right == sq_king
        || (ForwardPromSq(side,sq_arrival_singlestep)
            && (CheckDir[Queen][sq_king-sq_arrival_singlestep]
                || CheckDir[Knight][sq_king-sq_arrival_singlestep])))
      empile(sq_departure,sq_arrival_singlestep,sq_arrival_singlestep);

    {
      SquareFlags const double_step = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
      if (TSTFLAG(sq_spec[sq_departure],double_step))
      {
        square const sq_arrival_doublestep = sq_arrival_singlestep+dir_forward;
        if (e[sq_arrival_doublestep]==vide
            && (dir_battery!=0
                || sq_arrival_doublestep+dir_forward+dir_left==sq_king
                || sq_arrival_doublestep+dir_forward+dir_right==sq_king))
          empile(sq_departure,sq_arrival_doublestep,sq_arrival_doublestep);
      }
    }
  }
}

static void pawn_capture(square sq_departure, Side side, numvec dir_battery, square sq_king, numvec leftright)
{
  numvec const dir_forward = side==White ? dir_up : dir_down;
  square const sq_arrival = sq_departure+dir_forward+leftright;

  if (e[sq_arrival]!=vide && piece_belongs_to_opponent(sq_arrival,side))
    if (dir_battery!=0
        || sq_arrival+dir_forward+dir_left == sq_king
        || sq_arrival+dir_forward+dir_right == sq_king
        || (ForwardPromSq(side,sq_arrival)
            && (CheckDir[Queen][sq_king-sq_arrival]
                || CheckDir[Knight][sq_king-sq_arrival])))
      empile(sq_departure,sq_arrival,sq_arrival);
}

static void pawn(square sq_departure, square sq_king, Side side)
{
  SquareFlags const base_square = side==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_departure],base_square))
  {
    numvec const abs_dir_battery = detect_battery(sq_departure,sq_king,side);

    pawn_ep(sq_departure,side);

    if (abs_dir_battery!=dir_up)
      pawn_no_capture(sq_departure,abs_dir_battery,sq_king,side);

    pawn_capture(sq_departure,side,abs_dir_battery,sq_king,dir_left);
    pawn_capture(sq_departure,side,abs_dir_battery,sq_king,dir_right);
  }
}

static void king_neutral(square sq_departure, Side side)
{
  vec_index_type vec_index;
  for (vec_index = vec_queen_start; vec_index<=vec_queen_end; ++vec_index)
  {
    square const sq_arrival = sq_departure+vec[vec_index];
    /* must capture to mate the opponent */
    if (piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void king_nonneutral(square sq_departure, square sq_king, Side side)
{
  numvec const abs_dir_battery = detect_battery(sq_departure,sq_king,side);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (abs_dir_battery>0)
  {
    vec_index_type vec_index;
    for (vec_index = vec_queen_start; vec_index<=vec_queen_end; ++vec_index)
    {
      numvec const dir = vec[vec_index];
      TraceValue("%d\n",dir);
      if (abs(dir)!=abs_dir_battery)
      {
        square const sq_arrival = sq_departure+dir;
        if ((e[sq_arrival]==vide || piece_belongs_to_opponent(sq_arrival,side))
            && move_diff_code[abs(sq_king-sq_arrival)]>1+1) /* no contact */
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void king(square sq_departure, square sq_king, Side side)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (king_square[White]==king_square[Black])
    king_neutral(sq_departure,side);
  else
    king_nonneutral(sq_departure,sq_king,side);

  if (castling_supported)
    generate_castling(side);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void knight(square sq_departure, square sq_king, Side side)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  {
    numvec const abs_dir_battery = detect_battery(sq_departure,sq_king,side);
    numvec vec_to_king = abs(sq_king-sq_departure);

    if (abs_dir_battery!=0
        || (SquareCol(sq_departure)==SquareCol(sq_king)
            && move_diff_code[vec_to_king]<=move_diff_code[square_a3-square_e1]
            && move_diff_code[vec_to_king]!=move_diff_code[square_a3-square_c1]))
    {
      vec_index_type vec_index;
      for (vec_index = vec_knight_start; vec_index<=vec_knight_end; ++vec_index)
      {
        square const sq_arrival = sq_departure+vec[vec_index];
        if (e[sq_arrival]==vide || piece_belongs_to_opponent(sq_arrival,side))
          if (abs_dir_battery!=0 || CheckDir[Knight][sq_arrival-sq_king]!=0)
            empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider_try_moving_to(square sq_departure, square sq_king,
                                square sq_arrival, numvec dir_to_king)
{
  square sq_target;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceSquare(sq_arrival);
  TraceValue("%d",dir_to_king);
  TraceFunctionParamListEnd();

  EndOfLine(sq_arrival,dir_to_king,sq_target);
  if (sq_target==sq_king)
    empile(sq_departure,sq_arrival,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void queen_try_moving_to(square sq_departure, square sq_king, square sq_arrival)
{
  numvec const dir_to_king = CheckDir[Queen][sq_king-sq_arrival];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  if (dir_to_king!=0)
    rider_try_moving_to(sq_departure,sq_king,sq_arrival,dir_to_king);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void queen(square sq_departure, square sq_king, Side side)
{
  vec_index_type vec_index;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  for (vec_index = vec_queen_start; vec_index<=vec_queen_end; ++vec_index)
  {
    numvec const dir = vec[vec_index];

    square sq_arrival;
    for (sq_arrival = sq_departure+dir; e[sq_arrival]==vide; sq_arrival += dir)
      queen_try_moving_to(sq_departure,sq_king,sq_arrival);

    if (piece_belongs_to_opponent(sq_arrival,side))
      queen_try_moving_to(sq_departure,sq_king,sq_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void simple_rider_fire_battery(square sq_departure,
                                      Side side,
                                      vec_index_type index_start,
                                      vec_index_type index_end)
{
  vec_index_type vec_index;
  for (vec_index = index_start; vec_index<=index_end; ++vec_index)
  {
    numvec const dir = vec[vec_index];
    square sq_arrival;

    for (sq_arrival = sq_departure+dir; e[sq_arrival]==vide; sq_arrival += dir)
      empile(sq_departure,sq_arrival,sq_arrival);

    if (piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void simple_rider_directly_approach_king(square sq_departure,
                                                square sq_king,
                                                Side side,
                                                numvec dir_to_king)
{
  square sq_arrival;
  EndOfLine(sq_departure,dir_to_king,sq_arrival);
  if (piece_belongs_to_opponent(sq_arrival,side))
    rider_try_moving_to(sq_departure,sq_king,sq_arrival,dir_to_king);
}

static void simple_rider_indirectly_approach_king(square sq_departure,
                                                  square sq_king,
                                                  Side side,
                                                  vec_index_type index_start,
                                                  vec_index_type index_end,
                                                  PieNam rider_walk)
{
  move_diff_type const OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];
  vec_index_type vec_index;
  for (vec_index = index_start; vec_index<=index_end; ++vec_index)
  {
    numvec const dir = vec[vec_index];
    square sq_arrival = sq_departure+dir;
    if (e[sq_arrival]!=obs
        && move_diff_code[abs(sq_arrival-sq_king)]<OriginalDistance)
    {
      /* The rider must move closer to the king! */
      numvec dir_to_king = CheckDir[rider_walk][sq_king-sq_arrival];
      while (dir_to_king==0 && e[sq_arrival]==vide)
      {
        sq_arrival += dir;
        dir_to_king = CheckDir[rider_walk][sq_king-sq_arrival];
      }

      /* We are at the end of the line or in checking distance */
      if (dir_to_king!=0
          && (e[sq_arrival]==vide || piece_belongs_to_opponent(sq_arrival,side)))
        rider_try_moving_to(sq_departure,sq_king,sq_arrival,dir_to_king);
    }
  }
}

static void rook(square sq_departure, square sq_king, Side side)
{
  numvec const dir_battery = detect_directed_battery(sq_departure,sq_king,side,Bishop);

  if (dir_battery!=0)
    simple_rider_fire_battery(sq_departure,side,vec_rook_start,vec_rook_end);
  else
  {
    numvec const dir_to_king = CheckDir[Rook][sq_king-sq_departure];
    if (dir_to_king==0)
      simple_rider_indirectly_approach_king(sq_departure,sq_king,side,
                                            vec_rook_start,vec_rook_end,
                                            Rook);
    else
      simple_rider_directly_approach_king(sq_departure,sq_king,side,dir_to_king);
  }
}

static void bishop(square sq_departure, square sq_king, Side side)
{
  numvec const dir_battery = detect_directed_battery(sq_departure,sq_king,side,Rook);

  if (dir_battery!=0)
    simple_rider_fire_battery(sq_departure,side,vec_bishop_start,vec_bishop_end);
  else if (SquareCol(sq_departure)==SquareCol(sq_king))
  {
    numvec const dir_to_king = CheckDir[Bishop][sq_king-sq_departure];
    if (dir_to_king==0)
      simple_rider_indirectly_approach_king(sq_departure,sq_king,side,
                                           vec_bishop_start,vec_bishop_end,
                                           Bishop);
    else
      simple_rider_directly_approach_king(sq_departure,sq_king,side,dir_to_king);
  }
}

static void generate_move_reaching_goal()
{
  square square_a = square_a1;
  Side const side_at_move = trait[nbply];
  square const OpponentsKing = side_at_move==White ? king_square[Black] : king_square[White];
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (OpponentsKing!=initsquare)
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
            generate_moves_for_piece(side_at_move,sq_departure,p);
          else
            switch (abs(p))
            {
              case King:
                king(sq_departure,OpponentsKing,side_at_move);
                break;

              case Pawn:
                pawn(sq_departure,OpponentsKing,side_at_move);
                break;

              case Knight:
                knight(sq_departure,OpponentsKing,side_at_move);
                break;

              case Rook:
                rook(sq_departure,OpponentsKing,side_at_move);
                break;

              case Queen:
                queen(sq_departure,OpponentsKing,side_at_move);
                break;

              case Bishop:
                bishop(sq_departure,OpponentsKing,side_at_move);
                break;
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
