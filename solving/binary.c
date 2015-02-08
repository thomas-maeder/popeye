#include "solving/binary.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Solve one of the two legs of a binary slice
 * @param si identifies the binary slice
 * @param condition if true, solve next2, otherwise next1
 */
void binary_solve_if_then_else(slice_index si, boolean condition)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  if (condition)
    fork_solve_delegate(si);
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
