#include "stipulation/series_play/root.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "trace.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/shortcut.h"

#include <assert.h>


/* Allocate a STSeriesRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param next identifies following branch silice
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_root_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index next,
                                    slice_index short_sols)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",short_sols);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesRoot,length,min_length);

  {
    slice_index const shortcut = alloc_series_shortcut(length,min_length,
                                                       short_sols);
    pipe_link(result,shortcut);
    pipe_link(shortcut,next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the moves beyond a series root slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_root(slice_index si, stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);
  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_root_solve(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.shortcut.min_length>=slack_length_series);

  /* Do *not* delegate to series_solve() here:
   * If series_solve() has found solutions of a certain length, it won't
   * look for longer solutions.
   * Here, on the other hand, we want to find solutions of any length.
   */
  while (len<=full_length)
  {
    if (series_solve_in_n(next,len)==len)
      result = has_solution;
    ++len;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
