#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */

/* Allocate a new pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice, or no_slice on error
 */
slice_index alloc_pipe(slice_type type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  result = create_slice(type);
  if (result==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to create slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(1); /* TODO: Do we have to exit here? */
  }
  SLICE_NEXT1(result) = no_slice;

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
  if (state->spun_off[si]==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(2); /* TODO: Do we have to exit here? */
  }

  if (SLICE_NEXT1(si)!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    if (state->spun_off[SLICE_NEXT1(si)]==no_slice)
    {
      dealloc_slice(state->spun_off[si]);
      state->spun_off[si] = no_slice;
    }
    else
      link_to_branch(state->spun_off[si],state->spun_off[SLICE_NEXT1(si)]);
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
  TraceValue("%u",state->spun_off[SLICE_NEXT1(si)]);
  TraceEOL();

  state->spun_off[si] = state->spun_off[SLICE_NEXT1(si)];
  TraceValue("%u",state->spun_off[si]);
  TraceEOL();

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

  assert(slice_type_get_structural_type(SLICE_TYPE(pipe))!=slice_structure_leaf);
  SLICE_NEXT1(pipe) = succ;

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
  slice_index * succ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  assert(pipe!=no_slice);

  succ = &SLICE_NEXT1(pipe);

  if (SLICE_PREV(*succ)==pipe)
    SLICE_PREV(*succ) = no_slice;

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

  SLICE_PREV(substitute) = SLICE_PREV(replaced);
  SLICE_NEXT1(substitute) = SLICE_NEXT1(replaced);
  SLICE(replaced) = SLICE(substitute);
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
  slice_index const next = SLICE_NEXT1(pos);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pos);
  TraceFunctionParam("%u",appended);
  TraceFunctionParamListEnd();

  pipe_link(pos,appended);
  if ((next != no_slice) && (SLICE_PREV(next)==pos))
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
  slice_index const next = SLICE_NEXT1(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((next != no_slice) && (SLICE_PREV(next)==si))
    pipe_link(SLICE_PREV(si),next);
  else
    pipe_set_successor(SLICE_PREV(si),next);

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
  slice_index const next = SLICE_NEXT1(pipe);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(pipe)==no_side && next!=no_slice)
  {
    stip_traverse_structure_children_pipe(pipe,st);
    SLICE_STARTER(pipe) = SLICE_STARTER(next);
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

  SLICE_TESTER(si) = copy_slice(si);
  if (SLICE_TESTER(si)==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(2); /* TODO: Do we have to exit here? */
  }

  if (SLICE_NEXT1(si)!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    link_to_branch(SLICE_TESTER(si),SLICE_TESTER(SLICE_NEXT1(si)));
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

  SLICE_TESTER(si) = alloc_proxy_slice();

  if (SLICE_NEXT1(si)!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    link_to_branch(SLICE_TESTER(si),SLICE_TESTER(SLICE_NEXT1(si)));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
