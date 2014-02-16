#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a new pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_pipe(slice_type type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  result = create_slice(type);
  slices[result].next1 = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin a copy off a pipe to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void pipe_spin_off_copy(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->spun_off[si] = copy_slice(si);

  if (slices[si].next1!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    if (state->spun_off[slices[si].next1]==no_slice)
    {
      dealloc_slice(state->spun_off[si]);
      state->spun_off[si] = no_slice;
    }
    else
      link_to_branch(state->spun_off[si],state->spun_off[slices[si].next1]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Skip over a pipe while spinning of slices for the root or set play branch
 * @param si slice index
 * @param st state of traversal
 */
void pipe_spin_off_skip(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  TraceValue("%u\n",state->spun_off[slices[si].next1]);

  state->spun_off[si] = state->spun_off[slices[si].next1];
  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

  assert(slice_type_get_structural_type(slices[pipe].type)!=slice_structure_leaf);
  slices[pipe].next1 = succ;

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
  slice_index * const succ = &slices[pipe].next1;

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

/* Substitute a slice for another.
 * Copies "the guts" of slice substitute into slice replaced, but leaves
 * replaced's links to the previous and successive slices intact.
 * Deallocates substitute.
 * @param replaced identifies the replaced slice
 * @param substitute identifies the substitute
 */
void pipe_substitute(slice_index replaced, slice_index substitute)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",replaced);
  TraceFunctionParam("%u",substitute);
  TraceFunctionParamListEnd();

  slices[substitute].prev = slices[replaced].prev;
  slices[substitute].next1 = slices[replaced].next1;
  slices[replaced] = slices[substitute];
  dealloc_slice(substitute);

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
  slice_index const next = slices[pos].next1;
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

  if (slices[slices[si].next1].prev==si)
    pipe_link(slices[si].prev,slices[si].next1);
  else
    pipe_set_successor(slices[si].prev,slices[si].next1);

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
  slice_index const next = slices[pipe].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  if (slices[pipe].starter==no_side && next!=no_slice)
  {
    stip_traverse_structure_children_pipe(pipe,st);
    slices[pipe].starter = slices[next].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a pipe slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_pipe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].tester = copy_slice(si);

  if (slices[si].next1!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    link_to_branch(slices[si].tester,slices[slices[si].next1].tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a proxy slice off a pipe slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_pipe_skip(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].tester = alloc_proxy_slice();

  if (slices[si].next1!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    link_to_branch(slices[si].tester,slices[slices[si].next1].tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
