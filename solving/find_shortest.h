#if !defined(SOLVING_BATTLE_PLAY_ATTACK_FIND_SHORTEST_H)
#define SOLVING_BATTLE_PLAY_ATTACK_FIND_SHORTEST_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STFindShortest stipulation slices.
 */

/* Allocate a STFindShortest slice.
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return index of allocated slice
 */
slice_index alloc_find_shortest_slice(stip_length_type length,
                                      stip_length_type min_length);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type find_shortest_solve(slice_index si, stip_length_type n);

/* Instrument the stipulation with slices that attempt the shortest
 * solutions/variations
 * @param si root slice of stipulation
 */
void stip_insert_find_shortest_solvers(slice_index si);

#endif
