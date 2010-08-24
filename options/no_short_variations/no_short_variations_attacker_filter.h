#if !defined(OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_ATTACKER_FILTER_H)
#define OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_ATTACKER_FILTER_H

#include "boolean.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STNoShortVariations stipulation slices.
 */

/* Allocate a STNoShortVariations slice.
 * @return index of allocated slice
 */
slice_index alloc_no_short_variations_slice(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
no_short_variations_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
no_short_variations_solve_in_n(slice_index si,
                               stip_length_type n,
                               stip_length_type n_max_unsolvable);

#endif
