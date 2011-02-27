#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "pypipe.h"
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
