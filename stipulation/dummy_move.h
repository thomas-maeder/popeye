#if !defined(STIPULATION_DUMMY_MOVE_H)
#define STIPULATION_DUMMY_MOVE_H

#include "solving/solve.h"

/* Slices of type STDummyMove are used to represent the "dummy move" not
 * played by the side not playing the series.
 */

/* Allocate a STDummyMove slice.
 * @return index of allocated slice
 */
slice_index alloc_dummy_move_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type dummy_move_solve(slice_index si, stip_length_type n);

#endif
