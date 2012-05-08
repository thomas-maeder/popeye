#include "stipulation/boolean/and.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/constraint.h"
#include "stipulation/boolean/binary.h"
#include "solving/solving.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STAnd slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_and_slice(slice_index op1, slice_index op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STAnd,op1,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type and_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].next1;
  slice_index const op2 = slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(op1,n);
  if (slack_length<=result && result<=n)
    result = attack(op2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a testing pipe slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_and(slice_index si, stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
    stip_spin_off_testers_binary(si,st);
  else
  {
    stip_traverse_structure_binary_operand1(si,st);
    state->spinning_off = true;
    stip_traverse_structure_binary_operand2(si,st);
    state->spinning_off = false;

    assert(state->spun_off[slices[si].next2]!=no_slice);
    pipe_append(slices[si].next1,
                alloc_constraint_tester_slice(state->spun_off[slices[si].next2]));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
