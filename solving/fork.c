#include "solving/fork.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Delegate solving to next2
 * @param si identifies the fork
 */
void fork_solve_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate testing observation to next2
 * @param si identifies the fork
 */
void fork_is_square_observed_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  is_square_observed_recursive(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate testing observation to next2
 * @param si identifies the fork
 * @return true iff the target square is observed
 */
boolean fork_is_square_observed_nested_delegate(slice_index si,
                                                validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = is_square_observed_nested(SLICE_NEXT2(si),evaluate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Delegate generating to next2
 * @param si identifies the fork
 */
void fork_move_generation_delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  generate_moves_delegate(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Delegate validating to next2
 * @param si identifies the fork
 * @return true iff the observation is valid
 */
boolean fork_validate_observation_recursive_delegate(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result =  validate_observation_recursive(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
