#include "stipulation/if_then_else.h"
#include "stipulation/stipulation.h"
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
