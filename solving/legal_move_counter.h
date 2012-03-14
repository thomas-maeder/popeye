#if !defined(SOLVING_LEGAL_MOVE_COUNTER_H)
#define SOLVING_LEGAL_MOVE_COUNTER_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STLegalMoveCounter stipulation slices.
 */

/* current value of the count */
extern unsigned int legal_move_counter_count[maxply];

/* stop the move iteration once legal_move_counter_count exceeds this number */
extern unsigned int legal_move_counter_interesting[maxply];

/* Allocate a STLegalMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_move_counter_slice(void);

/* Allocate a STAnyMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_any_move_counter_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type legal_move_counter_attack(slice_index si, stip_length_type n);

#endif
