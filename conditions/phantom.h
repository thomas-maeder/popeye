#if !defined(CONDITIONS_PHANTOM_H)
#define CONDITIONS_PHANTOM_H

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "solving/observation.h"

/* This module provides implements the condition Phantom Chess
 */

extern boolean phantom_chess_rex_inclusive;

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @note the piece on the departure square need not have that walk
 */
void phantom_generate_moves_for_piece(slice_index si, PieNam p);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
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
stip_length_type phantom_en_passant_adjuster_solve(slice_index si,
                                                   stip_length_type n);

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si);

/* Determine whether a specific side is in check in Phantom Chess
 * @param si identifies tester slice
 * @return true iff side is in check
 */
boolean phantom_is_square_observed(slice_index si, validator_id evaluate);

#endif
