#if !defined(OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_ATTACKER_FILTER_H)
#define OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_ATTACKER_FILTER_H

#include "utilities/boolean.h"
#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STNoShortVariations stipulation slices.
 */

/* Allocate a STNoShortVariations slice.
 * @return index of allocated slice
 */
slice_index alloc_no_short_variations_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type no_short_variations_attack(slice_index si, stip_length_type n);

#endif
