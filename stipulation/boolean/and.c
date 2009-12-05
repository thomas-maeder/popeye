#include "pyrecipr.h"
#include "pyslice.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>


/* Allocate a reciprocal slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_reciprocal_slice(slice_index op1, slice_index op2)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  assert(op1!=no_slice);
  assert(op2!=no_slice);

  slices[result].type = STReciprocal; 
  slices[result].starter = no_side;
  slices[result].u.fork.op1 = op1;
  slices[result].u.fork.op2 = op2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean reci_are_threats_refuted(table threats, slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_are_threats_refuted(threats,op1)
            && slice_are_threats_refuted(threats,op2));

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reci_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(op1);
  if (result==has_solution)
    result = slice_has_solution(op2);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionParamListEnd();
  return result;
}

/* Determine whether a reciprocal slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean reci_has_non_starter_solved(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = (slice_has_non_starter_solved(op1)
            && slice_has_non_starter_solved(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_solve_postkey(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_solve_postkey(slices[si].u.fork.op1)
            && slice_solve_postkey(slices[si].u.fork.op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void reci_solve_threats(table threats, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_solve_threats(threats,slices[si].u.fork.op1);
  slice_solve_threats(threats,slices[si].u.fork.op2);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve at root level at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_root_solve(slice_index si)
{
  boolean result = false;

  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  if (slice_has_solution(op2)==has_solution && slice_root_solve(op1))
  {
    boolean const result2 = slice_root_solve(op2);
    assert(result2);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue solving at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_solve(slice_index si)
{
  boolean result = false;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  if (slice_has_solution(op2)==has_solution && slice_solve(op1))
  {
    boolean const result2 = slice_solve(op2);
    assert(result2);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter reci_detect_starter(slice_index si,
                                           boolean same_side_as_root)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  who_decides_on_starter result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    who_decides_on_starter const
        result1 = slice_detect_starter(op1,same_side_as_root);
    who_decides_on_starter const
        result2 = slice_detect_starter(op2,same_side_as_root);

    if (slices[op1].starter==no_side)
      slices[si].starter = slices[op2].starter;
    else
    {
      assert(slices[op1].starter==slices[op2].starter
             || slices[op2].starter==no_side);
      slices[si].starter = slices[op1].starter;
    }

    if (result1==dont_know_who_decides_on_starter)
      result = result2;
    else
    {
      assert(result2==dont_know_who_decides_on_starter
             || slices[op1].starter==slices[op2].starter);
      result = result1;
    }
  }
  else
    result = leaf_decides_on_starter;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean reci_impose_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
