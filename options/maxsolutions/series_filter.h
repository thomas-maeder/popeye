#if !defined(OPTIMISATIONS_MAXSOLUTIONS_SERIES_FILTER_H)
#define OPTIMISATIONS_MAXSOLUTIONS_SERIES_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STMaxSolutionsSeriesFilter
 * stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsSeriesFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_series_filter(void);

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 *         n-1 the previous move has solved the next slice
 */
stip_length_type maxsolutions_series_filter_solve_in_n(slice_index si,
                                                       stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 *         n-1 the previous move has solved the next slice
 */
stip_length_type
maxsolutions_series_filter_has_solution_in_n(slice_index si,
                                             stip_length_type n);

#endif
