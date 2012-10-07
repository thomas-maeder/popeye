#if !defined(STIPULATION_DEAD_END_H)
#define STIPULATION_DEAD_END_H

#include "solving/solve.h"

/* This module provides functionality dealing with the defending side
 * in STDeadEnd stipulation slices.
 */

/* Allocate a STDeadEnd defender slice.
 * @return index of allocated slice
 */
slice_index alloc_dead_end_slice(void);

/* Optimise away redundant deadend slices
 * @param si identifies the entry slice
 */
void stip_optimise_dead_end_slices(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type dead_end_solve(slice_index si, stip_length_type n);

#endif
