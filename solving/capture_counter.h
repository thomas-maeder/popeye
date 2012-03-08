#if !defined(SOLVING_LEGAL_CAPTURE_COUNTER_H)
#define SOLVING_LEGAL_CAPTURE_COUNTER_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STCaptureCounter stipulation slices.
 */

/* current value of the count */
extern unsigned int capture_counter_count;

/* stop the move iteration once capture_counter_count exceeds this number */
extern unsigned int capture_counter_interesting;

/* Allocate a STCaptureCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_capture_counter_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type capture_counter_attack(slice_index si, stip_length_type n);

#endif
