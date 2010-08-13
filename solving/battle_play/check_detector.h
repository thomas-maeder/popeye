#if !defined(STIPULATION_BATTLE_PLAY_CHECK_DETECTOR_H)
#define STIPULATION_BATTLE_PLAY_CHECK_DETECTOR_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality around STCheckDetector slices.
 */

/* Does the attack just played give check?
 * Exposed for read-only access only */
extern boolean attack_gives_check[maxply+1];

/* Allocate a STContinuationSolver defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_check_detector_slice(stip_length_type length,
                                            stip_length_type min_length);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
check_detector_defend_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
check_detector_can_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_max_unsolvable);

/* Instrument the stipulation representation so that it can deal with
 * continuations
 * @param si identifies slice where to start
 */
void stip_insert_check_detectors(slice_index si);

#endif
