#include "stipulation/help_play/root.h"
#include "pydata.h"
#include "pyoutput.h"
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

  init_output(root);

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);

  while (len<=full_length)
  {
    if (help_solve_in_n(next,len)<=len)
      result = true;
    len += 2;
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
has_solution_type help_root_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<=full_length)
    if (help_has_solution_in_n(next,len)<=len)
    {
      result = has_solution;
      break;
    }
    else
      len += 2;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
