#include "pyleafh.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyleaf.h"
#include "output/output.h"
#include "pyoutput.h"

#include <assert.h>
#include <stdlib.h>


/* Determine whether a side has an end in 1.
 * @param side_at_move
 * @param leaf slice index
 * @return true iff side_at_move can end in 1 move
 */
static boolean is_end_in_1_possible(Side side_at_move, slice_index leaf)
{
  boolean end_found = false;
  Goal const goal = slices[leaf].u.leaf.goal;

  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (are_prerequisites_for_reaching_goal_met(goal,side_at_move))
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    empile_for_goal = goal;
    empile_for_target = slices[leaf].u.leaf.target;
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(side_at_move);
    empile_for_goal = no_goal;

    --MovesLeft[side_at_move];

    while (encore() && !end_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && (!isIntelligentModeActive || isGoalReachable())
          && leaf_is_goal_reached(side_at_move,leaf)==goal_reached)
      {
        end_found = true;
        coupfort();
      }

      repcoup();

      if (hasMaxtimeElapsed())
        break;
    }

    ++MovesLeft[side_at_move];

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",end_found);
  TraceFunctionResultEnd();
  return end_found;
}

/* Determine whether there is a solution in a help leaf.
 * @param leaf slice index of leaf slice
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_h_has_solution(slice_index leaf)
{
  has_solution_type result = has_no_solution;
  Side const starter = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (is_end_in_1_possible(starter,leaf))
  {
    result = has_solution;
    coupfort();
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution of a leaf slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_h_root_solve(slice_index leaf)
{
  boolean const save_isIntelligentModeActive = isIntelligentModeActive;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  isIntelligentModeActive = false;
  result = leaf_h_solve(leaf)==has_solution;
  if (result)
    write_end_of_solution_phase();
  isIntelligentModeActive = save_isIntelligentModeActive;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_h_solve(slice_index leaf)
{
  has_solution_type result = has_no_solution;
  Side const side_at_move = slices[leaf].starter;
  Goal const goal = slices[leaf].u.leaf.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (are_prerequisites_for_reaching_goal_met(goal,side_at_move))
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = leaf;
    empile_for_goal = goal;
    empile_for_target = slices[leaf].u.leaf.target;
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(side_at_move);
    empile_for_goal = no_goal;

    --MovesLeft[side_at_move];

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !(isIntelligentModeActive && !isGoalReachable())
          && leaf_is_goal_reached(side_at_move,leaf)==goal_reached)
      {
        result = has_solution;
        write_battle_move();
        write_goal(slices[leaf].u.leaf.goal);
      }

      repcoup();
    }

    ++MovesLeft[side_at_move];

    finply();
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_h_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
