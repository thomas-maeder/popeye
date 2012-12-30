#include "optimisations/ohneschach/redundant_immobility_tests.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "debugging/trace.h"

#include <assert.h>

static void remember_goal_immobile(slice_index si, stip_structure_traversal *st)
{
  boolean * const instrumenting_goal_immobile = st->param;
  boolean const save_instrumenting_goal_immobile = *instrumenting_goal_immobile;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const tester = branch_find_slice(STGoalReachedTester,
                                                 slices[si].next2,
                                                 st->context);
    goal_type const goal = slices[tester].u.goal_handler.goal.type;
    assert(tester!=no_slice);
    *instrumenting_goal_immobile = (goal==goal_mate
                                    || goal==goal_stale
                                    || goal==goal_mate_or_stale);
    stip_traverse_structure_binary_operand1(si,st);
    *instrumenting_goal_immobile = save_instrumenting_goal_immobile;
  }

  stip_traverse_structure_binary_operand2(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  fate_dont_know,
  fate_obsolete,
  fate_still_used,
  fate_deallocated
} fate_type;

static fate_type fate[max_nr_slices];

static void optimise_stop(slice_index si, stip_structure_traversal *st)
{
  boolean * const instrumenting_goal_immobile = st->param;
  boolean const save_instrumenting_goal_immobile = *instrumenting_goal_immobile;
  slice_index const condition = slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *instrumenting_goal_immobile = false;

  stip_traverse_structure_children(si,st);

  *instrumenting_goal_immobile = save_instrumenting_goal_immobile;

  TraceValue("%u",condition);
  TraceValue("%u\n",*instrumenting_goal_immobile);
  if (*instrumenting_goal_immobile)
  {
    TraceValue("%u\n",fate[condition]);
    if (fate[condition]==fate_obsolete)
    {
      fate[condition] = fate_deallocated;
      dealloc_slices(condition);
    }
    if (slices[si].tester!=no_slice)
      pipe_substitute(slices[si].tester,alloc_pipe(STOhneschachStopIfCheck));
    pipe_substitute(si,alloc_pipe(STOhneschachStopIfCheck));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_slices(slice_index si)
{
  stip_structure_traversal st;
  boolean instrumenting_goal_immobile = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&instrumenting_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoalImmobile,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoal,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheckAndNotMate,&optimise_stop);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void visit_stop(slice_index si, stip_structure_traversal *st)
{
  boolean * const instrumenting_goal_immobile = st->param;
  boolean const save_instrumenting_goal_immobile = *instrumenting_goal_immobile;
  slice_index const condition = slices[si].next2;
  fate_type save_fate;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *instrumenting_goal_immobile = false;

  stip_traverse_structure_children_pipe(si,st);

  save_fate = fate[condition];
  stip_traverse_structure_conditional_pipe_tester(si,st);
  fate[condition] = save_fate;

  *instrumenting_goal_immobile = save_instrumenting_goal_immobile;

  TraceValue("%u",condition);
  TraceValue("%u\n",*instrumenting_goal_immobile);
  if (*instrumenting_goal_immobile)
  {
    if (fate[condition]==fate_dont_know)
      fate[condition] = fate_obsolete;
  }
  else
    fate[condition] = fate_still_used;

  TraceValue("%u\n",fate[condition]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_slice_fates(slice_index si)
{
  stip_structure_traversal st;
  boolean instrumenting_goal_immobile = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    fate[i] = fate_dont_know;

  stip_structure_traversal_init(&st,&instrumenting_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoalImmobile,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoal,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheckAndNotMate,&visit_stop);
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
  optimise_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_stop_if_check_solve(slice_index si,
                                                stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(slices[si].starter))
    result = slack_length-2;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
