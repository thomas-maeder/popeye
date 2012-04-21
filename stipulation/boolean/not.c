#include "stipulation/boolean/not.h"
#include "pystip.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/attack_play.h"
#include "pypipe.h"
#include "pyproc.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a not slice.
 * @return index of allocated slice
 */
slice_index alloc_not_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STNot);

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
stip_length_type not_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type next_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  next_result = attack(slices[si].next1,n);
  if (next_result>n)
    result = n;
  else if (next_result>=slack_length)
    result = n+2;
  else
    result = next_result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
