#if !defined(OPTIONS_ERBEN_H)
#define OPTIONS_ERBEN_H

/* This module implements the option erben */

#include "solving/machinery/solve.h"

/* Allocate a STErbenMovePlayer slice.
 * @param after_move identifies landing slice after move playing
 * @return index of allocated slice
 */
slice_index alloc_erben_player_slice(slice_index after_move);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void erben_player_solve(slice_index si);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void erben_generator_solve(slice_index si);

/* Instrument the solving machinery for erben
 * @param si identifies root slice of stipulation
 * @param side which side may play null moves? pass no_side for both_sides
 */
void erben_initialise_solving(slice_index si, Side side);

#endif
