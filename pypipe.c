#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a new pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_pipe(SliceType type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  result = alloc_slice(type);
  slices[result].u.pipe.next = no_slice;
  slices[result].prev = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void pipe_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const copy = copy_slice(si);
    link_to_branch(copy,*root_slice);
    *root_slice = copy;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#if !defined(NDEBUG)
/* Does a slice have a successor
 * @param si identifies slice
 * @return true iff si identifies a slice that has a .next member
 */
static boolean has_successor_slot(slice_index si)
{
  return (slices[si].type!=STLeaf
          && slices[si].type!=STQuodlibet
          && slices[si].type!=STReciprocal);
}
#endif

/* Make a slice the successor of a pipe
 * @param pipe identifies the pipe
 * @param succ slice to be made the successor of pipe
 */
void pipe_set_successor(slice_index pipe, slice_index succ)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%u",succ);
  TraceFunctionParamListEnd();

  assert(has_successor_slot(pipe));
  slices[pipe].u.pipe.next = succ;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Establish a link between a branch slice and its successor
 * @param branch identifies branch slice
 * @param succ identifies branch to become the successor
 */
void pipe_link(slice_index pipe, slice_index succ)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%u",succ);
  TraceFunctionParamListEnd();

  pipe_set_successor(pipe,succ);
  slice_set_predecessor(succ,pipe);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Unlink a pipe and its successor
 * @param pipe identifies pipe slice
 */
void pipe_unlink(slice_index pipe)
{
  slice_index * const succ = &slices[pipe].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  assert(pipe!=no_slice);

  if (slices[*succ].prev==pipe)
    slices[*succ].prev = no_slice;

  *succ = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Replace a slice by another. Links the substitute to the replaced
 * slice's predecessor and successor, but doesn't adjust the links
 * from other slices that may reference the replaced slice.
 * Deallocates the replaced slice.
 * @param replaced identifies the replaced slice
 * @param substitute identifies the substitute
 */
void pipe_replace(slice_index replaced, slice_index substitute)
{
  slice_index const prev = slices[replaced].prev;
  slice_index const next = slices[replaced].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",replaced);
  TraceFunctionParam("%u",substitute);
  TraceFunctionParamListEnd();

  pipe_link(prev,substitute);
  pipe_link(substitute,next);
  dealloc_slice(replaced);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a slice to another. Links the two slices and establishes the
 * same connection from the appended slice to the previous successor
 * that existed between the previously connected slices.
 * @param pos identifies where to append
 * @param appended identifies appended slice
 */
void pipe_append(slice_index pos, slice_index appended)
{
  slice_index const next = slices[pos].u.pipe.next;
  slice_index const next_prev = slices[next].prev;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pos);
  TraceFunctionParam("%u",appended);
  TraceFunctionParamListEnd();

  pipe_link(pos,appended);
  if (next_prev==pos)
    slice_set_predecessor(next,appended);
  pipe_set_successor(appended,next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Removes a pipe slice. Links the predecessor to the succesor (if any).
 * Deallocates the removed pipe slice.
 * @param si identifies the pipe slice to be removed
 */
void pipe_remove(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_link(slices[si].prev,slices[si].u.pipe.next);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param pipe identifies slice being traversed
 * @param st status of traversal
 */
void pipe_detect_starter(slice_index pipe, stip_structure_traversal *st)
{
  slice_index const next = slices[pipe].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  if (slices[pipe].starter==no_side)
  {
    stip_traverse_structure(slices[pipe].u.pipe.next,st);
    slices[pipe].starter = slices[next].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void pipe_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.pipe.next!=no_slice)
    proxy_slice_resolve(&slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice has a solution
 * @param pipe slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type pipe_has_solution(slice_index pipe)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[pipe].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_pipe(slice_index pipe,
                                  stip_structure_traversal *st)
{
  slice_index const next = slices[pipe].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (next!=no_slice)
    stip_traverse_structure(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of some pipe slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_pipe(slice_index si, stip_moves_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(next!=no_slice);
  stip_traverse_moves(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
