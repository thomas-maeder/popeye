#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "pypipe.h"
#include "pyselfcg.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "stipulation/boolean/and.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality that detects whether double stalemate goal
 * has been reached
 */

/* Allocate a system of slices that test whether double stalemate has
 * been reached
 * @return index of entry slice
 */
slice_index alloc_goal_doublestalemate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_dblstale, initsquare };
    slice_index const dblstale_tester = alloc_pipe(STGoalDoubleStalemateReachedTester);
    slice_index const proxy_starter = alloc_proxy_slice();
    slice_index const proxy_other = alloc_proxy_slice();
    slice_index const and = alloc_and_slice(proxy_other,proxy_starter);
    slice_index const notcheck_tester = alloc_goal_notcheck_reached_tester_slice();
    slice_index const immobile_tester_starter = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    slice_index const immobile_tester_other = alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary);

    pipe_link(dblstale_tester,and);

    pipe_link(proxy_starter,notcheck_tester);
    pipe_link(notcheck_tester,immobile_tester_starter);
    pipe_link(immobile_tester_starter,alloc_true_slice());

    pipe_link(proxy_other,immobile_tester_other);
    pipe_link(immobile_tester_other,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,dblstale_tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void prepend_selfcheck_guard(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_selfcheck_guard_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a doublestalemate testing system with a self check guard
 * @param tester STGoalReachedTester slice
 */
void goal_doublestalemate_insert_selfcheck_guard(slice_index tester)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",tester);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  /* no need to instrument the and operand where we test for stalemate of the
   * starting side */
  stip_structure_traversal_override_single(&st,
                                           STGoalNotCheckReachedTester,
                                           stip_structure_visitor_noop);
  stip_structure_traversal_override_single(&st,
                                           STGoalImmobileReachedTester,
                                           prepend_selfcheck_guard);
  stip_traverse_structure(tester,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
