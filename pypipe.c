#include "pypipe.h"
#include "pyseries.h"
#include "trace.h"

/* Allocate a new pipe and make an existing pipe its successor
 * @param successor successor of slice to be allocated
 * @return newly allocated slice
 */
slice_index alloc_pipe(slice_index successor)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",successor);
  TraceFunctionParamListEnd();

  slices[result].u.pipe.next = successor;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Installs a pipe slice before pipe slice si. I.e. every link that
 * currently leads to slice si will now lead to the new slice.
 * @param si identifies pipe slice before which to insert a new pipe slice
 */
void pipe_insert_before(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.pipe.next = copy_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Installs a pipe slice after pipe slice si.
 * @param si identifies pipe slice after which to insert a new pipe slice
 */
void pipe_insert_after(slice_index si)
{
  slice_index const curr_next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.pipe.next = copy_slice(curr_next);
  TraceValue("%u",curr_next);
  TraceValue("%u\n",slices[si].u.pipe.next);
  slices[slices[si].u.pipe.next].u.pipe.next = curr_next;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Removes a pipe slice after pipe slice si. This is the inverse
 * operation to pipe_insert_after(); if another slice references
 * slices[si].u.pipe.next, that reference will be dangling.  
 * @param si identifies pipe slice after which to insert a new pipe slice
 */
void pipe_remove_after(slice_index si)
{
  slice_index const removed = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.pipe.next = slices[removed].u.pipe.next;
  dealloc_slice_index(removed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean pipe_detect_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    result = slice_traverse_children(si,st);
    slices[si].starter = slices[next].starter;
  }
  else
    result = true;

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
boolean pipe_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  traverse_slices(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation. Impose the inverted
 * starter on the slice's successor. 
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean pipe_impose_inverted_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;

  *starter = advers(*starter);
  traverse_slices(slices[si].u.pipe.next,st);
  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse the sub-graph starting at the successor slice of a pipe
 * slice (but don't traverse possible other children of the pipe
 * slice)
 * @param pipe identifies pipe slice
 * @return true iff the sub-graph has been successfully traversed
 */
boolean pipe_traverse_next(slice_index pipe, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  traverse_slices(slices[pipe].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type pipe_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @param n exact number of half moves until end state has to be reached
 * @return true iff slice si has a solution
 */
boolean pipe_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = series_solve_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
