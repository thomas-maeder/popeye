#if !defined(CONDITIONS_SINGLEBOX_TYPE3_H)
#define CONDITIONS_SINGLEBOX_TYPE3_H

/* Implementation of condition Singlebox Type 3
 */

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "solving/observation.h"

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

/* Determine whether a square is observed in Singlebox Type 3
* @param si identifies tester slice
* @return true iff sq_target is observed
*/
boolean singleboxtype3_is_square_observed(slice_index si,
                                          evalfunction_t *evaluate);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void singleboxtype3_generate_moves_for_piece(slice_index si, PieNam p);

#endif
