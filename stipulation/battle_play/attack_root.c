#include "stipulation/battle_play/attack_root.h"
#include "pydata.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STAttackRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_root_slice(stip_length_type length,
                                    stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert((length%2)==(min_length%2));

  result = alloc_branch(STAttackRoot,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type attack_root_solve(slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type min_length = slices[si].u.branch.min_length;
  has_solution_type result = has_no_solution;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length==slack_length_battle+1
      && !are_prerequisites_for_reaching_goal_met(imminent_goal.type,attacker))
    min_length = slack_length_battle+3;

  if (min_length<=length)
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    if (length<=slack_length_battle+1
        && slices[si].u.branch.imminent_goal.type!=no_goal)
    {
      empile_for_goal = slices[si].u.branch.imminent_goal;
      generate_move_reaching_goal(attacker);
      empile_for_goal.type = no_goal;
    }
    else
      genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      {
        stip_length_type const
            nr_moves_needed = defense_defend_in_n(next,
                                                  length-1,min_length-1,
                                                  min_length-3);
        if (min_length-1<=nr_moves_needed)
        {
          if (nr_moves_needed<=length-1)
            result = has_solution;
        }
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void attack_root_reduce_to_postkey_play(slice_index si,
                                        stip_structure_traversal *st)
{
  slice_index const *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  if (*postkey_slice!=no_slice)
    dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
