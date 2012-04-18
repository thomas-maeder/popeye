#include "stipulation/boolean/false.h"
#include "pystip.h"
#include "debugging/trace.h"

/* Allocate a STFalse slice.
 * @return index of allocated slice
 */
slice_index alloc_false_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = create_slice(STFalse);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
