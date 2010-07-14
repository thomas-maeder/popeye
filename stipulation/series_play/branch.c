#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "pyselfcg.h"
#include "pymovein.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/move_to_goal.h"
#include "stipulation/series_play/shortcut.h"
#include "stipulation/series_play/or.h"
#include "stipulation/series_play/not_last_move.h"
#include "stipulation/series_play/only_last_move.h"
#include "trace.h"

#include <assert.h>

/* Shorten a series pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
void shorten_series_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.branch.length;
  if (slices[pipe].u.branch.min_length>slack_length_series)
    --slices[pipe].u.branch.min_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a series branch where the next slice's starter is the
 * opponent of the series's starter.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
                                stip_length_type min_length,
                                slice_index proxy_to_goal)
{
  slice_index result;
  slice_index const to_goal = slices[proxy_to_goal].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);
  assert(to_goal!=no_slice);
  assert(length>=slack_length_series+1);

  if (slices[to_goal].type==STGoalReachedTester)
  {
    Goal const goal = slices[to_goal].u.goal_reached_tester.goal;
    slice_index const move_to_goal = alloc_series_move_to_goal_slice(goal);

    pipe_append(proxy_to_goal,move_to_goal);

    if (length==slack_length_series+1)
      result = proxy_to_goal;
    else
    {
      slice_index const only_to_goal = alloc_series_only_last_move_slice();
      slice_index const
          guard1 = alloc_selfcheck_guard_series_filter(length,min_length);
      slice_index const or = alloc_series_OR_slice(length,min_length,
                                                   proxy_to_goal);
      slice_index const not_last = alloc_series_not_last_move_slice();
      slice_index const move = alloc_series_move_slice(length,min_length);
      slice_index const
          guard2 = alloc_selfcheck_guard_series_filter(length,min_length);
      slice_index const inverter = alloc_move_inverter_series_filter();

      result = alloc_proxy_slice();

      pipe_append(proxy_to_goal,only_to_goal);

      shorten_series_pipe(guard1);
      shorten_series_pipe(guard2);

      pipe_link(result,or);
      pipe_link(or,not_last);
      pipe_link(not_last,move);
      pipe_link(move,guard2);
      pipe_link(guard2,inverter);
      pipe_link(inverter,guard1);
      pipe_link(guard1,result);
    }
  }
  else
  {
    slice_index const
        guard1 = alloc_selfcheck_guard_series_filter(length,min_length);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const
        guard2 = alloc_selfcheck_guard_series_filter(length,min_length);
    slice_index const fork = alloc_series_fork_slice(length,min_length,
                                                     proxy_to_goal);
    slice_index const inverter = alloc_move_inverter_series_filter();

    assert(length>slack_length_series);

    result = alloc_proxy_slice();

    shorten_series_pipe(guard1);
    shorten_series_pipe(guard2);
    shorten_series_pipe(fork);

    pipe_link(result,move);
    pipe_link(move,guard2);
    pipe_link(guard2,fork);
    pipe_link(fork,inverter);
    pipe_link(inverter,guard1);
    pipe_link(guard1,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
