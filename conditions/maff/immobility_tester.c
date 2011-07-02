#include "conditions/maff/immobility_tester.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "stipulation/goals/immobile/reached_tester_non_king.h"
#include "solving/king_move_generator.h"
#include "solving/legal_move_counter.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_maff_specific_testers(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const next = slices[si].u.pipe.next;
    slice_index const king_tester = alloc_pipe(STMaffImmobilityTesterKing);

    pipe_link(si,alloc_and_slice(proxy1,proxy2));
    pipe_link(proxy1,king_tester);
    pipe_link(king_tester,next);

    pipe_link(proxy2,make_immobility_tester_non_king(stip_deep_copy(next)));

    {
      slice_index const generator = branch_find_slice(STMoveGenerator,next);
      assert(generator!=no_slice);
      pipe_substitute(generator,alloc_king_move_generator_slice());
    }

    pipe_remove(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Replace immobility tester slices to cope with condition MAFF
 * @param si where to start (entry slice into stipulation)
 */
void maff_replace_immobility_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_maff_specific_testers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maff_immobility_tester_king_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* avoid concurrent counts */
  assert(legal_move_counter_count==0);

  /* stop counting once we have >1 legal king moves */
  legal_move_counter_interesting = 1;

  slice_has_solution(slices[si].u.pipe.next);

  /* apply the MAFF rule */
  result = legal_move_counter_count==1 ? has_solution : has_no_solution;

  /* clean up after ourselves */
  legal_move_counter_count = 0;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
