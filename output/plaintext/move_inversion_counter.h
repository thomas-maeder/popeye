#if !defined(OUTPUT_PLAINTEXT_MOVE_INVERSION_COUNTER_H)
#define OUTPUT_PLAINTEXT_MOVE_INVERSION_COUNTER_H

#include "solving/solve.h"

/* This module provides the STOutputPlaintextMoveInversionCounter
 * slice type. Slices of this type count the move inversions occuring
 * throughout the play.
 */

/* Number of move inversions up to the current move.
 * Exposed for read-only access only
 */
extern unsigned int output_plaintext_nr_move_inversions;


/* Allocate a STOutputPlaintextMoveInversionCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_move_inversion_counter_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type
output_plaintext_move_inversion_counter_solve(slice_index si,
                                               stip_length_type n);

#endif
