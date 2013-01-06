#if !defined(SOLVING_FOR_EACH_MOVE_H)
#define SOLVING_FOR_EACH_MOVE_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STForEachAttack stipulation slices.
 */

/* Instrument the stipulation with move iterator slices
 * @param root_slice identifies root slice of stipulation
 */
void stip_insert_move_iterators(slice_index root_slice);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type for_each_attack_solve(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type for_each_defense_solve(slice_index si, stip_length_type n);

#endif
