#include "stipulation/goals/immobile/reached_tester_non_king.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "solving/non_king_move_generator.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile apart from possible king moves
 */

/* Convert a help branch into a branch testing for immobility apart from
 * possible king moves
 * @param help_branch identifies entry slice into help branch
 * @return identifier of entry slice into tester branch
 */
slice_index make_immobility_tester_non_king(slice_index help_branch)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",help_branch);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STImmobilityTesterNonKing);
  link_to_branch(result,help_branch);

  {
    slice_index const generator = branch_find_slice(STMoveGenerator,help_branch);
    assert(generator!=no_slice);
    pipe_substitute(generator,alloc_non_king_move_generator_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
