#include "stipulation/move_inverter.h"
#include "pystip.h"
#include "stipulation/battle_play/attack_play.h"
#include "pypipe.h"
#include "debugging/trace.h"
#ifdef _SE_
#include "se.h"
#endif

/* Allocate a STMoveInverter slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMoveInverter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_inverter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(slices[si].next1,n);

#ifdef _SE_DECORATE_SOLUTION_
  se_end_set_play();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_inverter_detect_starter(slice_index si, stip_structure_traversal *st)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].next1;
    Side next_starter;
    stip_traverse_structure_children(si,st);
    next_starter = slices[next].starter;
    if (next_starter!=no_side)
      slices[si].starter = (next_starter==no_side
                            ? no_side
                            : advers(next_starter));
  }

  TraceValue("->%u\n",slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
