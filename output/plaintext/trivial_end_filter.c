#include "output/plaintext/trivial_end_filter.h"
#include "pydata.h"
#include "pypipe.h"
#include "trace.h"

/* Used to inform STTrivialEndFilter about when to filter out trivial
 * variations (e.g. short mates in self stipulations if there are defenses that
 * don't deliver mate).
 *
 * Initialise element 1 to true to cause immediate mates in set and postkey only
 * play in self stipulations.
 *
 * This causes some superfluous moves and check tests to be performed in ser-r
 * stipulations, but any "correct" implementation would be significantly more
 * complex.
 */
boolean do_write_trivial_ends[maxply+1] = { false, true };

/* Allocate a STTrivialEndFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_trivial_end_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STTrivialEndFilter);

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
stip_length_type trivial_end_filter_can_attack(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = can_attack(slices[si].u.pipe.next,n,n_max_unsolvable);

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
stip_length_type trivial_end_filter_attack(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (nbply==nil_ply || do_write_trivial_ends[parent_ply[nbply]])
    result = attack(next,n,n_max_unsolvable);
  else
    /* variation is trivial - just determine the result */
    result = can_attack(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
