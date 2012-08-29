#if !defined(CONDITIONS_ARC_H)
#define CONDITIONS_ARC_H

/* This module implements the condition Actuated Revolving Centre. */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"
#include "solving/move_effect_journal.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_actuated_revolving_centre(slice_index si);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type actuated_revolving_centre_attack(slice_index si,
                                                  stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type actuated_revolving_centre_defend(slice_index si,
                                                  stip_length_type n);

void undo_centre_revolution(move_effect_journal_index_type curr);
void redo_centre_revolution(move_effect_journal_index_type curr);

#endif
