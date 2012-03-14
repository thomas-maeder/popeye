#if !defined(SOLVING_BATTLE_PLAY_CHECK_DETECTOR_H)
#define SOLVING_BATTLE_PLAY_CHECK_DETECTOR_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality around STCheckDetector slices.
 */

/* Does the attack just played give check?
 * Exposed for read-only access only */
extern boolean attack_gives_check[maxply+1];

/* Allocate a STCheckDetector defender slice.
 * @return index of allocated slice
 */
slice_index alloc_check_detector_slice(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type check_detector_defend(slice_index si, stip_length_type n);

#endif
