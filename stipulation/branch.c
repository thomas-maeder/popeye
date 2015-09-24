#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return newly allocated slice
 */
slice_index alloc_branch(slice_type type,
                         stip_length_type length,
                         stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  SLICE_U(result).branch.length = length;
  SLICE_U(result).branch.min_length = min_length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  slice_type to_be_found;
  slice_index result;
} branch_find_slice_state_type;

static void branch_find_slice_pipe(slice_index si, stip_structure_traversal *st)
{
  branch_find_slice_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_TYPE(si)==state->to_be_found)
    state->result = si;
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void branch_find_slice_binary(slice_index si, stip_structure_traversal *st)
{
  branch_find_slice_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_TYPE(si)==state->to_be_found)
    state->result = si;
  else
  {
    slice_index result1;
    slice_index result2;

    stip_traverse_structure_binary_operand1(si,st);
    result1 = state->result;
    state->result = no_slice;

    stip_traverse_structure_binary_operand2(si,st);
    result2 = state->result;

    if (result1==no_slice)
      state->result = result2;
    else if (result2==no_slice)
      state->result = result1;
    else
    {
      if (result1==result2)
        state->result = result1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the next1 slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching
 * @param context context at start of traversal
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(slice_type type,
                              slice_index si,
                              stip_traversal_context_type context)
{
  branch_find_slice_state_type state = { type, no_slice };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  st.context = context;
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &branch_find_slice_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &branch_find_slice_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &branch_find_slice_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_binary,
                                                  &branch_find_slice_binary);
  stip_traverse_structure_children_pipe(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.result);
  TraceFunctionResultEnd();
  return state.result;
}

/* Link a pipe slice to the entry slice of a branch
 * @param pipe identifies the pipe slice
 * @param entry identifies the entry slice of the branch
 */
void link_to_branch(slice_index pipe, slice_index entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%u",entry);
  TraceFunctionParamListEnd();

  if (SLICE_PREV(entry)==no_slice)
    pipe_link(pipe,entry);
  else
    pipe_set_successor(pipe,entry);


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void shorten_pipe(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const end_type = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_TYPE(si)!=*end_type)
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

static void shorten_branch(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const end_type = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_TYPE(si)!=*end_type)
  {
    stip_traverse_structure_children_pipe(si,st);
    SLICE_U(si).branch.length -= 2;
    SLICE_U(si).branch.min_length -= 2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Shorten slices of a branch by 2 half moves
 * @param start identfies start of sequence of slices to be shortened
 * @param end_type identifies type of slice where to stop shortening
 * @param context traversal context at start
 */
void branch_shorten_slices(slice_index start,
                           slice_type end_type,
                           stip_traversal_context_type context)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&end_type);
  st.context = context;
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &shorten_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &shorten_branch);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &shorten_pipe);
  stip_traverse_structure(start,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}


/* Instrument a traversal for traversing the "normal path" through a branch.
 * In particular, the traversal won't enter nested branches.
 * @param st traversal to be instrumented
 * @note The caller must already have invoked a stip_structure_traversal_init*
 *       function on st
 */
void branch_instrument_traversal_for_normal_path(stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_override_by_contextual(st,
                                                  slice_contextual_end_of_branch,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(st,
                                           STIfThenElse,
                                           &stip_traverse_structure_children_pipe);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
