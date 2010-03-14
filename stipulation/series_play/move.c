#include "stipulation/series_play/move.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "stipulation/series_play/play.h"
#include "trace.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/shortcut.h"

#include <assert.h>

/* Allocate a STSeriesMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_move_slice(stip_length_type length,
                                   stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_move_insert_root(slice_index si, slice_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;

    slice_index const root_branch = copy_slice(si);
    *root = alloc_series_root_slice(length,min_length,root_branch,prev);

    shorten_series_pipe(si);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void series_move_detect_starter(slice_index si, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    slice_traverse_children(si,st);
    slices[si].starter = (slices[next].starter==no_side
                          ? no_side
                          : advers(slices[next].starter));
  }

  TraceValue("%u\n",slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return true iff >= 1 solution was found
 */
boolean series_move_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const next_slice = slices[si].u.pipe.next;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  active_slice[nbply+1] = si;
  genmove(side_at_move);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_solve_in_n(next_slice,n-1))
      result = true;

    repcoup();
  }

  finply();

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
void series_move_solve_threats_in_n(table threats,
                                   slice_index si,
                                   stip_length_type n)
{
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_solve_in_n(slices[si].u.pipe.next,n-1))
    {
      append_to_top_table();
      coupfort();
    }

    repcoup();
  }
    
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean series_move_are_threats_refuted(table threats, slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n = slices[si].u.shortcut.length;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;

    active_slice[nbply+1] = si;
    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats)
          && series_has_solution_in_n(next,n-1))
        ++nr_successful_threats;

      repcoup();
    }

    finply();

    /* this happens if >=1 threat no longer works or some threats can
     * no longer be played after the defense.
     */
    result = nr_successful_threats<table_length(threats);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean series_move_has_solution_in_n(slice_index si, stip_length_type n)
{
  Side const side_at_move = slices[si].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  while (encore() && !result)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_has_solution_in_n(slices[si].u.pipe.next,n-1))
      result = true;

    repcoup();
  }
    
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
