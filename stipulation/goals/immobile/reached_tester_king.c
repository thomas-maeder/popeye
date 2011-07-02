#include "stipulation/goals/immobile/reached_tester_king.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "solving/legal_move_counter.h"
#include "solving/king_move_generator.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side's king is immobile
 */

/* Convert a help branch into a branch testing for immobility of a side's king
 * @param help_branch identifies entry slice into help branch
 * @return identifier of entry slice into tester branch
 */
slice_index make_immobility_tester_king(slice_index help_branch)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",help_branch);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STImmobilityTesterKing);
  link_to_branch(result,help_branch);

  {
    slice_index const generator = branch_find_slice(STMoveGenerator,help_branch);
    slice_index const prototype = alloc_pipe(STLegalMoveCounter);

    assert(generator!=no_slice);
    pipe_substitute(generator,alloc_king_move_generator_slice());

    branch_insert_slices(help_branch,&prototype,1);
  }

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
has_solution_type immobility_tester_king_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* avoid concurrent counts */
  assert(legal_move_counter_count==0);

  /* stop counting once we have >1 legal king moves */
  legal_move_counter_interesting = 0;

  slice_has_solution(slices[si].u.pipe.next);

  result = legal_move_counter_count==0 ? has_solution : has_no_solution;

  /* clean up after ourselves */
  legal_move_counter_count = 0;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
