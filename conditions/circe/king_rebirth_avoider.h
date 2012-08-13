#if !defined(CONDITIONS_CIRCE_KING_REBIRTH_AVOIDER_H)
#define CONDITIONS_CIRCE_KING_REBIRTH_AVOIDER_H

/* Avoid king rebirths in conditions like Isardam where kings may be under
 * (apparent) attack
 */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_king_rebirth_avoider_attack(slice_index si,
                                                   stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type circe_king_rebirth_avoider_defend(slice_index si,
                                                   stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_king_rebirth_avoiders(slice_index si);

#endif
