#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "pypipe.h"
#include "trace.h"

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return newly allocated slice
 */
slice_index alloc_branch(SliceType type,
                         stip_length_type length,
                         stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;
  slices[result].u.branch.imminent_goal.type = no_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the next slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(SliceType type, slice_index si)
{
  slice_index result = si;
  boolean slices_visited[max_nr_slices] = { false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  do
  {
    TraceValue("%u\n",slices[result].u.pipe.next);
    result = slices[result].u.pipe.next;
    if (slices[result].type==type)
      break;
    else if (slices_visited[result]
             || !slice_has_structure(result,slice_structure_pipe))
    {
      result = no_slice;
      break;
    }
    else
      slices_visited[result] = true;
  } while (true);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the full_length and remaing fields of a
 * stip_moves_traversal struct from a branch slice if necessary
 * @param si identifies the branch slice
 * @param st refers to the struct to be initialised
 */
void stip_traverse_moves_branch_init_full_length(slice_index si,
                                                 stip_moves_traversal *st)
{
  if (st->remaining==0)
  {
    st->full_length = slices[si].u.branch.length;
    TraceValue("->%u",st->full_length);
    st->remaining = slices[si].u.branch.length;
  }
}

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch_slice(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  --st->remaining;
  TraceValue("->%u\n",st->remaining);
  stip_traverse_moves_pipe(si,st);
  ++st->remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of a branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch(slice_index si, stip_moves_traversal *st)
{
  stip_length_type const save_remaining = st->remaining;
  stip_length_type const save_full_length = st->full_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++st->level;
  st->remaining = 0;

  stip_traverse_moves(si,st);

  st->full_length = save_full_length;
  st->remaining = save_remaining;
  TraceFunctionParam("->%u\n",st->remaining);
  --st->level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  if (slices[entry].prev==no_slice)
    pipe_link(pipe,entry);
  else
    pipe_set_successor(pipe,entry);


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
