#if !defined(CONDITIONS_MONOCHROME_H)
#define CONDITIONS_MONOCHROME_H

#include "stipulation/stipulation.h"
#include "solving/move_generator.h"
#include "utilities/boolean.h"

/* This module implements the condition Monochrome Chess */

/* Validate the geometry of observation according to Monochrome Chess
 * @return true iff the observation is valid
 */
boolean monochrome_validate_observation_geometry(slice_index si);

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
void monochrome_remove_non_monochrome_moves_solve(slice_index si);

/* Initialise solving in Monochrome Chess
 * @param si identifies the root slice of the stipulation
 */
void monochrome_initialise_solving(slice_index si);

#endif
