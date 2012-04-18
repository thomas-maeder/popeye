#include "stipulation/setplay_fork.h"
#include "pystip.h"
#include "pybrafrk.h"
#include "stipulation/has_solution_type.h"

#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STSetplayFork slice
 * @param set entry branch of set play
 * @return newly allocated slice
 */
slice_index alloc_setplay_fork_slice(slice_index set)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",set);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STSetplayFork,set);

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
stip_length_type setplay_fork_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  attack(slices[si].u.fork.fork,length_unspecified);
  result = attack(slices[si].u.fork.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
