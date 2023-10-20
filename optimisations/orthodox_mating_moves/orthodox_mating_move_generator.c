#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "conditions/grid.h"
#include "pieces/walks/generate_moves.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/pipe.h"
#include "optimisations/orthodox_check_directions.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/move_diff_code.h"
#include "conditions/conditions.h"

#include "debugging/assert.h"
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
                          numvec Direction,
                          Side side,
                          piece_walk_type RearPiece1,
                          piece_walk_type RearPiece2)
{
  square const FrontSquare = curr_generation->departure;
  square const sq_target = find_end_of_line(FrontSquare,Direction);
  if (sq_target==KingSquare)
  {
    square const sq_rear = find_end_of_line(FrontSquare,-Direction);
    piece_walk_type const pi_rear = get_walk_of_piece_on_square(sq_rear);
    if ((pi_rear==RearPiece1 || pi_rear==RearPiece2)
        && TSTFLAG(being_solved.spec[sq_rear],side))
      return true;
  }

  return false;
}

static numvec detect_directed_battery(square sq_king, Side side, piece_walk_type rider)
{
  numvec const dir_battery = CheckDir(rider)[sq_king-curr_generation->departure];
  numvec result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceEnumerator(Side,side);
  TraceWalk(rider);
  TraceFunctionParamListEnd();

  if (dir_battery!=0
      && IsABattery(sq_king,dir_battery,side,rider,Queen))
    result = abs(dir_battery);
  else
    result = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static numvec detect_battery(square sq_king, Side side)
{
  return (detect_directed_battery(sq_king,side,Bishop)
          +detect_directed_battery(sq_king,side,Rook));
}

static void pawn_ep_try_direction(Side side, numvec dir)
{
  curr_generation->arrival = curr_generation->departure+dir;

  if (en_passant_is_capture_possible_to(side,curr_generation->arrival))
  {
    square const pos_capturee = en_passant_find_capturee();
    if (pos_capturee!=initsquare)
      push_special_move(offset_en_passant_capture+pos_capturee);
  }
}

static void pawn_ep(Side side)
{
  if (trait[parent_ply[nbply]]!=trait[nbply])
  {
    numvec const dir_forward = side==White ? dir_up : dir_down;
    pawn_ep_try_direction(side,dir_forward+dir_right);
    pawn_ep_try_direction(side,dir_forward+dir_left);
  }
}

static void pawn_no_capture(numvec dir_battery, square sq_king, Side side)
{
  square const sq_departure = curr_generation->departure;
  numvec const dir_forward = side==White ? dir_up : dir_down;

  curr_generation->arrival = sq_departure+dir_forward;

  if (is_square_empty(curr_generation->arrival))
  {
    if (dir_battery!=0
        || curr_generation->arrival+dir_forward+dir_left == sq_king
        || curr_generation->arrival+dir_forward+dir_right == sq_king
        || (ForwardPromSq(side,curr_generation->arrival)
            && (CheckDir(Queen)[sq_king-curr_generation->arrival]
                || CheckDir(Knight)[sq_king-curr_generation->arrival])))
      push_move_no_capture();

    {
      SquareFlags const double_step = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
      if (TSTFLAG(sq_spec(sq_departure),double_step))
      {
        curr_generation->arrival += dir_forward;
        if (is_square_empty(curr_generation->arrival)
            && (dir_battery!=0
                || curr_generation->arrival+dir_forward+dir_left==sq_king
                || curr_generation->arrival+dir_forward+dir_right==sq_king))
          push_special_move(pawn_multistep);
      }
    }
  }
}

static void pawn_capture(Side side, numvec dir_battery, square sq_king, numvec leftright)
{
  numvec const dir_forward = side==White ? dir_up : dir_down;
  curr_generation->arrival = curr_generation->departure+dir_forward+leftright;

  if (!is_square_empty(curr_generation->arrival)
      && piece_belongs_to_opponent(curr_generation->arrival))
    if (dir_battery!=0
        || curr_generation->arrival+dir_forward+dir_left == sq_king
        || curr_generation->arrival+dir_forward+dir_right == sq_king
        || (ForwardPromSq(side,curr_generation->arrival)
            && (CheckDir(Queen)[sq_king-curr_generation->arrival]
                || CheckDir(Knight)[sq_king-curr_generation->arrival])))
      push_move_regular_capture();
}

static void pawn(square sq_king, Side side)
{
  SquareFlags const base_square = side==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec(curr_generation->departure),base_square))
  {
    numvec const abs_dir_battery = detect_battery(sq_king,side);

    pawn_ep(side);

    if (abs_dir_battery!=dir_up)
      pawn_no_capture(abs_dir_battery,sq_king,side);

    pawn_capture(side,abs_dir_battery,sq_king,dir_left);
    pawn_capture(side,abs_dir_battery,sq_king,dir_right);
  }
}

