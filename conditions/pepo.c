#include "conditions/pepo.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "solving/king_capture_avoider.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"

#include "debugging/assert.h"
#include "debugging/trace.h"

typedef enum
{
  not_testing_check,
  no_check_found,
  one_check_found
} check_test_status_type;

static check_test_status_type check_test_status;

/* Determine whether a square is observed be the side at the move according to
 * Pepo
 * @param si identifies next slice
 * @note sets observation_result
 */
void pepo_is_square_observed(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",check_test_status);TraceEOL();

  switch (check_test_status)
  {
    case not_testing_check:
      pipe_is_square_observed_delegate(si);
      break;

    case no_check_found:
      pipe_is_square_observed_delegate(si);

      if (observation_result)
      {
        square const pos_first_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
        piece_walk_type const save_first_observer_walk = being_solved.board[pos_first_observer];

        TraceSquare(pos_first_observer);
        TraceEOL();

        check_test_status = one_check_found;
        observation_result = false;

        being_solved.board[pos_first_observer] = Dummy;
        pipe_is_square_observed_delegate(si);
        being_solved.board[pos_first_observer] = save_first_observer_walk;

        TraceValue("%u",observation_result);
        TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
        TraceEOL();
      }
      break;

    case one_check_found:
      pipe_is_square_observed_delegate(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean pepo_check_test_initialiser_is_in_check(slice_index si,
                                                Side side_in_check)
{
  boolean result;
  Side const side_checking = advers(side_in_check);
  square const save_king_square = being_solved.king_square[side_checking];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_checking);
  TraceSquare(save_king_square);
  TraceEOL();

  assert(check_test_status==not_testing_check);
  check_test_status = no_check_found;

  if (save_king_square==initsquare)
    result = pipe_is_in_check_recursive_delegate(si,side_in_check);
  else
  {
    piece_walk_type const save_king_walk = get_walk_of_piece_on_square(save_king_square);
    being_solved.board[save_king_square] = Dummy;
    result = pipe_is_in_check_recursive_delegate(si,side_in_check);
    being_solved.board[save_king_square] = save_king_walk;
  }

  assert(check_test_status!=not_testing_check);
  check_test_status = not_testing_check;

  TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_paralysed(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  boolean result;
  Side const paralysed_side = trait[nbply];
  Side const paralysing_side = advers(paralysed_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,paralysing_side);
  TraceSquare(sq_departure);
  TraceEOL();

  if (TSTFLAG(being_solved.spec[sq_departure],Royal))
  {
    siblingply(paralysing_side);
    push_observation_target(sq_departure);
    result = fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed[paralysed_side],
                                                     EVALUATE(observation_geometry));
    finply();
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observater according to Pepo
 * @return true iff the observation is valid
 */
boolean pepo_validate_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!is_paralysed(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param si identifies generator slice
 */
void pepo_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!is_paralysed(current_generation))
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Pepo
 * @param si identifies root slice of solving machinery
 */
void pepo_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_for_piece_generation(si,nr_sides,STPepoMovesForPieceGenerator);

  solving_instrument_check_testing(si,STPepoCheckTestInitialiser);
  stip_instrument_is_square_observed_testing(si,nr_sides,STPepoCheckTestHack);

  solving_insert_king_capture_avoiders(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
