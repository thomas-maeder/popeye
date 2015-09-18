#include "options/interruption.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void option_interruption_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).flag_handler.value = false;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void option_interruption_remember(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).flag_handler.value = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @return true iff solving has been interrupted
 */
boolean option_interruption_is_set(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = SLICE_U(si).flag_handler.value;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
