#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseMoveLegalityChecked defender slice.
 * @return index of allocated slice
 */
slice_index alloc_defense_move_legality_checked_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STDefenseMoveLegalityChecked);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Create the root slices sequence for a battle play branch; shorten
 * the non-root slices by the moves represented by the root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_move_legality_checked_make_root(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_make_root(si,st);

  si = slices[si].u.pipe.next;

  while (slices[si].type!=STDefenseMoveShoeHorningDone)
  {
    battle_branch_shorten_slice(si);
    si = slices[si].u.pipe.next;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
