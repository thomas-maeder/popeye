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
    slice_index const fork = alloc_series_fork_slice(length,min_length,
                                                     proxy_to_goal);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const played1 = alloc_branch(STSeriesMovePlayed,
                                             length-1,min_length-1);
    slice_index const checked1 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length-1,min_length-1);
    slice_index const dealt1 = alloc_branch(STSeriesMoveDealtWith,
                                            length-1,min_length-1);
    slice_index const inverter = alloc_move_inverter_series_filter();
    slice_index const played2 = alloc_branch(STSeriesMovePlayed,
                                             length-1,min_length-1);
    slice_index const checked2 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length-1,min_length-1);
    slice_index const dealt2 = alloc_branch(STSeriesMoveDealtWith,
                                            length-1,min_length-1);
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           length-1,min_length-1);

    pipe_link(fork,ready);
    pipe_link(ready,move);
    pipe_link(move,played1);
    pipe_link(played1,checked1);
    pipe_link(checked1,dealt1);
    pipe_link(dealt1,inverter);
    pipe_link(inverter,played2);
    pipe_link(played2,checked2);
    pipe_link(checked2,dealt2);
    pipe_link(dealt2,result);
    pipe_link(result,fork);
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
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           length,min_length);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const played1 = alloc_branch(STSeriesMovePlayed,
                                             length-1,min_length-1);
    slice_index const checked1 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length-1,min_length-1);
    slice_index const dealt1 = alloc_branch(STSeriesMoveDealtWith,
                                            length-1,min_length-1);
    slice_index const fork = alloc_series_fork_slice(length-1,min_length-1,
                                                     proxy_to_next);
    slice_index const inverter = alloc_move_inverter_series_filter();
    slice_index const played2 = alloc_branch(STSeriesMovePlayed,
                                             length-1,min_length-1);
    slice_index const checked2 = alloc_branch(STSeriesMoveLegalityChecked,
                                              length-1,min_length-1);
    slice_index const dealt2 = alloc_branch(STSeriesMoveDealtWith,
                                            length-1,min_length-1);

    pipe_link(ready,move);
    pipe_link(move,played1);
    pipe_link(played1,checked1);
    pipe_link(checked1,dealt1);
    pipe_link(dealt1,fork);
    pipe_link(fork,inverter);
    pipe_link(inverter,played2);
    pipe_link(played2,checked2);
    pipe_link(checked2,dealt2);
    pipe_link(dealt2,result);
    pipe_link(result,ready);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a the appropriate proxy slices before each
 * STGoalReachedTester slice
 * @param si identifies STGoalReachedTester slice
 * @param st address of structure representing the traversal
 */
static void instrument_tester(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    Goal const goal = slices[si].u.goal_reached_tester.goal;
    slice_index const ready = alloc_branch(STReadyForSeriesMove,
                                           slack_length_series+1,
                                           slack_length_series+1);
    slice_index const move_to_goal = alloc_series_move_to_goal_slice(goal);
    slice_index const played = alloc_branch(STSeriesMovePlayed,
                                            slack_length_series,
                                            slack_length_series);
    slice_index const checked = alloc_branch(STSeriesMoveLegalityChecked,
                                             slack_length_series,
                                             slack_length_series);
    slice_index const dealt = alloc_branch(STSeriesMoveDealtWith,
                                           slack_length_series,
                                           slack_length_series);
    pipe_append(slices[si].prev,ready);
    pipe_append(ready,move_to_goal);
    pipe_append(move_to_goal,played);

    pipe_append(si,checked);
    pipe_append(checked,dealt);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    instrument_tester(to_next);

    if (length==slack_length_series+1)
      /* that move is the only move */
      result = proxy_to_next;
    else
      /* allocate a branch for the other moves */
      result = alloc_series_to_goal(length-1,min_length-1,proxy_to_next);
  }
  else
    result = alloc_series_to_nested(length,min_length,proxy_to_next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
