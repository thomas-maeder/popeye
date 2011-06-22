#include "stipulation/boolean/binary.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a binary slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_binary_slice(slice_type type,
                               slice_index op1, slice_index op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  assert(op1!=no_slice);
  assert(slices[op1].type==STProxy);

  assert(op2!=no_slice);
  assert(slices[op2].type==STProxy);

  result = alloc_slice(type);
  slices[result].u.binary.op1 = op1;
  slices[result].u.binary.op2 = op2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void binary_make_root(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;
  slice_index root_op1;
  slice_index root_op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  root_op1 = state->spun_off[slices[si].u.binary.op1];
  TraceValue("%u\n",root_op1);

  stip_traverse_structure(slices[si].u.binary.op2,st);
  root_op2 = state->spun_off[slices[si].u.binary.op2];
  TraceValue("%u\n",root_op2);

  if (st->context==stip_traversal_context_global)
  {
    state->spun_off[si] = si;
    pipe_unlink(slices[si].prev);
  }
  else
    state->spun_off[si] = copy_slice(si);

  slices[state->spun_off[si]].u.binary.op1 = root_op1;
  slices[state->spun_off[si]].u.binary.op2 = root_op2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void binary_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  proxy_slice_resolve(&slices[si].u.binary.op1,st);
  proxy_slice_resolve(&slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param fork root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_binary(slice_index fork,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[fork].u.binary.op1,st);
  stip_traverse_structure(slices[fork].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of an operand of a binary operator
 * @param op identifies operand
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_binary_operand(slice_index op,
                                        stip_moves_traversal *st)
{
  stip_length_type const save_remaining = st->remaining;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_global)
  {
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    stip_traverse_moves(op,st);
    st->remaining = save_remaining;
  }
  else
    stip_traverse_moves(op,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of a binary operator
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_binary(slice_index si, stip_moves_traversal *st)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_binary_operand(slices[si].u.binary.op1,st);
  stip_traverse_moves_binary_operand(slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void get_max_nr_moves_binary(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;
  stip_length_type const save_result = *result;
  stip_length_type result1;
  stip_length_type result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_binary_operand(slices[si].u.binary.op1,st);
  result1 = *result;
  TraceValue("%u\n",result1);

  *result = save_result;
  stip_traverse_moves_binary_operand(slices[si].u.binary.op2,st);
  result2 = *result;
  TraceValue("%u\n",result2);

  if (result1>result2)
    *result = result1;
  TraceValue("%u\n",*result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void binary_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    stip_traverse_structure(op1,st);
    stip_traverse_structure(op2,st);

    if (slices[op1].starter==no_side)
      slices[si].starter = slices[op2].starter;
    else
    {
      assert(slices[op2].starter==no_side
             || slices[op1].starter==slices[op2].starter);
      slices[si].starter = slices[op1].starter;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
