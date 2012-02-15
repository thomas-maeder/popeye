#include "stipulation/boolean/and.h"
#include "pypipe.h"
#include "stipulation/constraint.h"
#include "stipulation/boolean/binary.h"
#include "solving/solving.h"
#include "trace.h"

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

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type and_solve(slice_index si)
{
  has_solution_type result;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(op1);
  if (result==has_solution)
    result = slice_solve(op2);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
    stip_traverse_structure(slices[si].u.binary.op1,st);
    state->spinning_off = true;
    stip_traverse_structure(slices[si].u.binary.op2,st);
    state->spinning_off = false;

    assert(state->spun_off[slices[si].u.binary.op2]!=no_slice);
    pipe_append(slices[si].u.binary.op1,
                alloc_constraint_tester_slice(state->spun_off[slices[si].u.binary.op2]));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
