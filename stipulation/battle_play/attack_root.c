#include "stipulation/battle_play/attack_root.h"
#include "pydata.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_move.h"
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

  result = alloc_branch(STAttackRoot,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_root_solve_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>slack_length_battle);

  if (n_max_unsolvable<length)
  {
    move_generation_mode = move_generation_not_optimized;
    if (length==slack_length_battle+1 && imminent_goal.type!=no_goal)
    {
      empile_for_goal = imminent_goal;
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
                                                  length-1,
                                                  n_max_unsolvable-1)+1;
        if (result>nr_moves_needed)
          result = nr_moves_needed;
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
