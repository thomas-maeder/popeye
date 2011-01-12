#if !defined(STIPULATION_SERIES_PLAY_DUMMY_MOVE_H)
#define STIPULATION_SERIES_PLAY_DUMMY_MOVE_H

#include "pyslice.h"

/* Slices of type STSeriesDummyMove are used to represent the "dummy move" not
 * played by the side not playing the series.
 */

/* Allocate a STSeriesDummyMove slice.
 * @return index of allocated slice
 */
slice_index alloc_series_dummy_move_slice(void);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_dummy_move_solve_in_n(slice_index si,
                                              stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_dummy_move_has_solution_in_n(slice_index si,
                                                     stip_length_type n);

#endif
