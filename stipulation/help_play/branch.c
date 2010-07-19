#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "pyselfcg.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/move_to_goal.h"
#include "trace.h"

#include <assert.h>

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
    stip_length_type const length = slices[si].u.branch.length;
    slice_index const proxy = slices[si].u.pipe.next;
    slice_index const move = slices[proxy].u.pipe.next;
    slice_index const guard = slices[move].u.pipe.next;
    slice_index const fork = slices[guard].u.pipe.next;

    assert(slices[si].type==STSelfCheckGuardHelpFilter);
    assert(slices[proxy].type==STProxy);
    assert(slices[move].type==STHelpMove);
    assert(slices[guard].type==STSelfCheckGuardHelpFilter);
    assert(slices[fork].type==STHelpFork);
    
    if (length==slack_length_help+1)
    {
      result = slices[fork].u.branch_fork.towards_goal;
      slices[fork].u.branch_fork.towards_goal = no_slice;
      dealloc_slices(si);
    }
    else
    {
      result = fork;

      help_branch_shorten_slice(si);
      help_branch_shorten_slice(move);
      help_branch_shorten_slice(guard);
    }
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
    slice_index const proxy1 = slices[si].u.pipe.next;
    slice_index const move1 = slices[proxy1].u.pipe.next;
    slice_index const proxy2 = slices[move1].u.pipe.next;
    slice_index const guard = slices[proxy2].u.pipe.next;

    assert(slices[si].type==STHelpFork);
    assert(slices[proxy1].type==STProxy);
    assert(slices[move1].type==STHelpMove);
    assert(slices[proxy2].type==STProxy);
    assert(slices[guard].type==STSelfCheckGuardHelpFilter);
    
    result = proxy2;

    help_branch_shorten_slice(si);
    help_branch_shorten_slice(move1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a help branch with an even number of half moves
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of initial slice of allocated help branch
 */
static slice_index alloc_help_branch_even(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  {
    slice_index const
        guard1 = alloc_selfcheck_guard_help_filter(length,min_length);
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const move1 = alloc_help_move_slice(length,min_length);
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const
        guard2 = alloc_selfcheck_guard_help_filter(length+1,min_length+1);
    slice_index const proxy3 = alloc_proxy_slice();
    slice_index const move2 = alloc_help_move_slice(length+1,min_length+1);

    help_branch_shorten_slice(guard2);
    help_branch_shorten_slice(move2);
    help_branch_shorten_slice(guard1);

    result = alloc_help_fork_slice(length,min_length,proxy_to_goal);

    pipe_link(result,proxy1);
    pipe_link(proxy1,move1);
    pipe_link(move1,proxy2);
    pipe_link(proxy2,guard2);
    pipe_link(guard2,proxy3);
    pipe_link(proxy3,move2);
    pipe_link(move2,guard1);
    pipe_link(guard1,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static slice_index alloc_adjusted_help_branch(stip_length_type length,
                                              stip_length_type min_length,
                                              slice_index proxy_to_next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_next);
  TraceFunctionParamListEnd();

  if ((length-slack_length_help)%2==0)
    result = alloc_help_branch_even(length,min_length,proxy_to_next);
  else
  {
    /* this indirect approach avoids some code duplication */
    slice_index const branch = alloc_help_branch_even(length+1,min_length+1,
                                                      proxy_to_next);
    result = shorten_fork(branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length,
                              slice_index proxy_to_next)
{
  slice_index result;
  slice_index const to_next = slices[proxy_to_next].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_next);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);
  assert(slices[proxy_to_next].type==STProxy);

  if (slices[to_next].type==STGoalReachedTester)
  {
    /* last move is represented by a STHelpMoveToGoal slice */
    Goal const goal = slices[to_next].u.goal_reached_tester.goal;
    pipe_append(proxy_to_next,alloc_help_move_to_goal_slice(goal));
    --length;
    --min_length;

    if (length==slack_length_help)
      result = proxy_to_next;
    else
      result = alloc_adjusted_help_branch(length,min_length,proxy_to_next);
  }
  else
    result = alloc_adjusted_help_branch(length,min_length,proxy_to_next);

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
      result = no_slice;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
