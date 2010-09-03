#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "pymovein.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/move_to_goal.h"
#include "stipulation/series_play/shortcut.h"
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

/* Allocate a series branch that leads to a branch which represents a
 * single move by the same side that is to reach the goal 
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
static slice_index alloc_series_to_goal(stip_length_type length,
                                        stip_length_type min_length,
                                        slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_proxy_slice();

  {
    slice_index const played1 = alloc_branch(STSeriesMovePlayed,
                                             length,min_length);
    slice_index const checked1 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length,min_length);
    slice_index const dealt1 = alloc_branch(STSeriesMoveDealtWith,
                                            length,min_length);
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           length,min_length);
    slice_index const fork = alloc_series_fork_slice(length,min_length,
                                                     proxy_to_goal);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const played2 = alloc_branch(STSeriesMovePlayed,
                                             length,min_length);
    slice_index const checked2 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length,min_length);
    slice_index const dealt2 = alloc_branch(STSeriesMoveDealtWith,
                                            length,min_length);
    slice_index const inverter = alloc_move_inverter_series_filter();

    shorten_series_pipe(played1);
    shorten_series_pipe(checked1);
    shorten_series_pipe(dealt1);
    shorten_series_pipe(played2);
    shorten_series_pipe(checked2);
    shorten_series_pipe(dealt2);
    shorten_series_pipe(inverter);

    pipe_link(result,fork);
    pipe_link(fork,ready);
    pipe_link(ready,move);
    pipe_link(move,played2);
    pipe_link(played2,checked2);
    pipe_link(checked2,dealt2);
    pipe_link(dealt2,inverter);
    pipe_link(inverter,played1);
    pipe_link(played1,checked1);
    pipe_link(checked1,dealt1);
    pipe_link(dealt1,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a series branch where that leads to some nested branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies proxy slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
static slice_index alloc_series_to_nested(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index proxy_to_next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_next);
  TraceFunctionParamListEnd();

  result = alloc_proxy_slice();

  {
    slice_index const played1 = alloc_branch(STSeriesMovePlayed,
                                             length,min_length);
    slice_index const checked1 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length,min_length);
    slice_index const dealt1 = alloc_branch(STSeriesMoveDealtWith,
                                            length,min_length);
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           length,min_length);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const played = alloc_branch(STSeriesMovePlayed,
                                            length,min_length);
    slice_index const checked2 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length,min_length);
    slice_index const dealt2 = alloc_branch(STSeriesMoveDealtWith,
                                            length,min_length);
    slice_index const fork = alloc_series_fork_slice(length,min_length,
                                                     proxy_to_next);
    slice_index const inverter = alloc_move_inverter_series_filter();

    shorten_series_pipe(played1);
    shorten_series_pipe(checked1);
    shorten_series_pipe(dealt1);
    shorten_series_pipe(played);
    shorten_series_pipe(checked2);
    shorten_series_pipe(dealt2);
    shorten_series_pipe(fork);

    pipe_link(result,ready);
    pipe_link(ready,move);
    pipe_link(move,played);
    pipe_link(played,checked2);
    pipe_link(checked2,dealt2);
    pipe_link(dealt2,fork);
    pipe_link(fork,inverter);
    pipe_link(inverter,played1);
    pipe_link(played1,checked1);
    pipe_link(checked1,dealt1);
    pipe_link(dealt1,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a series branch where the next slice's starter is the
 * opponent of the series's starter.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies proxy slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
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

  assert(slices[proxy_to_next].type==STProxy);
  assert(to_next!=no_slice);
  assert(length>=slack_length_series+1);
  assert(min_length>=slack_length_series+1);

  if (slices[to_next].type==STGoalReachedTester)
  {
    /* last move is represented by a STSeriesMoveToGoal slice */
    Goal const goal = slices[to_next].u.goal_reached_tester.goal;
    pipe_append(proxy_to_next,alloc_series_move_to_goal_slice(goal));
    --length;
    --min_length;

    if (length==slack_length_series)
      /* that move is the only move */
      result = proxy_to_next;
    else
      /* allocate a branch for the other moves */
      result = alloc_series_to_goal(length,min_length,proxy_to_next);
  }
  else
    result = alloc_series_to_nested(length,min_length,proxy_to_next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
