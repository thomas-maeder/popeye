#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "solving/king_move_generator.h"
#include "solving/non_king_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_king_first(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].starter!=no_side);

  stip_traverse_structure_children(si,st);

  /* this optimisation doesn't work if an ultra-mummer condition applies
   * to the side to be immobilised */
  if (!ultra_mummer[slices[si].starter])
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const king_branch = slices[si].u.pipe.next;
    slice_index const nonking_branch = stip_deep_copy(king_branch);
    slice_index const king_move_tester = alloc_pipe(STImmobilityTester);
    slice_index const nonking_move_tester = alloc_pipe(STImmobilityTester);

    pipe_link(si,alloc_and_slice(proxy1,proxy2));

    pipe_link(proxy1,king_move_tester);
    link_to_branch(king_move_tester,king_branch);

    {
      slice_index const generator = branch_find_slice(STMoveGenerator,king_branch);
      assert(generator!=no_slice);
      pipe_substitute(generator,alloc_king_move_generator_slice());
    }

    pipe_link(proxy2,nonking_move_tester);
    link_to_branch(nonking_move_tester,nonking_branch);

    {
      slice_index const generator = branch_find_slice(STMoveGenerator,nonking_branch);
      assert(generator!=no_slice);
      pipe_substitute(generator,alloc_non_king_move_generator_slice());
    }

    pipe_remove(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 */
void immobility_testers_substitute_king_first(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_king_first);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
