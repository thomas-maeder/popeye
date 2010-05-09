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
    slice_index const proxy = alloc_proxy_slice();
    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const
        guard1 = alloc_selfcheck_guard_defender_filter(length-1,min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const
        guard2 = alloc_selfcheck_guard_attacker_filter(length,min_length);
    pipe_link(proxy,guard2);
    pipe_link(guard2,attack);
    pipe_link(attack,guard1);
    pipe_link(guard1,defense);
    pipe_link(defense,proxy);

    pipe_set_successor(result,proxy);
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

  if (n+min_length>slack_length_battle+1+length)
    result = n-(length-min_length);
  else
    result = slack_length_battle+2-parity;

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
  if (slices[si].u.branch.min_length>=slack_length_battle+2)
    slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
