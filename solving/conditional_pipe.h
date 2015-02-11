#if !defined(SOLVING_CONDITIONAL_PIPE_H)
#define SOLVING_CONDITIONAL_PIPE_H

/* Conditional pipes
 * Delegate solving along the pipe only if testing the condition referred
 * to by .next2 is successful.
 * Own the branch attached to .next2 - will deallocate it while being deallocated
 */

#include "solving/machinery/solve.h"

/* Solve the next2 part of a conditional pipe
 * @param si identifies the fork slice
 * @return one of
 *        previous_move_has_solved
 *        previous_move_has_not_solved
 *        previous_move_is_illegal
 *        immobility_on_next_move
 */
stip_length_type conditional_pipe_solve_delegate(slice_index si);

#endif
