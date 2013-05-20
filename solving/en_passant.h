#if !defined(SOLVING_EN_PASSANT_H)
#define SOLVING_EN_PASSANT_H

#include "solving/solve.h"

/* This module provides implements en passant captures
 */

extern square ep[maxply+1];

/* Remember a square avoided by a multistep move of a pawn
 * @param s avoided square
 */
void en_passant_remember_multistep_over(square s);

/* Forget the last square remembered by en_passant_remember_multistep_over()
 */
void en_passant_forget_multistep(void);

/* Was a pawn multistep move played in a certain ply?
 * @param ply the ply
 * @return true iff a multi step move was played in ply ply
 */
boolean en_passant_was_multistep_played(ply ply);

/* Is an en passant capture possible to a specific square?
 * @param s the square
 * @return true iff an en passant capture to s is currently possible
 */
boolean en_passant_is_capture_possible_to(square s);

/* Adjust en passant possibilities of the following move after a non-capturing
 * move
 * @param sq_multistep_departure departure square of pawn move
 * @return the square avoided by the multistep; initsquare if no multistep
 */
square en_passant_find_potential(square sq_multistep_departure);

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
stip_length_type en_passant_adjuster_solve(slice_index si, stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_en_passant_adjusters(slice_index si);

#endif
