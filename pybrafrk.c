#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @param proxy_to_goal identifies proxy slice that leads towards goal
 *                      from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(SliceType type,
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

  assert(slices[proxy_to_goal].type==STProxy);

  result = alloc_branch(type,length,min_length);
  slices[result].u.branch_fork.towards_goal = proxy_to_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void branch_fork_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (slices[si].u.branch_fork.towards_goal!=no_slice)
    proxy_slice_resolve(&slices[si].u.branch_fork.towards_goal);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type branch_fork_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void branch_fork_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    stip_traverse_structure(towards_goal,st);
    if (slices[towards_goal].starter==no_side)
    {
      slice_index const next = slices[si].u.pipe.next;
      stip_traverse_structure(next,st);
      slices[si].starter = slices[next].starter;
    }
    else
      slices[si].starter = slices[towards_goal].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void branch_fork_impose_starter(slice_index si, stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
