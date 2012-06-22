#if !defined(STIPULATION_CIRCE_CIRCUIT_B_SPECIAL_H)
#define STIPULATION_CIRCE_CIRCUIT_B_SPECIAL_H

#include "solving/battle_play/attack_play.h"

/* This module provides slice type STCirceCircuitSpecial - detects
 * circuits by a reborn captured piece
 */

/* Allocate a STCirceCircuitSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_circuit_special_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_circuit_special_attack(slice_index si, stip_length_type n);

#endif
