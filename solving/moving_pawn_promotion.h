#if !defined(SOLVING_MOVING_PAWN_PROMOTION_H)
#define SOLVING_MOVING_PAWN_PROMOTION_H

#include "solving/solve.h"

/* This module provides implements the promotion of the moving pawn
 */

extern PieNam current_promotion_of_moving[maxply+1];

/* Has a pawn reached a promotion square
 * @param side the pawn's side
 * @param square_reached square reached by the pawn
 * @return true iff square_reached is a promotion square
 */
boolean has_pawn_reached_promotion_square(Side side, square square_reached);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type moving_pawn_promoter_solve(slice_index si, stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_moving_pawn_promoters(slice_index si);

#endif
