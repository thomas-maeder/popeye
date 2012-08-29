#if !defined(CONDITIONS_IMITATOR_H)
#define CONDITIONS_IMITATOR_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"
#include "solving/move_effect_journal.h"

/* This module implements imitators */

extern boolean promotion_of_moving_into_imitator[maxply+1];
extern square im0;                    /* position of the 1st imitator */

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type imitator_mover_attack(slice_index si, stip_length_type n);

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
stip_length_type imitator_mover_defend(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type moving_pawn_to_imitator_promoter_attack(slice_index si,
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
stip_length_type moving_pawn_to_imitator_promoter_defend(slice_index si,
                                                         stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_imitator(slice_index si);

void undo_imitator_movement(move_effect_journal_index_type curr);
void redo_imitator_movement(move_effect_journal_index_type curr);

void undo_imitator_addition(move_effect_journal_index_type curr);
void redo_imitator_addition(move_effect_journal_index_type curr);

#endif
