#include "conditions/ohneschach.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/battle_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

static slice_index alloc_immobility_test_branch(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STImmobilityTester);
  link_to_branch(result,alloc_defense_branch(slack_length+1,slack_length+1));
  slice_index const prototype = alloc_pipe(STRecursionStopper);
  branch_insert_slices(result,&prototype,1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_stop(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const prototype0 = alloc_conditional_pipe(STOhneschachStopIfCheckAndNotMate,proxy1);
    slice_index const prototype1 = alloc_conditional_pipe(STOhneschachStopIfCheckAndNotMate,proxy2);
    slice_index const prototype2 = alloc_conditional_pipe(STOhneschachStopIfCheckAndNotMate,proxy1);
    slice_index const tester1 = alloc_immobility_test_branch();
    slice_index const not_goal1 = branch_find_slice(STNotEndOfBranchGoal,tester1,stip_traversal_context_intro);
    slice_index const tester2 = alloc_immobility_test_branch();
    slice_index const not_goal2 = branch_find_slice(STNotEndOfBranchGoal,tester2,stip_traversal_context_intro);

    assert(not_goal1!=no_slice);
    assert(not_goal2!=no_slice);

    link_to_branch(proxy1,tester1);
    link_to_branch(proxy2,tester2);
    branch_insert_slices_contextual(si,st->context,&prototype0,1);
    defense_branch_insert_slices(not_goal1,&prototype1,1);
    defense_branch_insert_slices(not_goal2,&prototype2,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void ohneschach_insert_check_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STNotEndOfBranchGoal,&insert_stop);
  stip_structure_traversal_override_single(&st,STBrunnerDefenderFinder,&stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STKingCaptureLegalityTester,&stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STTakeMakeCirceCollectRebirthSquaresFork,&stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STCastlingIntermediateMoveLegalityTester,&stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STSATFlightsCounterFork,&stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STUltraMummerMeasurerFork,&stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean immobile(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* ohneschach_stop_if_check_solve() may invoke itself recursively. Protect
   * ourselves from infinite recursion. */
  if (nbply>250)
    FtlMsg(ChecklessUndecidable);

  result = solve(slices[si].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_stop_if_check_and_not_mate_solve(slice_index si,
                                                             stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(slices[si].starter) && !immobile(si))
    result = slack_length-2;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
