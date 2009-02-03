#include "pyconst.h"
#include "trace.h"

/* Allocate a slice with constant value.
 * @param value constant value
 * @return index of allocated slice
 */
slice_index alloc_constant_slice(boolean value)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",value);

  result = alloc_slice_index();
  slices[result].type = STConstant;
  slices[result].u.constant.value = value;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean constant_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slices[si].u.constant.value;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
