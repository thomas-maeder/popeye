#if !defined(CONDITIONS_ANTICIRCE_REBIRTH_HANDLER_H)
#define CONDITIONS_ANTICIRCE_REBIRTH_HANDLER_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"
#include "solving/move_effect_journal.h"

extern PieNam current_promotion_of_reborn_moving[maxply+1];

/* Perform an Anticirce rebirth
 * @param reason reason for rebirth
 */
void anticirce_do_rebirth(move_effect_reason_type reason);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_rebirth_handler_attack(slice_index si,
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
stip_length_type anticirce_rebirth_handler_defend(slice_index si,
                                                   stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_reborn_promoter_attack(slice_index si,
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
stip_length_type anticirce_reborn_promoter_defend(slice_index si,
                                                  stip_length_type n);

/* Instrument a stipulation for strict Circe rebirths
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_rebirth_handlers(slice_index si);

#endif
