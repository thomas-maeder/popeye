#if !defined(CONDITIONS_PHANTOM_H)
#define CONDITIONS_PHANTOM_H

#include "solving/solve.h"
#include "pyproc.h"

/* This module provides implements the condition Phantom Chess
 */

extern boolean phantom_chess_rex_inclusive;

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not have that walk
 */
void phantom_chess_generate_moves(Side side, PieNam p, square sq_departure);

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
stip_length_type phantom_en_passant_adjuster_solve(slice_index si,
                                                    stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_phantom_en_passant_adjusters(slice_index si);

/* Determine whether a specific side is in check in Phantom Chess
 * @param side_in_check the side
 * @param sq_target square potentially observed
 * @return true iff side is in check
 */
boolean phantom_is_square_observed(square sq_target, evalfunction_t *evaluate);

#endif
