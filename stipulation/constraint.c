#include "pyreflxg.h"
#include "pyhelp.h"
#include "pynot.h"
#include "trace.h"

#include <assert.h>

/* Insert a STReflexGuard slice in front (and at the place)
 * of an existing slice.
 * @param si identifies slice to be superseded by a STReflexGuard slice
 * @param to_be_avoided prototype of slice that must be solvable
 */
void insert_reflex_guard_slice(slice_index si, slice_index to_be_avoided)
{
  slice_index not_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_be_avoided);
  TraceFunctionParamListEnd();

  slices[si].u.pipe.next = copy_slice(si);

  slices[si].type = STReflexGuard; 
  slices[si].starter = no_side; 

  /* don't link not_slice to to_be_avoided: to_be_avoided and
   * to_be_avoided_copy will have different starters!
   * TODO deep copy needed in general
   */
  not_slice = alloc_not_slice(copy_slice(to_be_avoided));
  slices[si].u.pipe.u.reflex_guard.not_slice = not_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean reflex_guard_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
