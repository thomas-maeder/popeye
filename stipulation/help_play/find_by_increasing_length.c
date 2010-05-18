#include "stipulation/help_play/root.h"
#include "pydata.h"
#include "pyslice.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/play.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_root_slice(stip_length_type length,
                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_help)
    min_length += 2;

  result = alloc_branch(STHelpRoot,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_root_root_solve(slice_index root)
{
  boolean result = false;
  slice_index const next = slices[root].u.pipe.next;
  stip_length_type const full_length = slices[root].u.branch.length;
  stip_length_type len = slices[root].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  /* Do *not* delegate to help_solve() here:
   * If help_solve() has found solutions of a certain length, it won't
   * look for longer solutions.
   * Here, on the other hand, we want to find solutions of any length.
   */
  while (len<=full_length)
  {
    if (help_solve_in_n(next,len)==len)
      result = true;
    len += 2;
  }

  write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_root_has_solution_in_n(slice_index si,
                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = help_has_solution_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

