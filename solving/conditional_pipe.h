#if !defined(SOLVING_CONDITIONAL_PIPE_H)
#define SOLVING_CONDITIONAL_PIPE_H

/* Conditional pipes
 */

#include "solving/machinery/solve.h"

/* Solve the next2 part of a conditional pipe
 * @param si identifies the fork slice
 * @return one of
 *        previous_move_has_solved
 *        previous_move_is_illegal
 *        next_move_has_no_solution
 *        next_move_has_solution
 *        immobility_on_next_move
 */
stip_length_type conditional_pipe_solve(slice_index si);

#endif
