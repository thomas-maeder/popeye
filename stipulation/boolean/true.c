#include "stipulation/boolean/true.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */

/* This module provides functionality dealing with leaf slices
 */

/* Allocate a STTrue slice.
 * @return index of allocated slice
 */
slice_index alloc_true_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = create_slice(STTrue);
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

/* Spin a copy off a pipe to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void leaf_spin_off_copy(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->spun_off[si] = copy_slice(si);
  if (state->spun_off[si]==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(2); /* TODO: Do we have to exit here? */
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a leaf slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_leaf(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_TESTER(si) = copy_slice(si);
  if (SLICE_TESTER(si)==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(2); /* TODO: Do we have to exit here? */
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
