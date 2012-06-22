#if !defined(SOLVING_FIND_MOVE_H)
#define SOLVING_FIND_MOVE_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* This module provides functionality dealing with the attacking side
 * in STFindMove stipulation slices.
 */

/* Allocate a STFindMove slice.
 * @return index of allocated slice
 */
slice_index alloc_find_move_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_move_attack(slice_index si, stip_length_type n);

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
stip_length_type find_move_defend(slice_index si, stip_length_type n);

#endif
