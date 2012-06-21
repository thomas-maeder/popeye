#include "stipulation/binary.h"
#include "pystip.h"
#include "stipulation/proxy.h"
#include "solving/solving.h"
#include "debugging/trace.h"

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

  assert(op1==no_slice || slices[op1].type==STProxy);
  assert(op2==no_slice || slices[op2].type==STProxy);

  result = create_slice(type);
  slices[result].next1 = op1;
  slices[result].next2 = op2;

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

  stip_traverse_structure_binary_operand1(si,st);
  root_op1 = state->spun_off[slices[si].next1];
  TraceValue("%u\n",root_op1);

  stip_traverse_structure_binary_operand2(si,st);
  root_op2 = state->spun_off[slices[si].next2];
  TraceValue("%u\n",root_op2);

  if (st->context==stip_traversal_context_intro)
  {
    state->spun_off[si] = si;
    pipe_unlink(slices[si].prev);
  }
  else
    state->spun_off[si] = copy_slice(si);

  slices[state->spun_off[si]].next1 = root_op1;
  slices[state->spun_off[si]].next2 = root_op2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void binary_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const op1 = slices[si].next1;
  slice_index const op2 = slices[si].next2;

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

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a binary slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_binary(slice_index si, stip_structure_traversal *st)
{
  boolean const * const spinning_off = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*spinning_off)
  {
    slices[si].tester = copy_slice(si);
    stip_traverse_structure_children(si,st);
    assert(slices[slices[si].next1].tester!=no_slice);
    assert(slices[slices[si].next2].tester!=no_slice);
    slices[slices[si].tester].next1 = slices[slices[si].next1].tester;
    slices[slices[si].tester].next2 = slices[slices[si].next2].tester;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
