#include "stipulation/series_play/root.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyoutput.h"
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

/* Solve a branch slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_root_root_solve(slice_index root)
{
  boolean result = false;
  stip_length_type const full_length = slices[root].u.branch.length;
  stip_length_type len = slices[root].u.branch.min_length;
  slice_index const next = slices[root].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root].u.shortcut.min_length>=slack_length_series);

  /* Do *not* delegate to series_solve() here:
   * If series_solve() has found solutions of a certain length, it won't
   * look for longer solutions.
   * Here, on the other hand, we want to find solutions of any length.
   */
  while (len<=full_length)
  {
    if (series_solve_in_n(next,len)==len)
      result = true;
    ++len;
  }

  write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
