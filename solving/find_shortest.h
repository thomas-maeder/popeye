#if !defined(SOLVING_BATTLE_PLAY_ATTACK_FIND_SHORTEST_H)
#define SOLVING_BATTLE_PLAY_ATTACK_FIND_SHORTEST_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STFindShortest stipulation slices.
 */

/* Allocate a STFindShortest slice.
 * @return index of allocated slice
 */
slice_index alloc_find_shortest_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_shortest_attack(slice_index si, stip_length_type n);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type find_shortest_help(slice_index si, stip_length_type n);

/* Instrument the stipulation with slices that attempt the shortest
 * solutions/variations
 * @param si root slice of stipulation
 */
void stip_insert_find_shortest_solvers(slice_index si);

#endif
