#if !defined(CONDITIONS_ANTICIRCE_TARGET_SQUARE_FILTER_H)
#define CONDITIONS_ANTICIRCE_TARGET_SQUARE_FILTER_H

#include "solving/battle_play/attack_play.h"

/* This module provides slice type STAnticirceTargetSquareFilter - tests
 * additional conditions on some goals imposed by Anticirce:
 * - the mated side must have >=1 move that only fails because of self-check
 */

/* Allocate a STAnticirceTargetSquareFilter slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_anticirce_target_square_filter_slice(square target);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_target_square_filter_attack(slice_index si, stip_length_type n);

#endif
