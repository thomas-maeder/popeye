#if !defined(OPTIMISATIONS_MAXSOLUTIONS_ROOT_DEFENDER_FILTER_H)
#define OPTIMISATIONS_MAXSOLUTIONS_ROOT_DEFENDER_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with
 * STMaxSolutionsRootDefenderFilter stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsRootDefenderFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_root_defender_filter(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
maxsolutions_root_defender_filter_defend(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable);

#endif
