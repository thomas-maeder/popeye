#if !defined(CONDITIONS_WORMHOLE_H)
#define CONDITIONS_WORMHOLE_H

#include "solving/solve.h"

/* This module implements the condition Wormholes */

enum
{
  wormholes_capacity = 100
};

extern square wormhole_positions[wormholes_capacity];

/* index into wormhole_positions */
extern unsigned int wormhole_next_transfer[maxply+1];

/* Validate an observation according to Worm holes
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean wormhole_validate_observation(slice_index si,
                                      square sq_observer,
                                      square sq_landing);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type wormhole_transferer_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type wormhole_transfered_promoter_solve(slice_index si,
                                                    stip_length_type n);

/* Initialise solving in Wormholes
 * @param si root slice of stipulation
 */
void wormhole_initialse_solving(slice_index si);

#endif
