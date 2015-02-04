#if !defined(SOLVING_FORK_H)
#define SOLVING_FORK_H

/* Forks
 */

#include "solving/machinery/solve.h"

/* Solve the next2 part of a fork
 * @param si identifies the fork slice
 * @param n maximum number of moves (typically slack_length or
 *         length_unspecified)
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
stip_length_type fork_solve(slice_index si, stip_length_type n);

/* Delegate solving to next2
 * @param si identifies the pipe
 */
void fork_solve_delegate(slice_index si);

/* Delegate testing observation to next2
 * @param si identifies the pipe
 */
void fork_is_square_observed_delegate(slice_index si);

/* Delegate generating to next2
 * @param si identifies the pipe
 */
void fork_move_generation_delegate(slice_index si);

#endif
