#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "pyselfcg.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/defense_move.h"
#include "trace.h"

#include <assert.h>

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert(length>=slack_length_battle);
  assert(min_length>=slack_length_battle);
  assert(min_length%2==length%2);

  result = alloc_proxy_slice();

  if (length==slack_length_battle)
  {
    slice_index const defense = alloc_defense_move_slice(slack_length_battle,
                                                         slack_length_battle);
    slice_index const
        guard = alloc_selfcheck_guard_attacker_filter(slack_length_battle-1,
                                                      slack_length_battle-1);
    pipe_link(result,defense);
    pipe_link(defense,guard);
    pipe_link(guard,result);
  }
  else
  {
    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const
        guard1 = alloc_selfcheck_guard_defender_filter(length-1,min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const
        guard2 = alloc_selfcheck_guard_attacker_filter(length-2,min_length-2);
    pipe_link(result,attack);
    pipe_link(attack,guard1);
    pipe_link(guard1,defense);
    pipe_link(defense,guard2);
    pipe_link(guard2,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Calculate the minimum number of moves to solve for depending of the
 * maximum number and the minimum and maximum length of a slice
 * @param si identifies battle play branch slice
 * @param n maximum number of moves
 * @return minimum number of moves
 */
stip_length_type battle_branch_calc_n_min(slice_index si, stip_length_type n)
{
  stip_length_type const parity = (n-slack_length_battle)%2;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n+min_length>slack_length_battle+length)
    result = n-(length-min_length);
  else
    result = slack_length_battle-parity;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
