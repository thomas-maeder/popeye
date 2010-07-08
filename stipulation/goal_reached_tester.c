#include "stipulation/goal_reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a goal has just been reached
 */

/* Determine whether a side has reached the goal of a leaf slice.
 * @param camp side
 * @param goal goal to be tested
 * @return whether camp has reached leaf's goal
 */
goal_checker_result_type is_goal_reached(Side just_moved, Goal goal)
{
  boolean result = goal_not_reached;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,just_moved,"");
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_mate:
      if (CondFlag[blackultraschachzwang]
          || CondFlag[whiteultraschachzwang])
        result = goal_checker_mate_ultraschachzwang(just_moved);
      else
        result = goal_checker_mate(just_moved);
      break;

    case goal_stale:
      result = goal_checker_stale(just_moved);
      break;

    case goal_dblstale:
      result = goal_checker_dblstale(just_moved);
      break;

    case goal_target:
      assert(goal.target!=initsquare);
      result = goal_checker_target(just_moved,goal.target);
      break;

    case goal_check:
      result = goal_checker_check(just_moved);
      break;

    case goal_capture:
      result = goal_checker_capture(just_moved);
      break;

    case goal_steingewinn:
      result = goal_checker_steingewinn(just_moved);
      break;

    case goal_ep:
      result = goal_checker_ep(just_moved);
      break;

    case goal_doublemate:
    case goal_countermate:
      result = goal_checker_doublemate(just_moved);
      break;

    case goal_castling:
      result = goal_checker_castling(just_moved);
      break;

    case goal_autostale:
      result = goal_checker_autostale(just_moved);
      break;

    case goal_circuit:
      result = goal_checker_circuit(just_moved);
      break;

    case goal_exchange:
      result = goal_checker_exchange(just_moved);
      break;

    case goal_circuitB:
      result = goal_checker_circuitB(just_moved);
      break;

    case goal_exchangeB:
      result = goal_checker_exchangeB(just_moved);
      break;

    case goal_any:
      result = goal_checker_any(just_moved);
      break;

    case goal_proof:
    case goal_atob:
      result = goal_checker_proof(just_moved);
      break;

    case goal_mate_or_stale:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(goal_checker_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalReachedTester slice.
 * @param type goal to be tested
 * @return index of allocated slice
 */
slice_index alloc_goal_reached_tester_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalReachedTester);
  slices[result].u.goal_reached_tester.goal = goal;

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
has_solution_type goal_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
      result = opponent_self_check;
      break;

    case goal_not_reached:
      result = has_no_solution;
      break;

    case goal_reached:
      result = slice_has_solution(next);
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
      result = opponent_self_check;
      break;

    case goal_not_reached:
      result = has_no_solution;
      break;

    case goal_reached:
      result = slice_solve(next);
      if (result==has_solution)
        write_goal(goal.type);
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void goal_reached_tester_insert_root(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  slice_index copy;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  copy = copy_slice(si);
  pipe_link(copy,*root);
  *root = copy;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
