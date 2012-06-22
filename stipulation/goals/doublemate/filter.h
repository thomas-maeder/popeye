#if !defined(STIPULATION_GOALS_DOUBLEMATE_FILTER_H)
#define STIPULATION_GOALS_DOUBLEMATE_FILTER_H

#include "solving/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STDoubleMateFilter stipulation slices.
 * Slices of this type make sure that the prerequisites for reaching a
 * doublemate are fulfilled (i.e. that the side delivering the doublemate
 * isn't immobile before the move delivering it).
 */

/* Allocate a STDoubleMateFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_doublemate_filter_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type doublemate_filter_attack(slice_index si, stip_length_type n);

#endif
