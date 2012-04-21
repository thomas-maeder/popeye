#include "pybrafrk.h"
#include "pystip.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "solving/solving.h"
#include "debugging/trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param fork identifies proxy slice that leads towards goal
 *                from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(slice_type type, slice_index fork)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].next2 = fork;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void branch_fork_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (slices[si].next2!=no_slice)
  {
    stip_traverse_structure_next_branch(si,st);
    proxy_slice_resolve(&slices[si].next2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void branch_fork_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const fork = slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_next_branch(si,st);

  if (slices[si].starter==no_side)
  {
    if (slices[fork].starter==no_side)
    {
      stip_traverse_structure_children_pipe(si,st);
      slices[si].starter = slices[slices[si].next1].starter;
    }
    else
      slices[si].starter = slices[fork].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a fork slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_fork(slice_index si, stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    state->spun_off[si] = copy_slice(si);
    stip_traverse_structure_children(si,st);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].next1]);
    slices[state->spun_off[si]].next2 = state->spun_off[slices[si].next2];
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
