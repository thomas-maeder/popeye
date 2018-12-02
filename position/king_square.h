#if !defined(POSITION_KING_SQUARE_H)
#define POSITION_KING_SQUARE_H

/* This modules implements saving the positions of the kings (if any).
 * In theory, this should be an optimisation only, but in fact a number of modules
 * rely on it.
 */

#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"

/* Update the king squares according to the effects since king_square_horizon
 * @note Updates king_square_horizon; solvers invoking this function should
 *       reset king_square_horizon to its previous value before returning
 */
void update_king_squares(void);

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
void king_square_updater_solve(slice_index si);

/* Add king square piece_movement to the current move of the current ply
 * @param reason reason for moving the king square
 * @param side whose king square to move
 * @param to where to move the king square
 */
void move_effect_journal_do_king_square_movement(move_effect_reason_type reason,
                                                 Side side,
                                                 square to);

/* Initalise the module */
void king_square_initialise(void);

#endif
