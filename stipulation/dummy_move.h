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
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type dummy_move_solve(slice_index si, stip_length_type n);

#endif
