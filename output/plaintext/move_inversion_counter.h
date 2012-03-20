#if !defined(OUTPUT_PLAINTEXT_MOVE_INVERSION_COUNTER_H)
#define OUTPUT_PLAINTEXT_MOVE_INVERSION_COUNTER_H

#include "stipulation/battle_play/attack_play.h"

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_move_inversion_counter_attack(slice_index si,
                                               stip_length_type n);

#endif
