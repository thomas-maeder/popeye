#include "pynot.h"
#include "pyslice.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* Allocate a not slice.
 * @param op operand
 * @return index of allocated slice
 */
slice_index alloc_not_slice(slice_index op)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op);
  TraceFunctionParamListEnd();

  assert(op!=no_slice);

  result = alloc_slice(STNot);
  slices[result].u.pipe.next = op;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean not_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* Don't write anything, but return the correct value so that it can
   * be written to the hash table!
   */

  result = slice_has_solution(slices[si].u.pipe.next)!=has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type not_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(slices[si].u.pipe.next))
  {
    case defender_self_check:
      result = defender_self_check;
      break;

    case has_solution:
      result = has_no_solution;
      break;

    case has_no_solution:
      result = has_solution;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void not_solve_threats(table threats, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the solution of a slice
 * @param slice index
 * @return true iff >=1 solution was found
 */
boolean not_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.next)==has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
