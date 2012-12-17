#if !defined(CONDITIONS_MARSCIRCE_ANTI_H)
#define CONDITIONS_MARSCIRCE_ANTI_H

#include "solving/solve.h"

/* This module provides implements the condition Anti-Mars-Circe
 */

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not have that walk
 */
void antimars_generate_moves(Side side, piece p, square sq_departure);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type antimars_en_passant_adjuster_solve(slice_index si,
                                                     stip_length_type n);

/* Instrument slices with Anti-Mars-Circe
 */
void stip_insert_antimars_en_passant_adjusters(slice_index si);

#endif
