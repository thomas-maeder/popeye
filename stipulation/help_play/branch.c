#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/fork.h"
#include "pypipe.h"
#include "pyselfcg.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>

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
    slice_index const
        guard1 = alloc_selfcheck_guard_help_filter(length,min_length);
    slice_index const move1 = alloc_help_move_slice(length,min_length);
    slice_index const proxy = alloc_proxy_slice();
    slice_index const
        guard2 = alloc_selfcheck_guard_help_filter(length-1,min_length-1);
    slice_index const move2 = alloc_help_move_slice(length-1,min_length-1);

    result = alloc_help_fork_slice(length,min_length,proxy_to_goal);

    pipe_link(result,guard1);
    pipe_link(guard1,move1);
    pipe_link(move1,proxy);
    pipe_link(proxy,guard2);
    pipe_link(guard2,move2);
    pipe_link(move2,result);
  }
  else
  {
    slice_index const fork = alloc_help_fork_slice(length,min_length,
                                                   proxy_to_goal);
    slice_index const guard1 = alloc_selfcheck_guard_help_filter(length,
                                                                 min_length);
    slice_index const move1 = alloc_help_move_slice(length,min_length);
    slice_index const guard2 = alloc_selfcheck_guard_help_filter(length-1,
                                                                 min_length-1);
    slice_index const move2 = alloc_help_move_slice(length-1,min_length-1);

    result = alloc_proxy_slice();

    pipe_link(result,guard1);
    pipe_link(guard1,move1);
    pipe_link(move1,fork);
    pipe_link(fork,guard2);
    pipe_link(guard2,move2);
    pipe_link(move2,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a help slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void help_branch_shorten_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.branch.length -= 2;
  if (slices[si].u.branch.min_length-slack_length_help>=2)
    slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help branch whose entry slice is a selfcheck guard
 * @param si identifies entry slice
 * @return entry slice of shortened branch
 *         no_slice if shortening isn't applicable
 */
static slice_index shorten_guard(slice_index si)
{
  slice_index result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const move = slices[si].u.pipe.next;
    slice_index const fork = slices[move].u.pipe.next;

    assert(slices[si].type==STSelfCheckGuardHelpFilter);
    assert(slices[move].type==STHelpMove);
    assert(slices[fork].type==STHelpFork);
    
    result = fork;

    help_branch_shorten_slice(si);
    help_branch_shorten_slice(move);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a help branch whose entry slice is a STHelpFork
 * @param si identifies entry slice
 * @return entry slice of shortened branch
 *         no_slice if shortening isn't applicable
 */

static slice_index shorten_fork(slice_index si)
{
  slice_index result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const guard1 = slices[si].u.pipe.next;
    slice_index const move1 = slices[guard1].u.pipe.next;
    slice_index const proxy = slices[move1].u.pipe.next;
    slice_index const guard2 = slices[proxy].u.pipe.next;

    assert(slices[si].type==STHelpFork);
    assert(slices[guard1].type==STSelfCheckGuardHelpFilter);
    assert(slices[move1].type==STHelpMove);
    assert(slices[proxy].type==STProxy);
    assert(slices[guard2].type==STSelfCheckGuardHelpFilter);
    
    result = guard2;

    help_branch_shorten_slice(si);
    help_branch_shorten_slice(guard1);
    help_branch_shorten_slice(move1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 * @return entry slice of shortened branch
 *         no_slice if shortening isn't applicable
 */
slice_index help_branch_shorten(slice_index si)
{
  slice_index result;
  SliceType const type = slices[si].type;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (type)
  {
    case STHelpFork:
      result = shorten_fork(si);
      break;

    case STSelfCheckGuardHelpFilter:
      result = shorten_guard(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
