#include "pyquodli.h"
#include "pyslice.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Construct a quodlibet slice over an already allocated slice object
 * @param si index of slice object where to construct quodlibet slice
 * @param op1 1st operand
 * @param op2 2nd operand
 */
void make_quodlibet_slice(slice_index si, slice_index op1, slice_index op2)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  slices[si].type = STQuodlibet; 
  slices[si].u.fork.op1 = op1;
  slices[si].u.fork.op2 = op2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a quodlibet slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index op1, slice_index op2)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  make_quodlibet_slice(result,op1,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void quodlibet_solve_threats(table threats, slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_solve_threats(threats,op1);
  slice_solve_threats(threats,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void quodlibet_root_solve_in_n(slice_index si, stip_length_type n)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_root_solve_in_n(op1,n);
  slice_root_solve_in_n(op2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a quodlibet slice at root level
 * @param si slice index
 */
boolean quodlibet_root_solve(slice_index si)
{
  boolean result = false;
  boolean result1;
  boolean result2;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result1 = slice_root_solve(op1);
  result2 = slice_root_solve(op2);
  result = result1 || result2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean quodlibet_root_defend(slice_index si,
                              unsigned int max_number_refutations)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  boolean result1;
  boolean result2;
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",max_number_refutations);
  TraceFunctionParamListEnd();

  result1 = slice_root_defend(op1,max_number_refutations);
  result2 = slice_root_defend(op2,max_number_refutations);
  result = result1 && result2;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean quodlibet_are_threats_refuted(table threats,
                                      stip_length_type len_threat,
                                      slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* TODO this can't be correct */
  result = (slice_are_threats_refuted(threats,len_threat,op1)
            && slice_are_threats_refuted(threats,len_threat,op2));

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(op1);
  if (result==has_no_solution)
    result = slice_has_solution(op2);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionParamListEnd();
  return result;
}

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_has_non_starter_solved(slice_index si)
{
  boolean result = true;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_has_non_starter_solved(op1)
            || slice_has_non_starter_solved(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations
 * @param leaf slice index
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int quodlibet_count_refutations(slice_index si,
                                         unsigned int max_result)
{
  unsigned int result;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_count_refutations(op1,max_result);
  if (result>max_result)
    result = slice_count_refutations(op2,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean quodlibet_defend(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_defend(op1) && slice_defend(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a quodlibet slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve(slice_index si)
{
  boolean found_solution_op1 = false;
  boolean found_solution_op2 = false;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  /* avoid short-cut boolean evaluation */
  found_solution_op1 = slice_solve(op1);
  found_solution_op2 = slice_solve(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution_op1 || found_solution_op2);
  return found_solution_op1 || found_solution_op2;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter quodlibet_detect_starter(slice_index si,
                                                boolean same_side_as_root)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  who_decides_on_starter result;
  who_decides_on_starter result1;
  who_decides_on_starter result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STQuodlibet);

  TraceValue("%u",slices[si].u.fork.op1);
  TraceValue("%u\n",slices[si].u.fork.op2);

  result1 = slice_detect_starter(op1,same_side_as_root);
  result2 = slice_detect_starter(op2,same_side_as_root);

  if (slices[op1].starter==no_side)
    slices[si].starter = slices[op2].starter;
  else
  {
    assert(slices[op2].starter==no_side
           || slices[op1].starter==slices[op2].starter);
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
boolean quodlibet_impose_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
