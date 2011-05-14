#if !defined(STIPULATION_BATTLE_PLAY_POSTKEYPLAY_H)
#define STIPULATION_BATTLE_PLAY_POSTKEYPLAY_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with post key play
 */

/* Allocate a STPostKeyPlaySuppressor defender slice.
 * @return index of allocated slice
 */
slice_index alloc_postkeyplay_suppressor_slice(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
stip_length_type
postkeyplay_suppressor_defend(slice_index si,
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
postkeyplay_suppressor_can_defend(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable);

#endif
