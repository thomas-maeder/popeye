#if !defined(SOLVING_PIPE_H)
#define SOLVING_PIPE_H

/* Pipes
 */

#include "solving/machinery/solve.h"
#include "pieces/pieces.h"

/* Report illegality under some condition; go on otherwise
 * @param si identifies the pipe slice
 * @param condition if true, report this_move_is_illegal, otherwise solve next1
 */
void pipe_this_move_illegal_if(slice_index si, boolean condition);

/* Report success in solving under some condition; go on otherwise
 * @param si identifies the pipe slice
 * @param condition if true, report that this move solves, otherwise solve next1
 */
void pipe_this_move_solves_if(slice_index si, boolean condition);

/* Report failure to solve under some condition; go on otherwise
 * @param si identifies the pipe slice
 * @param condition if true, report that this move doesn't solve, otherwise solve next1
 */
void pipe_this_move_doesnt_solve_if(slice_index si, boolean condition);

/* Report success in solving under some condition and not otherwise
 * @param condition if true, report that this move solves, otherwise that it doesn't
 */
void pipe_this_move_solves_exactly_if(boolean condition);

/* Delegate solving to next1
 * @param si identifies the pipe
 */
void pipe_solve_delegate(slice_index si);

/* Delegate testing observation to next1
 * @param si identifies the pipe
 */
void pipe_is_square_observed_delegate(slice_index si);

/* Delegate generating to next1
 * @param si identifies the pipe
 */
void pipe_move_generation_delegate(slice_index si);

/* Delegate generating with different walk to next1
 * @param si identifies the pipe
 * @param w walk to generate for
 */
void pipe_move_generation_different_walk_delegate(slice_index si,
                                                 piece_walk_type w);

/* Delegate validating to next1
 * @param si identifies the pipe
 * @return true iff the observation is valid
 */
boolean pipe_validate_observation_recursive_delegate(slice_index si);

/* Delegate testing to next1
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean pipe_is_in_check_recursive_delegate(slice_index si, Side side_in_check);

/* Dispatch to next1. Intended to be used by pipes that can be used in different
 * contexts.
 * @param si identifies the pipe
 */
void pipe_dispatch_delegate(slice_index si);

#endif
