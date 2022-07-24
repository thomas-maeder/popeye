#include "optimisations/ohneschach/redundant_immobility_tests.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/has_solution_type.h"
#include "optimisations/ohneschach/stop_if_check.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void remember_goal_immobile(slice_index si, stip_structure_traversal *st)
{
  boolean * const instrumenting_goal_immobile = st->param;
  boolean const save_instrumenting_goal_immobile = *instrumenting_goal_immobile;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const tester = branch_find_slice(STGoalReachedTester,
                                                 SLICE_NEXT2(si),
                                                 st->context);
    goal_type goal;
    assert(tester!=no_slice);
    goal = SLICE_U(tester).goal_handler.goal.type;
    *instrumenting_goal_immobile = (goal==goal_mate
                                    || goal==goal_stale
                                    || goal==goal_mate_or_stale);
    TraceValue("->%u",*instrumenting_goal_immobile);
    TraceEOL();
    stip_traverse_structure_binary_operand1(si,st);
    *instrumenting_goal_immobile = save_instrumenting_goal_immobile;
  }

  stip_traverse_structure_binary_operand2(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_goal_immobile(slice_index si, stip_structure_traversal *st)
{
  boolean * const instrumenting_goal_immobile = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* current value of *instrumenting_goal_immobile has been consumed */
  *instrumenting_goal_immobile = false;

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void plan_optimisation(slice_index si, stip_structure_traversal *st)
{
  boolean const * const instrumenting_goal_immobile = st->param;
  boolean const to_be_optimised = *instrumenting_goal_immobile;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ohneschach_stop_if_check_plan_to_optimise_away_stop(si,to_be_optimised);
  forget_goal_immobile(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_slice_fates(slice_index si)
{
  stip_structure_traversal st;
  boolean instrumenting_goal_immobile = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&instrumenting_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoalImmobile,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoal,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheck,&forget_goal_immobile);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheckAndNotMate,&plan_optimisation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise away redundant immobility tests
 * @param si identifies root slice of stipulation
 */
void ohneschach_optimise_away_redundant_immobility_tests(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  determine_slice_fates(si);
  ohneschach_stop_if_check_execute_optimisations(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
