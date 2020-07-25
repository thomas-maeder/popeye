#include "stipulation/boolean/false.h"
#include "debugging/trace.h"

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */

/* Allocate a STFalse slice.
 * @return index of allocated slice
 */
slice_index alloc_false_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = create_slice(STFalse);
  if (result==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to create slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(1); /* TODO: Do we have to exit here? */
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
