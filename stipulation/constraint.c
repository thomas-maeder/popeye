#include "pyreflxg.h"
#include "pyhelp.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STReflexGuard slice.
 * @param not_slice identifies slice representing positions to avoid
 * @return index of allocated slice
 */
slice_index alloc_reflex_guard_slice(slice_index not_slice)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",not_slice);
  TraceFunctionParamListEnd();

  slices[result].type = STReflexGuard; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = no_slice;
  slices[result].u.pipe.u.reflex_guard.not_slice = not_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  result = (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice)
            && help_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  result = (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice)
            && help_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void reflex_guard_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (not_has_solution(slices[si].u.pipe.u.reflex_guard.not_slice))
    help_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
