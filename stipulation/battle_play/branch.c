#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "pyselfcg.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/defense_move.h"
#include "trace.h"

#include <assert.h>

/* Allocate a branch consisting mainly of an attack move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_attack_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;
  slice_index attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_selfcheck_guard_attacker_filter(length,min_length);
  attack = alloc_attack_move_slice(length,min_length);
  pipe_link(result,attack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch consisting mainly of an defense move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_proxy_slice();

  {
    slice_index const
        guard1 = alloc_selfcheck_guard_defender_filter(length,min_length);
    slice_index const defense = alloc_defense_move_slice(length,min_length);
    slice_index const
        guard2 = alloc_selfcheck_guard_attacker_filter(length-1,min_length-1);
    pipe_link(result,guard1);
    pipe_link(guard1,defense);
    pipe_link(defense,guard2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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

  assert(length>slack_length_battle);
  assert(min_length>slack_length_battle);
  assert(min_length%2==length%2);

  result = alloc_proxy_slice();

  {
    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const
        guard1 = alloc_selfcheck_guard_defender_filter(length-1,min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const
        guard2 = alloc_selfcheck_guard_attacker_filter(length,min_length);
    pipe_link(result,guard2);
    pipe_link(guard2,attack);
    pipe_link(attack,guard1);
    pipe_link(guard1,defense);
    pipe_link(defense,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a battle slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void battle_branch_shorten_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.branch.length -= 2;
  slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
