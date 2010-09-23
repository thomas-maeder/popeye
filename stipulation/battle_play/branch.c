#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_find_shortest.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/ready_for_attack.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_move_played.h"
#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "stipulation/battle_play/ready_for_defense.h"
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
  slice_index shortest;
  slice_index attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_ready_for_attack_slice(length,min_length);
  shortest = alloc_attack_find_shortest_slice(length,min_length);
  attack = alloc_attack_move_slice(length,min_length);
  pipe_link(result,shortest);
  pipe_link(shortest,attack);

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

  {
    slice_index const dready = alloc_ready_for_defense_slice(length,
                                                             min_length);
    slice_index const defense = alloc_defense_move_slice(length,min_length);
    slice_index const dplayed = alloc_defense_move_played_slice(length-1,
                                                                min_length-1);
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length-1,
                                                           min_length-1);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length-1,min_length-1);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length-1,min_length-1);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);

    result = dready;
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

  {
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length,min_length);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length,
                                                           min_length);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length,min_length);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length,min_length);
    slice_index const aready = alloc_ready_for_attack_slice(length,min_length);
    slice_index const shortest = alloc_attack_find_shortest_slice(length,
                                                                  min_length);

    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const aplayed = alloc_branch(STAttackMovePlayed,
                                             length-1,min_length-1);
    slice_index const ashoehorned = alloc_branch(STAttackMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const checked = alloc_branch(STAttackMoveLegalityChecked,
                                             length-1,min_length-1);
    slice_index const afiltered = alloc_branch(STAttackMoveFiltered,
                                               length-1,min_length-1);
    slice_index const adealt = alloc_branch(STAttackDealtWith,
                                            length-1,min_length-1);
    slice_index const dready = alloc_ready_for_defense_slice(length-1,
                                                             min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const dplayed = alloc_defense_move_played_slice(length-2,
                                                                min_length-2);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,aready);
    pipe_link(aready,shortest);
    pipe_link(shortest,attack);
    pipe_link(attack,aplayed);
    pipe_link(aplayed,ashoehorned);
    pipe_link(ashoehorned,checked);
    pipe_link(checked,afiltered);
    pipe_link(afiltered,adealt);
    pipe_link(adealt,dready);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);

    result = dchecked;
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
