#if !defined(SOLVING_FORK_ON_REMAINING_H)
#define SOLVING_FORK_ON_REMAINING_H

#include "solving/solve.h"

/* STForkOnRemaining fork if there are less than a certain number of moves left
 * in a branch.
 */

/* Allocate a STForkOnRemaining slice.
 * @param op1 identifies direction taken if threshold is not met
 * @param op2 identifies direction taken if threshold is met
 * @param threshold at which move should we continue with op2?
 * @return index of allocated slice
 */
slice_index alloc_fork_on_remaining_slice(slice_index op1,
                                          slice_index op2,
                                          stip_length_type threshold);

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
stip_length_type fork_on_remaining_solve(slice_index si, stip_length_type n);

#endif
