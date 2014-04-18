#include "stipulation/move_inverter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
    stip_traverse_structure_children_pipe(si,st);
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
