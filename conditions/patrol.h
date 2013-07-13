#if !defined(CONDITIONS_PATROL_H)
#define CONDITIONS_PATROL_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the conditions Patrol Chess and Ultra-Patro Chess */

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
stip_length_type patrol_remove_unsupported_captures_solve(slice_index si,
                                                          stip_length_type n);

/* Initialise solving in Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void patrol_initialise_solving(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void ultrapatrol_generate_moves_for_piece(slice_index si,
                                          square sq_departure,
                                          PieNam p);

/* Inialise the solving machinery with Ultra-Patrol Chess
 * @param si identifies root slice of solving machinery
 */
void ultrapatrol_initialise_solving(slice_index si);

#endif
