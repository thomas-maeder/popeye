#include "output/plaintext/end_of_phase_writer.h"
#include "pymsg.h"
#include "pypipe.h"
#include "trace.h"

/* Allocate a STEndOfPhaseWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_phase_writer_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEndOfPhaseWriter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type end_of_phase_writer_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(slices[si].u.pipe.next);
  Message(NewLine);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
