#include "pyleaff.h"
#include "pyleaf.h"
#include "pydata.h"
#include "output/output.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with leaf slices that
 * detect whether the defending side has just the goal that it defends
 * against.
 */

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_forced_has_solution(slice_index leaf)
{
  Side const attacker = advers(slices[leaf].starter);
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  switch (leaf_is_goal_reached(attacker,leaf))
  {
    case goal_not_reached_selfcheck:
      result = opponent_self_check;
      break;

    case goal_not_reached:
      result = has_no_solution;
      break;

    case goal_reached:
      result = has_solution;
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
has_solution_type leaf_forced_solve(slice_index leaf)
{
  Side const attacker = advers(slices[leaf].starter);
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  switch (leaf_is_goal_reached(attacker,leaf))
  {
    case goal_not_reached_selfcheck:
      result = opponent_self_check;
      break;

    case goal_not_reached:
      result = has_no_solution;
      break;

    case goal_reached:
      result = has_solution;
      active_slice[nbply] = leaf;
      write_goal(slices[leaf].u.leaf.goal);
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
