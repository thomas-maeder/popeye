#if !defined(OPTIMISATIONS_STOPONSHORTSOLUTIONS_HELP_FILTER_H)
#define OPTIMISATIONS_STOPONSHORTSOLUTIONS_HELP_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with
 * STStopOnShortSolutionsHelpFilter stipulation slice type.
 * Slices of this type make sure that solving stops after a short solution
 * solution has been found
 */

/* Allocate a STStopOnShortSolutionsHelpFilter slice.
 * @param length full length 
 * @param length minimum length 
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_help_filter(stip_length_type length,
                                                   stip_length_type min_length);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type
stoponshortsolutions_help_filter_solve_in_n(slice_index si,
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
stip_length_type
stoponshortsolutions_help_filter_has_solution_in_n(slice_index si,
                                                   stip_length_type n);

#endif
