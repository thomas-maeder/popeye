#include "stipulation/discriminate_by_right_to_move.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/fork.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STDiscriminateByRightToMove slice
 * @param white successor if White has the right to move
 * @param black successof if Black has the right to move
 */
slice_index alloc_discriminate_by_right_to_move_slice(slice_index white,
                                                      slice_index black)
{
  slice_index const result = alloc_fork_slice(STDiscriminateByRightToMove,black);
  pipe_link(result,white);
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type discriminate_by_right_to_move_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = trait[nbply]==White ? slices[si].next1 : slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(slices[si].starter==White || slices[si].starter==Black);
  result = attack(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
