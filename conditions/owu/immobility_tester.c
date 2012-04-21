#include "conditions/owu/immobility_tester.h"
#include "pystip.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "solving/king_move_generator.h"
#include "solving/non_king_move_generator.h"
#include "solving/legal_move_counter.h"
#include "solving/capture_counter.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_owu_specific_testers(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy_king = alloc_proxy_slice();
    slice_index const proxy_non_king = alloc_proxy_slice();
    slice_index const king_branch = slices[si].next1;
    slice_index const non_king_branch = stip_deep_copy(king_branch);
    slice_index const king_tester = alloc_pipe(STOWUImmobilityTesterKing);
    slice_index const non_king_tester = alloc_pipe(STImmobilityTester);

    pipe_link(si,alloc_and_slice(proxy_king,proxy_non_king));

    pipe_link(proxy_king,king_tester);
    link_to_branch(king_tester,king_branch);

    pipe_link(proxy_non_king,non_king_tester);
    link_to_branch(non_king_tester,non_king_branch);

    {
      slice_index const generator = branch_find_slice(STMoveGenerator,
                                                      king_branch);
      slice_index const prototype = alloc_pipe(STCaptureCounter);

      assert(generator!=no_slice);
      pipe_substitute(generator,alloc_king_move_generator_slice());

      branch_insert_slices(king_branch,&prototype,1);
    }

    {
      slice_index const generator = branch_find_slice(STMoveGenerator,
                                                      non_king_branch);
      assert(generator!=no_slice);
      pipe_substitute(generator,alloc_non_king_move_generator_slice());
    }

    pipe_remove(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute OWU specific immobility testers
 * @param si where to start (entry slice into stipulation)
 */
void owu_replace_immobility_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_owu_specific_testers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type owu_immobility_tester_king_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* avoid concurrent counts */
  assert(legal_move_counter_count[nbply+1]==0);
  assert(capture_counter_count==0);

  /* stop counting once we have >1 legal king moves */
  legal_move_counter_interesting[nbply+1] = 0;

  /* stop counting once we have >1 legal king captures */
  capture_counter_interesting = 1;

  attack(slices[si].next1,n);

  result = (legal_move_counter_count[nbply+1]==0 && capture_counter_count==1
            ? has_solution
            : has_no_solution);

  /* clean up after ourselves */
  capture_counter_count = 0;
  legal_move_counter_count[nbply+1] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
