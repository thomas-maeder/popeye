#include "pyleafh.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyleaf.h"
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

  assert(slices[leaf].starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  generate_move_reaching_goal(leaf,side_at_move);

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

/* Determine and find final moves of a help leaf
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean leaf_h_solve_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",side_at_move);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,side_at_move);

  --MovesLeft[side_at_move];

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !(isIntelligentModeActive && !isGoalReachable())
        && leaf_is_goal_reached(side_at_move,leaf)==goal_reached)
    {
      final_move_found = true;
      write_final_help_move(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  ++MovesLeft[side_at_move];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",final_move_found);
  TraceFunctionResultEnd();
  return final_move_found;
}

/* Solve the final move for a countermate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_cmate_solve_final_move(slice_index leaf)
{
  boolean result = false;
  Side const just_moved = advers(slices[leaf].starter);
  Side const side_at_move = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (goal_checker_mate(just_moved)==goal_reached)
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,side_at_move);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(side_at_move,leaf)==goal_reached)
      {
        result = true;
        write_final_help_move(goal_countermate);
      }
      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve the final move for a doublemate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_dmate_solve_final_move(slice_index leaf)
{
  boolean result = false;
  Side const side_at_move = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (!immobile(side_at_move))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,side_at_move);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(side_at_move,leaf)==goal_reached)
      {
        result = true;
        write_final_help_move(goal_doublemate);
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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

  init_output(leaf);

  isIntelligentModeActive = false;
  result = leaf_h_solve(leaf);
  write_end_of_solution_phase();
  isIntelligentModeActive = save_isIntelligentModeActive;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution of a help leaf slice.
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_h_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_h_cmate_solve_final_move(leaf);
      break;

    case goal_doublemate:
      result = leaf_h_dmate_solve_final_move(leaf);
      break;

    default:
      result = leaf_h_solve_final_move(leaf);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_h_defend(slice_index leaf)
{
  boolean result;
  Side const starter = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (is_end_in_1_possible(starter,leaf))
  {
    result = false;
    leaf_h_solve_final_move(leaf);
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_h_detect_starter(slice_index si, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
