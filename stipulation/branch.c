#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "pypipe.h"
#include "trace.h"

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @param proxy_to_goal identifies proxy slice that leads towards goal
 *                      from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch(SliceType type,
                         stip_length_type length,
                         stip_length_type min_length,
                         slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.pipe.u.help_root.length = length;
  slices[result].u.pipe.u.help_root.min_length = min_length;
  slices[result].u.pipe.u.help_root.towards_goal = proxy_to_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Establish a link between a branch slice and its successor
 * @param branch identifies branch slice
 * @param succ identifies branch to become the successor
 */
void branch_link(slice_index branch, slice_index succ)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",succ);
  TraceFunctionParamListEnd();

  pipe_set_successor(branch,succ);
  slice_set_predecessor(succ,branch);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 * @return true iff slice si has been successfully traversed
 */
boolean branch_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (slices[si].u.pipe.u.branch.towards_goal!=no_slice)
    proxy_slice_resolve(&slices[si].u.pipe.u.branch.towards_goal);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
