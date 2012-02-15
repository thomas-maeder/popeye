#if !defined(SOLVING_AVOID_UNSOLVABLE_H)
#define SOLVING_AVOID_UNSOLVABLE_H

/* Avoid solving branches that we know are unsolvable
 */

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* Instrument STEndOfBranch* slices with the necessary STAvoidUnusable slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_avoid_unsolvable_forks(slice_index root_slice);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type avoid_unsolvable_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type avoid_unsolvable_defend(slice_index si, stip_length_type n);

#endif
