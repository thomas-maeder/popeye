#include "stipulation/series_play/fork.h"
#include "pybrafrk.h"
#include "stipulation/series_play/play.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_fork_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  if (slices[to_goal].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,to_goal);
    to_goal = proxy;
  }

  result = alloc_branch_fork(STSeriesFork,length,min_length,to_goal);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_fork_insert_root(slice_index si, slice_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.pipe.next,st);
  slices[*root].u.shortcut.short_sols = si;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean series_fork_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    result = slice_solve(to_goal);
  else
    result = series_solve_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean series_fork_has_solution_in_n(slice_index si,
                                             stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    result = slice_has_solution(to_goal)==has_solution;
  else
    result = series_has_solution_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void series_fork_solve_threats_in_n(table threats,
                                    slice_index si,
                                    stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    slice_solve_threats(threats,to_goal);
  else
    series_solve_threats_in_n(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 */
void series_fork_make_setplay_slice(slice_index si, slice_traversal *st)
{
  setplay_slice_production * const prod = st->param;
  slice_index const proxy_to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);
  prod->setplay_slice = slices[proxy_to_goal].u.pipe.next;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
