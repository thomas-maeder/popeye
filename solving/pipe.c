#include "solving/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Report illegality under some condition; go on otherwise
 * @param si identifies the pipe slice
 * @param condition if true, report this_move_is_illegal, otherwise solve next1
 */
void pipe_this_move_illegal_if(slice_index si, boolean condition)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  if (condition)
    solve_result = this_move_is_illegal;
  else
    solve(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report success in solvin under some condition; go on otherwise
 * @param si identifies the pipe slice
 * @param condition if true, report that this move solves, otherwise solve next1
 */
void pipe_this_move_solves_if(slice_index si, boolean condition)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  if (condition)
    solve_result = MOVE_HAS_SOLVED_LENGTH();
  else
    solve(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report failure to solve under some condition; go on otherwise
 * @param si identifies the pipe slice
 * @param condition if true, report that this move doesn't solve, otherwise solve next1
 */
void pipe_this_move_doesnt_solve_if(slice_index si, boolean condition)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  if (condition)
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  else
    solve(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report success in solving under some condition and not otherwise
 * @param condition if true, report that this move solves, otherwise that it doesn't
 */
void pipe_this_move_solves_exactly_if(boolean condition)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  solve_result = condition ? MOVE_HAS_SOLVED_LENGTH() : MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate solving to next1
 * @param si identifies the pipe
 */
void pipe_solve_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate testing observation to next1
 * @param si identifies the pipe
 */
void pipe_is_square_observed_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  is_square_observed_recursive(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate generating to next1
 * @param si identifies the pipe
 */
void pipe_move_generation_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  generate_moves_delegate(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate generating with different walk to next1
 * @param si identifies the pipe
 * @param w walk to generate for
 */
void pipe_move_generation_differnt_walk_delegate(slice_index si,
                                                 piece_walk_type w)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceWalk(w);
  TraceFunctionParamListEnd();

  generate_moves_different_walk(SLICE_NEXT1(si),w);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate validating to next1
 * @param si identifies the pipe
 * @return true iff the observation is valid
 */
boolean pipe_validate_observation_recursive_delegate(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Delegate testing to next1
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean pipe_is_in_check_recursive_delegate(slice_index si, Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  result = is_in_check_recursive(SLICE_NEXT1(si),side_in_check);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Dispatch to next1. Intended to be used by pipes that can be used in different
 * contexts.
 * @param si identifies the pipe
 */
void pipe_dispatch_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  dispatch(SLICE_NEXT1(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
