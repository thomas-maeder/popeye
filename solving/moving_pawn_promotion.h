#if !defined(SOLVING_MOVING_PAWN_PROMOTION_H)
#define SOLVING_MOVING_PAWN_PROMOTION_H

#include "solving/solve.h"
#include "pieces/walks/pawns/promotion.h"

/* This module provides implements the promotion of the moving pawn
 */

extern pieces_pawns_promotion_sequence_type moving_pawn_promotion_state[maxply+1];

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
stip_length_type moving_pawn_promoter_solve(slice_index si, stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_moving_pawn_promoters(slice_index si);

#endif
