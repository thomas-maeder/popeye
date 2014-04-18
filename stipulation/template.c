#include "stipulation/template.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

enum
{
  STTemplate = 0
};

/* Allocate a STTemplate slice.
 * @return index of allocated slice
 */
slice_index alloc_template_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STTemplate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
