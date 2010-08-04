#if !defined(STIPULATION_BATTLE_PLAY_POSTKEYPLAY_H)
#define STIPULATION_BATTLE_PLAY_POSTKEYPLAY_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with post key play
 */

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
stip_length_type
postkeyplay_suppressor_defend_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
postkeyplay_suppressor_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable,
                                       unsigned int max_nr_refutations);

/* Instrument the stipulation representation so that it can write
 * refuting variations
 * @param si identifies slice where to start
 */
void stip_insert_postkey_handlers(slice_index si);

/* Instrument the stipulation representation so that post key play is
 * suppressed from output
 * @param si identifies slice where to start
 */
void stip_insert_postkeyplay_suppressors(slice_index si);

#endif
