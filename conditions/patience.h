#if !defined(CONDITIONS_PATIENCE_H)
#define CONDITIONS_PATIENCE_H

/* Implementation of condition Patience chess (DOESN'T REALLY WORK!!!!)
 */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

extern boolean PatienceB;

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_patience_chess(slice_index si);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type patience_chess_legality_tester_attack(slice_index si,
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
stip_length_type patience_chess_legality_tester_defend(slice_index si,
                                                       stip_length_type n);

#endif
