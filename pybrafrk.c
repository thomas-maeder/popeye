#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param fork identifies proxy slice that leads towards goal
 *                from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(SliceType type, slice_index fork)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.fork.fork = fork;

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

  if (slices[si].u.fork.fork!=no_slice)
  {
    stip_traverse_structure(slices[si].u.fork.fork,st);
    proxy_slice_resolve(&slices[si].u.fork.fork,st);
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
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(fork,st);

  if (slices[si].starter==no_side)
  {
    if (slices[fork].starter==no_side)
    {
      stip_traverse_structure_pipe(si,st);
      slices[si].starter = slices[slices[si].u.pipe.next].starter;
    }
    else
      slices[si].starter = slices[fork].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
