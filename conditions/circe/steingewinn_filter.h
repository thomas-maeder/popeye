#if !defined(STIPULATION_CIRCE_STEINGEWINN_FILTER_H)
#define STIPULATION_CIRCE_STEINGEWINN_FILTER_H

#include "solving/battle_play/attack_play.h"

/* This module provides slice type STCirceSteingewinnFilter - detects whether a
 *  capture isn't a Steingewinn because the captured piece is reborn
 */

/* Allocate a STCirceSteingewinnFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_steingewinn_filter_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_steingewinn_filter_attack(slice_index si, stip_length_type n);

#endif