static void king_neutral(Side side)
{
  vec_index_type vec_index;
  for (vec_index = vec_queen_start; vec_index<=vec_queen_end; ++vec_index)
  {
    curr_generation->arrival = curr_generation->departure+vec[vec_index];
    /* must capture to mate the opponent */
    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

static void king_nonneutral(square sq_king, Side side)
{
  numvec const abs_dir_battery = detect_battery(sq_king,side);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (abs_dir_battery>0)
  {
    vec_index_type vec_index;
    for (vec_index = vec_queen_start; vec_index<=vec_queen_end; ++vec_index)
    {
      numvec const dir = vec[vec_index];
      TraceValue("%d",dir);
      TraceEOL();
      if (abs(dir)!=abs_dir_battery)
      {
        curr_generation->arrival = curr_generation->departure+dir;
        if (move_diff_code[abs(sq_king-curr_generation->arrival)]>1+1)
        {
          if (is_square_empty(curr_generation->arrival))
            push_move_no_capture();
          else if (piece_belongs_to_opponent(curr_generation->arrival))
            push_move_regular_capture();
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void king(square sq_king, Side side)
{
  Flags const mask = BIT(White)|BIT(Black)|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (TSTFULLFLAGMASK(being_solved.spec[sq_king],mask))
    king_neutral(side);
  else
    king_nonneutral(sq_king,side);

  generate_castling();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void knight(square sq_king, Side side)
{
  square const sq_departure = curr_generation->departure;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  {
    numvec const abs_dir_battery = detect_battery(sq_king,side);
    numvec vec_to_king = abs(sq_king-sq_departure);

    if (abs_dir_battery!=0
        || (SquareCol(sq_departure)==SquareCol(sq_king)
            && move_diff_code[vec_to_king]<=move_diff_code[square_a3-square_e1]
            && move_diff_code[vec_to_king]!=move_diff_code[square_a3-square_c1]))
    {
      vec_index_type vec_index;
      for (vec_index = vec_knight_start; vec_index<=vec_knight_end; ++vec_index)
      {
        curr_generation->arrival = sq_departure+vec[vec_index];
        if (abs_dir_battery!=0 || CheckDir(Knight)[curr_generation->arrival-sq_king]!=0)
        {
          if (is_square_empty(curr_generation->arrival))
            push_move_no_capture();
          else if (piece_belongs_to_opponent(curr_generation->arrival))
            push_move_regular_capture();
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider_try_moving_to(square sq_king, numvec dir_to_king)
{
  square sq_target;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceValue("%d",dir_to_king);
  TraceFunctionParamListEnd();

  sq_target = find_end_of_line(curr_generation->arrival,dir_to_king);
  if (sq_target==sq_king)
    push_move_no_capture();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rider_try_capturing_on(square sq_king, numvec dir_to_king)
{
  square sq_target;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceValue("%d",dir_to_king);
  TraceFunctionParamListEnd();

  sq_target = find_end_of_line(curr_generation->arrival,dir_to_king);
  if (sq_target==sq_king)
    push_move_regular_capture();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void queen_try_moving_to(square sq_king)
{
  numvec const dir_to_king = CheckDir(Queen)[sq_king-curr_generation->arrival];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (dir_to_king!=0)
    rider_try_moving_to(sq_king,dir_to_king);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void queen_try_capturing_on(square sq_king)
{
  numvec const dir_to_king = CheckDir(Queen)[sq_king-curr_generation->arrival];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (dir_to_king!=0)
    rider_try_capturing_on(sq_king,dir_to_king);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void queen(square sq_king, Side side)
{
  vec_index_type vec_index;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  for (vec_index = vec_queen_start; vec_index<=vec_queen_end; ++vec_index)
  {
    numvec const dir = vec[vec_index];

    for (curr_generation->arrival = curr_generation->departure+dir;
         is_square_empty(curr_generation->arrival);
         curr_generation->arrival += dir)
      queen_try_moving_to(sq_king);

    if (piece_belongs_to_opponent(curr_generation->arrival))
      queen_try_capturing_on(sq_king);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void simple_rider_fire_battery(Side side,
                                      vec_index_type index_start,
                                      vec_index_type index_end)
{
  vec_index_type vec_index;
  for (vec_index = index_start; vec_index<=index_end; ++vec_index)
  {
    numvec const dir = vec[vec_index];

    for (curr_generation->arrival = curr_generation->departure+dir;
         is_square_empty(curr_generation->arrival);
         curr_generation->arrival += dir)
      push_move_no_capture();

    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

static void simple_rider_directly_approach_king(square sq_king,
                                                Side side,
                                                numvec dir_to_king)
{
  curr_generation->arrival = find_end_of_line(curr_generation->departure,dir_to_king);
  if (piece_belongs_to_opponent(curr_generation->arrival))
    rider_try_capturing_on(sq_king,dir_to_king);
}

static void simple_rider_indirectly_approach_king(square sq_king,
                                                  Side side,
                                                  vec_index_type index_start,
                                                  vec_index_type index_end,
                                                  piece_walk_type rider_walk)
{
  square const sq_departure = curr_generation->departure;
  move_diff_type const OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];
  vec_index_type vec_index;
  for (vec_index = index_start; vec_index<=index_end; ++vec_index)
  {
    numvec const dir = vec[vec_index];
    curr_generation->arrival = sq_departure+dir;
    if (!is_square_blocked(curr_generation->arrival)
        && move_diff_code[abs(curr_generation->arrival-sq_king)]<OriginalDistance)
    {
      /* The rider must move closer to the king! */
      numvec dir_to_king = CheckDir(rider_walk)[sq_king-curr_generation->arrival];
      while (dir_to_king==0 && is_square_empty(curr_generation->arrival))
      {
        curr_generation->arrival += dir;
        dir_to_king = CheckDir(rider_walk)[sq_king-curr_generation->arrival];
      }

      /* We are at the end of the line or in checking distance */
      if (dir_to_king!=0)
      {
        if (is_square_empty(curr_generation->arrival))
          rider_try_moving_to(sq_king,dir_to_king);
        else if (piece_belongs_to_opponent(curr_generation->arrival))
          rider_try_capturing_on(sq_king,dir_to_king);
      }
    }
  }
}

static void rook(square sq_king, Side side)
{
  numvec const dir_battery = detect_directed_battery(sq_king,side,Bishop);

  if (dir_battery!=0)
    simple_rider_fire_battery(side,vec_rook_start,vec_rook_end);
  else
  {
    numvec const dir_to_king = CheckDir(Rook)[sq_king-curr_generation->departure];
    if (dir_to_king==0)
      simple_rider_indirectly_approach_king(sq_king,side,
                                            vec_rook_start,vec_rook_end,
                                            Rook);
    else
      simple_rider_directly_approach_king(sq_king,side,dir_to_king);
  }
}

static void bishop(square sq_king, Side side)
{
  square const sq_departure = curr_generation->departure;
  numvec const dir_battery = detect_directed_battery(sq_king,side,Rook);

  if (dir_battery!=0)
    simple_rider_fire_battery(side,vec_bishop_start,vec_bishop_end);
  else if (SquareCol(sq_departure)==SquareCol(sq_king))
  {
    numvec const dir_to_king = CheckDir(Bishop)[sq_king-sq_departure];
    if (dir_to_king==0)
      simple_rider_indirectly_approach_king(sq_king,side,
                                            vec_bishop_start,vec_bishop_end,
                                            Bishop);
    else
      simple_rider_directly_approach_king(sq_king,side,dir_to_king);
  }
}

static void generate_move_reaching_goal(void)
{
  square square_a = square_a1;
  Side const side_at_move = trait[nbply];
  square const OpponentsKing = side_at_move==White ? being_solved.king_square[Black] : being_solved.king_square[White];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (OpponentsKing!=initsquare)
  {
    /* Don't try to "optimize" by hand. The double-loop is tested as
     * the fastest way to compute (due to compiler-optimizations!) */
    int i;
    for (i = nr_rows_on_board; i>0; i--, square_a += onerow)
    {
      int j;
      curr_generation->departure = square_a;
      for (j = nr_files_on_board; j>0; j--, curr_generation->departure += dir_right)
      {
        piece_walk_type const p = get_walk_of_piece_on_square(curr_generation->departure);
        if (p!=Empty && TSTFLAG(being_solved.spec[curr_generation->departure],side_at_move))
        {
          if (CondFlag[gridchess]
              && !GridLegal(curr_generation->departure,OpponentsKing))
          {
            move_generation_current_walk = p;
            generate_moves_for_piece_based_on_walk();
          }
          else
            switch (p)
            {
              case King:
                king(OpponentsKing,side_at_move);
                break;

              case Pawn:
                pawn(OpponentsKing,side_at_move);
                break;

              case Knight:
                knight(OpponentsKing,side_at_move);
                break;

              case Rook:
                rook(OpponentsKing,side_at_move);
                break;

              case Queen:
                queen(OpponentsKing,side_at_move);
                break;

              case Bishop:
                bishop(OpponentsKing,side_at_move);
                break;

              default:
                /* avoid compiler warning */
                break;
            }
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void orthodox_mating_move_generator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining==slack_length+1);

  nextply(SLICE_STARTER(si));
  generate_move_reaching_goal();
  pipe_solve_delegate(si);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
