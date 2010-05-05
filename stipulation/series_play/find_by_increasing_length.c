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

/* Shorten a root series branch. Reduces the length members of
 * slices[root] and resets the next member to the appropriate
 * position.
 * @param root index of the series root slice
 */
static void shorten_root_branch(slice_index root)
{
  slice_index fork;
  slice_index branch;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  fork = branch_find_slice(STSeriesFork,root);
  branch = slices[fork].u.pipe.next;
  assert(fork!=no_slice);
  assert(slices[branch].type==STSeriesMove);
  if (slices[root].u.shortcut.length==slack_length_series+2)
  {
    slices[fork].u.pipe.next = no_slice;
    dealloc_slice(branch);
  }

  shorten_series_pipe(root);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a series branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return - no_slice if not applicable (already shortened)
 *         - slice representing subsequent play if root has 1 half-move only
 *         - root (shortened) otherwise
 */
slice_index series_root_shorten_series_play(slice_index root)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root].type==STSeriesRoot);
  assert(slices[root].u.shortcut.length>slack_length_series);

  if (slices[root].u.shortcut.length==slack_length_series+1)
  {
    result = branch_deallocate(root);
    if (slices[result].type==STMoveInverterSeriesFilter)
    {
      slice_index const mi = result;
      result = slices[result].u.pipe.next;
      dealloc_slice(mi);
    }
  }
  else
  {
    shorten_root_branch(root);
    result = root;
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

  init_output(root);

  assert(slices[root].u.shortcut.min_length>=slack_length_series);

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);

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

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_root_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<=full_length)
    if (series_has_solution_in_n(next,len)==len)
    {
      result = has_solution;
      break;
    }
    else
      ++len;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
