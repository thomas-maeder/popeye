#include "stipulation/battle_play/defense_move_played.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseMovePlayed defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_move_played_slice(stip_length_type length,
                                            stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDefenseMovePlayed,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void defense_move_played_make_setplay_slice(slice_index si,
                                            stip_structure_traversal *st)
{
  slice_index * const result = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>=slack_length_battle)
  {
    stip_length_type const length_h = (length+1-slack_length_battle
                                       +slack_length_help);
    *result = alloc_help_move_slice(length_h,length_h-1);
    pipe_set_successor(*result,slices[si].u.branch.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
