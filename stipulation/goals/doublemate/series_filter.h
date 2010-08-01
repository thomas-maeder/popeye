#if !defined(STIPULATION_GOALS_DOUBLEMATE_SERIES_FILTER_H)
#define STIPULATION_GOALS_DOUBLEMATE_SERIES_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with
 * STDoubleMateSeriesFilter stipulation slices.
 * Slices of this type make sure that the prerequisites for reaching a
 * doublemate are fulfilled (i.e. that the side delivering the doublemate
 * isn't immobile before the move delivering it).
 */

/* Allocate a STDoubleMateSeriesFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_doublemate_series_filter_slice(stip_length_type length,
                                                 stip_length_type min_length);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type doublemate_series_filter_solve_in_n(slice_index si,
                                                     stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type
doublemate_series_filter_has_solution_in_n(slice_index si,
                                           stip_length_type n);

#endif
