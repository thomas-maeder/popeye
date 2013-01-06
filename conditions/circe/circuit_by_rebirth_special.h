#if !defined(STIPULATION_CIRCE_CIRCUIT_B_SPECIAL_H)
#define STIPULATION_CIRCE_CIRCUIT_B_SPECIAL_H

#include "solving/solve.h"

/* This module provides slice type STCirceCircuitSpecial - detects
 * circuits by a reborn captured piece
 */

/* Allocate a STCirceCircuitSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_circuit_special_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_circuit_special_solve(slice_index si, stip_length_type n);

#endif
