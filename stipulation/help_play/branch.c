#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/fork.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>

/* Shorten a help pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
void shorten_help_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.branch.length;
  --slices[pipe].u.branch.min_length;
  if (slices[pipe].u.branch.min_length<slack_length_help)
    slices[pipe].u.branch.min_length += 2;
  slices[pipe].starter = (slices[pipe].starter==no_side
                          ? no_side
                          : advers(slices[pipe].starter));
  TraceValue("%u",slices[pipe].starter);
  TraceValue("%u",slices[pipe].u.branch.length);
  TraceValue("%u\n",slices[pipe].u.branch.min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of initial slice of allocated help branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length,
                              slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);
  assert(slices[proxy_to_goal].type==STProxy);

  if ((length-slack_length_help)%2==0)
  {
    slice_index const branch1 = alloc_help_move_slice(length,min_length);
    slice_index const branch2 = alloc_help_move_slice(length,min_length);
    result = alloc_help_fork_slice(length,min_length,proxy_to_goal);

    shorten_help_pipe(branch2);

    pipe_link(result,branch1);
    pipe_link(branch1,branch2);
    pipe_link(branch2,result);
  }
  else
  {
    slice_index const fork = alloc_help_fork_slice(length,min_length,
                                                   proxy_to_goal);
    slice_index const branch1 = alloc_help_move_slice(length,min_length);
    slice_index const branch2 = alloc_help_move_slice(length,min_length);
    result = alloc_proxy_slice();

    shorten_help_pipe(fork);
    shorten_help_pipe(branch1);

    pipe_link(result,branch2);
    pipe_link(branch2,fork);
    pipe_link(fork,branch1);
    pipe_link(branch1,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
