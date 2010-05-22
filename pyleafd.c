#include "pyleafd.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyoutput.h"
#include "pyleaf.h"
#include "pyoutput.h"
#include "optimisations/maxsolutions/maxsolutions.h"

#include <assert.h>
#include <stdlib.h>

/* Determine whether there is a solution in a direct leaf.
 * @param leaf slice index of leaf slice
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_d_has_solution(slice_index leaf)
{
  has_solution_type result = has_no_solution;
  Side const attacker = slices[leaf].starter;
  Goal const goal = slices[leaf].u.leaf.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (are_prerequisites_for_reaching_goal_met(goal,attacker))
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    TraceValue("->%u\n",move_generation_mode);
    empile_for_goal = goal;
    empile_for_target = slices[leaf].u.leaf.target;
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(attacker);
    empile_for_goal = no_goal;

    while (encore() && result==has_no_solution)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf)==goal_reached)
      {
        result = has_solution;
        coupfort();
      }

      repcoup();

      if (hasMaxtimeElapsed())
        break;
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write keys at root level
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
boolean leaf_d_root_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  Goal const goal = slices[leaf].u.leaf.goal;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  output_start_continuation_level(leaf);

  if (are_prerequisites_for_reaching_goal_met(goal,attacker))
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = leaf;
    empile_for_goal = goal;
    empile_for_target = slices[leaf].u.leaf.target;
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(attacker);
    empile_for_goal = no_goal;

    reset_nr_found_solutions_per_phase();

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf)==goal_reached)
      {
        result = true;
        write_battle_move();
        write_goal(slices[leaf].u.leaf.goal);
        write_battle_move_decoration(nbply,attack_key);
        write_end_of_solution();
      }

      repcoup();

      if (max_nr_solutions_found_in_phase())
        break;
    }

    finply();
  }

  output_end_continuation_level();

  if (result)
    write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_d_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  Goal const goal = slices[leaf].u.leaf.goal;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  output_start_continuation_level(leaf);

  if (are_prerequisites_for_reaching_goal_met(goal,attacker))
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = leaf;
    empile_for_goal = goal;
    empile_for_target = slices[leaf].u.leaf.target;
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(attacker);
    empile_for_goal = no_goal;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf)==goal_reached)
      {
        result = true;
        write_battle_move();
        write_goal(slices[leaf].u.leaf.goal);
      }

      repcoup();
    }

    finply();
  }

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_d_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
