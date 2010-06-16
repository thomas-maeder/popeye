#include "pynot.h"
#include "pyslice.h"
#include "pypipe.h"
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

  result = alloc_pipe(STNot);
  pipe_link(result,op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void not_insert_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  slice_index not;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  *root = alloc_not_slice(*root);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type not_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_solve(slices[si].u.pipe.next))
  {
    case has_solution:
      result = has_no_solution;
      break;

    case has_no_solution:
      result = has_solution;
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
    case opponent_self_check:
      result = opponent_self_check;
      break;

    case has_no_solution:
      result = has_solution;
      break;

    case has_solution:
      result = has_no_solution;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
