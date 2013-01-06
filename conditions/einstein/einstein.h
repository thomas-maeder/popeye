#if !defined(CONDITIONS_EINSTEIN_EINSTEIN_H)
#define CONDITIONS_EINSTEIN_EINSTEIN_H

#include "solving/solve.h"

/* This module implements the condition Einstein Chess */

/* Decrease the rank of a piece
 * @param p piece whose rank to decrease
 */
piece einstein_decrease_piece(piece p);

/* Increase the rank of a piece
 * @param p piece whose rank to increase
 */
piece einstein_increase_piece(piece p);

/* Adjust the castling partner if the current move is a castling
 * @param trait_ply side executing the current move
 */
void einstein_decrease_castling_partner(Side trait_ply);

/* Adjust the castling partner if the current move is a castling
 * @param trait_ply side executing the current move
 */
void einstein_increase_castling_partner(Side trait_ply);

/* Remember all capturers of the current move
 * @param is_capturer remembers capturers by their departure square (minus square_a1)
 */
void einstein_collect_capturers(boolean is_capturer[square_h8-square_a1]);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type einstein_moving_adjuster_solve(slice_index si,
                                                 stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_einstein_moving_adjusters(slice_index si);

#endif
