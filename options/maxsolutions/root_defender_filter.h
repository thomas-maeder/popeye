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

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
maxsolutions_root_defender_filter_defend(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min,
                                         unsigned int max_nr_refutations);

#endif
