#if !defined(STIPULATION_SERIES_PLAY_ONLY_LAST_MOVE_H)
#define STIPULATION_SERIES_PLAY_ONLY_LAST_MOVE_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STSeriesOnlyLastMove
 * stipulation slices.
 */

/* Allocate a STSeriesOnlyLastMove slice.
 * @return index of allocated slice
 */
slice_index alloc_series_only_last_move_slice(void);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_only_last_move_solve_in_n(slice_index si,
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
stip_length_type series_only_last_move_has_solution_in_n(slice_index si,
                                                         stip_length_type n);

#endif
