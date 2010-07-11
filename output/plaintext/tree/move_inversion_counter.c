#include "output/plaintext/tree/move_inversion_counter.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* This module provides the STOutputPlaintextTreeGoalWriter slice type.
 * Slices of this type write the goal at the end of a variation
 */

/* Number of move inversions up to the current move
 */
unsigned int output_plaintext_tree_nr_move_inversions;

/* Allocate a STOutputPlaintextTreeMoveInversionCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_tree_move_inversion_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextTreeMoveInversionCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
output_plaintext_tree_move_inversion_counter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
output_plaintext_tree_move_inversion_counter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++output_plaintext_tree_nr_move_inversions;
  result = slice_solve(next);
  --output_plaintext_tree_nr_move_inversions;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
