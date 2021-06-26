#include "stipulation/conditional_pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */


/* Allocate a conditional pipe slice.
 * @param type which slice type
 * @param condition entry slice into condition
 * @return index of allocated slice
 */
slice_index alloc_conditional_pipe(slice_type type, slice_index condition)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  SLICE_NEXT2(result) = condition;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a pipe slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_conditional_pipe(slice_index si,
                                            stip_structure_traversal *st)
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
  stip_traverse_structure_children_pipe(si,st);
  link_to_branch(SLICE_TESTER(si),SLICE_TESTER(SLICE_NEXT1(si)));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin a copy off a conditional pipe to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void conditional_pipe_spin_off_copy(slice_index si,
                                    stip_structure_traversal *st)
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

  stip_traverse_structure_children_pipe(si,st);

  if (state->spun_off[SLICE_NEXT1(si)]==no_slice)
  {
    dealloc_slice(state->spun_off[si]);
    state->spun_off[si] = no_slice;
  }
  else
    link_to_branch(state->spun_off[si],state->spun_off[SLICE_NEXT1(si)]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
