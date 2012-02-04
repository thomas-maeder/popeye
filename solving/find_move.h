#if !defined(SOLVING_FIND_MOVE_H)
#define SOLVING_FIND_MOVE_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STFindMove stipulation slices.
 */

/* Allocate a STFindMove slice.
 * @return index of allocated slice
 */
slice_index alloc_find_move_slice(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_move_can_attack(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type find_move_can_defend(slice_index si, stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type find_move_can_help(slice_index si, stip_length_type n);

#endif
