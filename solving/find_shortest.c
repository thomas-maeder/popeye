#include "solving/find_shortest.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STFindShortest slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_find_shortest_slice(stip_length_type length,
                                      stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STFindShortest,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_shortest_can_attack(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n_min = (n_max_unsolvable<slack_length_battle
                                  ? slack_length_battle+1
                                  : n_max_unsolvable+1);
  stip_length_type n_current;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  for (n_current = n_min+(n-n_min)%2; n_current<=n; n_current += 2)
  {
    result = can_attack(next,n_current,n_max_unsolvable);
    if (result<=n_current)
      break;
    else
      n_max_unsolvable = n_current;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_shortest_solve_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n_min = (n_max_unsolvable<slack_length_battle
                                  ? slack_length_battle+1
                                  : n_max_unsolvable+1);
  stip_length_type n_current;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  for (n_current = n_min+(n-n_min)%2; n_current<=n; n_current += 2)
  {
    result = attack(next,n_current,n_max_unsolvable);
    if (result<=n_current)
      break;
    else
      n_max_unsolvable = n_current;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type find_shortest_help(slice_index si, stip_length_type n)
{
  stip_length_type result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result += (n-result)%2;

  while (result<=n)
    if (help(slices[si].u.pipe.next,result)==result)
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type find_shortest_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result += (n-result)%2;

  while (result<=n)
    if (can_help(slices[si].u.pipe.next,result)==result)
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
