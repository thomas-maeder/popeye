#include "stipulation/if_then_else.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/binary.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STIfThenElse slice.
 * @param normal identifies "normal" (else) successor
 * @param exceptional identifies "exceptional" (if) successor
 * @param condition identifies condition on which to take exceptional path
 * @return index of allocated slice
 */
slice_index alloc_if_then_else_slice(slice_index normal,
                                     slice_index exceptional,
                                     slice_index condition)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",normal);
  TraceFunctionParam("%u",exceptional);
  TraceFunctionParam("%u",condition);
  TraceFunctionParamListEnd();

  result =  alloc_binary_slice(STIfThenElse,normal,exceptional);
  slices[result].u.if_then_else.condition = condition;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type if_then_else_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index succ;
  slice_index const op1 = slices[si].next1;
  slice_index const op2 = slices[si].next2;
  slice_index const condition = slices[si].u.if_then_else.condition;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  succ = solve(condition,slack_length)==slack_length ? op2 : op1;
  result = solve(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a binary slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_if_then_else(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_spin_off_testers_binary(si,st);

  slices[slices[si].tester].u.if_then_else.condition = slices[slices[si].u.if_then_else.condition].tester;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
