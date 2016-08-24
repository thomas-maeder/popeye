#include "stipulation/binary.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
  TraceEnumerator(slice_type,type);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  assert(op1==no_slice || SLICE_TYPE(op1)==STProxy);
  assert(op2==no_slice || SLICE_TYPE(op2)==STProxy);

  result = create_slice(type);
  SLICE_NEXT1(result) = op1;
  SLICE_NEXT2(result) = op2;

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
  root_op1 = state->spun_off[SLICE_NEXT1(si)];
  TraceValue("%u",root_op1);
  TraceEOL();

  stip_traverse_structure_binary_operand2(si,st);
  root_op2 = state->spun_off[SLICE_NEXT2(si)];
  TraceValue("%u",root_op2);
  TraceEOL();

  if (st->context==stip_traversal_context_intro)
  {
    state->spun_off[si] = si;
    pipe_unlink(SLICE_PREV(si));
  }
  else
    state->spun_off[si] = copy_slice(si);

  SLICE_NEXT1(state->spun_off[si]) = root_op1;
  SLICE_NEXT2(state->spun_off[si]) = root_op2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void binary_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const op1 = SLICE_NEXT1(si);
  slice_index const op2 = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==no_side)
  {
    stip_traverse_structure(op1,st);
    stip_traverse_structure(op2,st);

    if (SLICE_STARTER(op1)==no_side)
      SLICE_STARTER(si) = SLICE_STARTER(op2);
    else
    {
      assert(SLICE_STARTER(op2)==no_side
             || SLICE_STARTER(op1)==SLICE_STARTER(op2));
      SLICE_STARTER(si) = SLICE_STARTER(op1);
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_TESTER(si) = copy_slice(si);
  stip_traverse_structure_children(si,st);
  assert(SLICE_TESTER(SLICE_NEXT1(si))!=no_slice);
  assert(SLICE_TESTER(SLICE_NEXT2(si))!=no_slice);
  SLICE_NEXT1(SLICE_TESTER(si)) = SLICE_TESTER(SLICE_NEXT1(si));
  SLICE_NEXT2(SLICE_TESTER(si)) = SLICE_TESTER(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
