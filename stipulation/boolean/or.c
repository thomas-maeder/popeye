#include "stipulation/boolean/or.h"
#include "stipulation/binary.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STOr slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_or_slice(slice_index op1, slice_index op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STOr,op1,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
