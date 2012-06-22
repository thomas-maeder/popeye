#if !defined(CONDITIONS_ANTICIRCE_EXCHANGE_FILTER_H)
#define CONDITIONS_ANTICIRCE_EXCHANGE_FILTER_H

#include "solving/battle_play/attack_play.h"

/* This module provides slice type STAnticirceExchangeFilter - prevent
 * move sequences terminated by a capture from accidentally passing the orthodox
 * test for the exchange goal.
 */

/* Allocate a STAnticirceExchangeFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_exchange_filter_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_exchange_filter_attack(slice_index si, stip_length_type n);

#endif
