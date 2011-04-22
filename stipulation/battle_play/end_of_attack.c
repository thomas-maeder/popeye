#include "stipulation/battle_play/end_of_attack.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STEndOfAttack slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_attack_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEndOfAttack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
