#if !defined(STIPULATION_GOALS_COUNTERMATE_HELP_FILTER_H)
#define STIPULATION_GOALS_COUNTERMATE_HELP_FILTER_H

#include "pystip.h"

/* This module provides functionality dealing with STCounterMateHelpFilter
 * stipulation slices.
 * Slices of this type make sure that the prerequisites for reaching a
 * countermate are fulfilled (i.e. that the side delivering the countermate
 * is mated before the move delivering it).
 */

/* Allocate a STCounterMateHelpFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_countermate_help_filter_slice(stip_length_type length,
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
stip_length_type countermate_help_filter_solve_in_n(slice_index si,
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
stip_length_type countermate_help_filter_has_solution_in_n(slice_index si,
                                                           stip_length_type n);

#endif
