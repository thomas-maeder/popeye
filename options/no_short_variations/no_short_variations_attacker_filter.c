#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STNoShortVariations slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_no_short_variations_slice(stip_length_type length,
                                            stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STNoShortVariations,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a short solution after the defense played
 * in a slice
 * @param si identifies slice that just played the defense
 * @param n maximum number of half moves until end of branch
 */
static boolean has_short_solution(slice_index si, stip_length_type n)
{
  boolean result;
  stip_length_type const n_min = battle_branch_calc_n_min(si,n);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(slices[si].u.pipe.next,n,n_min)<=n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type no_short_variations_has_solution_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type no_short_variations_solve_in_n(slice_index si,
                                                stip_length_type n,
                                                stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (n>slack_length_battle+1
      && encore() /* otherwise we are solving threats */
      && has_short_solution(si,n-2))
    result = n_min;
  else
    result = attack_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
