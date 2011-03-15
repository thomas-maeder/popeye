#include "stipulation/goals/countermate/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a counter mate goal has just been reached
 */

extern boolean are_we_testing_immobility_with_opposite_king_en_prise;

/* Allocate a STGoalCounterMateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_countermate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalCounterMateReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_countermate_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const just_moved = advers(starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",parent_ply[nbply]);
  if (TSTFLAG(goal_preprequisites_met[parent_ply[nbply]],goal_countermate)
      && echecc(nbply,starter) && echecc(nbply,just_moved))
  {
    are_we_testing_immobility_with_opposite_king_en_prise =
      (TSTFLAG(PieSpExFlags,Neutral)) && rb!=initsquare && TSTFLAG(spec[rb],Neutral);
    result = slice_has_solution(next);
    are_we_testing_immobility_with_opposite_king_en_prise = false;
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_countermate_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const starter = slices[si].starter;
  Side const just_moved = advers(starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",parent_ply[nbply]);
  if (TSTFLAG(goal_preprequisites_met[parent_ply[nbply]],goal_countermate)
      && echecc(nbply,starter) && echecc(nbply,just_moved))
  {
    are_we_testing_immobility_with_opposite_king_en_prise =
      (TSTFLAG(PieSpExFlags,Neutral)) && rb!=initsquare && TSTFLAG(spec[rb],Neutral);
    result = slice_solve(next);
    are_we_testing_immobility_with_opposite_king_en_prise = false;
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
