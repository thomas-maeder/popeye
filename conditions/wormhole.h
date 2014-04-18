#if !defined(CONDITIONS_WORMHOLE_H)
#define CONDITIONS_WORMHOLE_H

#include "solving/machinery/solve.h"
#include "solving/ply.h"

/* This module implements the condition Wormholes */

enum
{
  wormholes_capacity = 100
};

extern square wormhole_positions[wormholes_capacity];

/* index into wormhole_positions */
extern unsigned int wormhole_next_transfer[maxply+1];

/* Validate an observation according to Worm holes
 * @return true iff the observation is valid
 */
boolean wormhole_validate_observation(slice_index si);

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
void wormhole_remove_illegal_captures_solve(slice_index si);

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
void wormhole_transferer_solve(slice_index si);

/* Initialise solving in Wormholes
 * @param si root slice of stipulation
 */
void wormhole_initialse_solving(slice_index si);

#endif
