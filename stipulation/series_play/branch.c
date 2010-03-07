#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "pymovein.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/move.h"
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

/* Allocate a series branch where the next slice's starter is the
 * opponent of the series's starter.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies proxy slice leading towards goal
 * @return index of adapter slice of allocated series branch
 */
slice_index alloc_series_branch_next_other_starter(stip_length_type length,
                                                   stip_length_type min_length,
                                                   slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_series);

  if (slices[to_goal].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,to_goal);
    to_goal = proxy;
  }

  result = alloc_proxy_slice();

  {
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const inverter = alloc_move_inverter_series_filter();
    slice_index const fork = alloc_series_fork_slice(length,min_length,to_goal);

    shorten_series_pipe(fork);

    pipe_link(result,move);
    pipe_link(move,fork);
    pipe_link(fork,inverter);
    pipe_link(inverter,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a series branch where the next slice has the same starter
 * as the series.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies proxy slice leading towards goal
 * @return index of adapter slice of allocated series branch
 */
slice_index alloc_series_branch_next_same_starter(stip_length_type length,
                                                  stip_length_type min_length,
                                                  slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_series);

  if (slices[to_goal].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,to_goal);
    to_goal = proxy;
  }

  result = alloc_series_fork_slice(length,min_length,to_goal);

  {
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const inverter = alloc_move_inverter_series_filter();

    pipe_link(result,move);
    pipe_link(move,inverter);
    pipe_link(inverter,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
