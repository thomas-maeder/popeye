#if !defined(SOLVING_FOR_EACH_MOVE_H)
#define SOLVING_FOR_EACH_MOVE_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STForEachMove stipulation slices.
 */

/* Allocate a STForEachMove slice.
 * @return index of allocated slice
 */
slice_index alloc_for_each_move_slice(void);

/* Instrument the stipulation with move iterator slices
 * @param root_slice identifies root slice of stipulation
 */
void stip_insert_move_iterators(slice_index root_slice);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type for_each_move_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type for_each_move_defend(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type for_each_move_help(slice_index si, stip_length_type n);

#endif
