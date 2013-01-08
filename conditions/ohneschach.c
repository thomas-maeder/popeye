#include "conditions/ohneschach.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "solving/recursion_stopper.h"
#include "debugging/trace.h"

#include <assert.h>

boolean ohneschach_undecidable_goal_detected[maxply+1];

static slice_index alloc_immobility_test_branch(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STImmobilityTester);
  link_to_branch(result,alloc_defense_branch(slack_length+1,slack_length+1));

  {
    slice_index const prototype = alloc_recursion_stopper_slice();
    branch_insert_slices(result,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_stop_on_check(slice_index si, stip_structure_traversal *st)
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

static structure_traversers_visitor avoid_temporary_hacks[] =
{
  { STBrunnerDefenderFinder,                  &stip_traverse_structure_children_pipe },
  { STKingCaptureLegalityTester,              &stip_traverse_structure_children_pipe },
  { STTakeMakeCirceCollectRebirthSquaresFork, &stip_traverse_structure_children_pipe },
  { STCastlingIntermediateMoveLegalityTester, &stip_traverse_structure_children_pipe },
  { STSATFlightsCounterFork,                  &stip_traverse_structure_children_pipe },
  { STUltraMummerMeasurerFork,                &stip_traverse_structure_children_pipe }
};

enum
{
  nr_avoid_temporary_hacks = (sizeof avoid_temporary_hacks
                              / sizeof avoid_temporary_hacks[0])
};

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
  stip_structure_traversal_override_single(&st,STNotEndOfBranchGoal,&insert_stop_on_check);
  stip_structure_traversal_override(&st,
                                    avoid_temporary_hacks,
                                    nr_avoid_temporary_hacks);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type ohneschach_stop_if_check_and_not_mate_solve(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(slices[si].starter))
  {
    ohneschach_undecidable_goal_detected[nbply+1] = false;

    switch (solve(slices[si].next2,length_unspecified))
    {
      case previous_move_is_illegal:
        ohneschach_undecidable_goal_detected[nbply] = recursion_stopped;
        TraceValue("%u",nbply);
        TraceValue("%u\n",ohneschach_undecidable_goal_detected[nbply]);
        recursion_stopped = false;
        result = n;
        break;

      case next_move_has_no_solution:
        result = previous_move_is_illegal;
        break;

      case next_move_has_solution:
        ohneschach_undecidable_goal_detected[nbply] = ohneschach_undecidable_goal_detected[nbply+1];
        TraceValue("%u",nbply);
        TraceValue("%u\n",ohneschach_undecidable_goal_detected[nbply]);
        result = n;
        break;

      case immobility_on_next_move:
        result = solve(slices[si].next1,n);
        break;

      default:
        assert(0);
        break;
    }
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
