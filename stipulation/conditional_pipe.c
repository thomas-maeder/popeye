#include "stipulation/conditional_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "solving/solving.h"
#include "trace.h"

#include <assert.h>


/* Allocate a conditional pipe slice.
 * @param type which slice type
 * @param condition entry slice into condition
 * @return index of allocated slice
 */
slice_index alloc_conditional_pipe(slice_type type, slice_index condition)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.conditional_pipe.condition = condition;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_conditional_pipe(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  assert(slices[si].u.conditional_pipe.condition!=no_slice);
  stip_traverse_structure(slices[si].u.conditional_pipe.condition,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void conditional_pipe_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  assert(slices[si].u.conditional_pipe.condition!=no_slice);
  proxy_slice_resolve(&slices[si].u.conditional_pipe.condition,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a pipe slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_conditional_pipe(slice_index si,
                                            stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_spin_off_testers_pipe(si,st);

  slices[state->spun_off[si]].u.conditional_pipe.condition = state->spun_off[slices[si].u.conditional_pipe.condition];

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

  stip_traverse_structure_pipe(si,st);

  if (state->spun_off[slices[si].u.pipe.next]==no_slice)
  {
    dealloc_slice(state->spun_off[si]);
    state->spun_off[si] = no_slice;
  }
  else
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.pipe.next]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
