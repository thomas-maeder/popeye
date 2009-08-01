#include "pypipe.h"
#include "trace.h"

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

  slices[si].u.pipe.next = alloc_slice_index();
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
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
