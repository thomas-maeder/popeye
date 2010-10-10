#include "stipulation/goals/doublemate/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a double mate goal has just been reached
 */

boolean are_we_testing_immobility_with_opposite_king_en_prise = false;

/* Allocate a STGoalDoubleMateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_doublemate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalDoubleMateReachedTester);
  slices[result].u.goal_reached_tester.goal.type = goal_doublemate;

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
has_solution_type goal_doublemate_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.goal_reached_tester.next;
  Side const starter = slices[si].starter;
  Side const just_moved = advers(starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,starter) && echecc(nbply,just_moved))
  {
    boolean both_immobile;

    are_we_testing_immobility_with_opposite_king_en_prise =
      (TSTFLAG(PieSpExFlags,Neutral)) && rb!=initsquare && TSTFLAG(spec[rb],Neutral);
    /* modified to allow isardam + ##  */
    /* may still have problem with isardam + nK + ##  !*/
    both_immobile = immobile(starter) && immobile(just_moved);
    are_we_testing_immobility_with_opposite_king_en_prise = false;
    if (both_immobile)
      result = slice_has_solution(next);
    else
      result = has_no_solution;
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
has_solution_type goal_doublemate_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.goal_reached_tester.next;
  Side const starter = slices[si].starter;
  Side const just_moved = advers(starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,starter) && echecc(nbply,just_moved))
  {
    boolean both_immobile;

    are_we_testing_immobility_with_opposite_king_en_prise =
      (TSTFLAG(PieSpExFlags,Neutral)) && rb!=initsquare && TSTFLAG(spec[rb],Neutral);
    /* modified to allow isardam + ##  */
    /* may still have problem with isardam + nK + ##  !*/
    both_immobile = immobile(starter) && immobile(just_moved);
    are_we_testing_immobility_with_opposite_king_en_prise = false;
    if (both_immobile)
      result = slice_solve(next);
    else
      result = has_no_solution;
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
