#if !defined(OPTIMISATIONS_MAXSOLUTIONS_HELP_FILTER_H)
#define OPTIMISATIONS_MAXSOLUTIONS_HELP_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STMaxSolutionsHelpFilter
 * stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsHelpFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_help_filter(void);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type maxsolutions_help_filter_help(slice_index si,
                                               stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type maxsolutions_help_filter_can_help(slice_index si,
                                                   stip_length_type n);

#endif
