#include "stipulation/move_played.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STAttackPlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_attack_played_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAttackPlayed);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STDefensePlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_defense_played_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STDefensePlayed);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STHelpMovePlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_help_move_played_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STHelpMovePlayed);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_played_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==no_side)
  {
    slice_index const next = SLICE_NEXT1(si);
    stip_traverse_structure_children_pipe(si,st);
    SLICE_STARTER(si) = (SLICE_STARTER(next)==no_side
                          ? no_side
                          : advers(SLICE_STARTER(next)));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
