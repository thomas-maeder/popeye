#include "conditions/maff/immobility_tester.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "solving/king_move_generator.h"
#include "solving/legal_move_counter.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

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
    slice_index const next1 = slices[si].u.pipe.next;
    slice_index const next2 = stip_deep_copy(next1);
    slice_index const king_tester = alloc_pipe(STMaffImmobilityTesterKing);
    slice_index const other_tester = alloc_pipe(STImmobilityTesterNonKing);

    slice_index const generator1 = branch_find_slice(STMoveGenerator,next1);
    assert(generator1!=no_slice);
    pipe_substitute(generator1,alloc_king_move_generator_slice());

    {
      slice_index const prototype = alloc_pipe(STLegalMoveCounter);
      branch_insert_slices(next1,&prototype,1);
    }

    pipe_link(si,alloc_and_slice(proxy1,proxy2));
    pipe_link(proxy1,king_tester);
    pipe_link(king_tester,next1);
    pipe_link(proxy2,other_tester);
    pipe_link(other_tester,next2);

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
