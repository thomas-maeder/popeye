#if !defined(CONDITIONS_SINGLEBOX_TYPE3_H)
#define CONDITIONS_SINGLEBOX_TYPE3_H

/* Implementation of condition Singlebox Type 3
 */

#include "solving/solve.h"
#include "pyproc.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type3(slice_index si);

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
stip_length_type singlebox_type3_pawn_promoter_solve(slice_index si,
                                                     stip_length_type n);

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
stip_length_type singlebox_type3_legality_tester_solve(slice_index si,
                                                       stip_length_type n);

/* is the indicated king square attacked according to type 3?
 */
boolean singleboxtype3_is_black_king_square_attacked(evalfunction_t *evaluate);
boolean singleboxtype3_is_white_king_square_attacked(evalfunction_t *evaluate);

/* Generate the moves for a black/white piece
 * @param side for which to generate moves
 * @param sq_departure departure square of the moves
 * @param p walk and side of the piece
 */
void singleboxtype3_generate_moves_for_piece(Side side, square sq_departure, piece p);

#endif
