#include "stipulation/boolean/not.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a not slice.
 * @return index of allocated slice
 */
slice_index alloc_not_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STNot);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
