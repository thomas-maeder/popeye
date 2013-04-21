#if !defined(SOLVING_MOVING_PAWN_PROMOTION_H)
#define SOLVING_MOVING_PAWN_PROMOTION_H

#include "solving/solve.h"

/* This module provides implements the promotion of the moving pawn
 */

typedef enum
{
  promotee_chain_orthodox,
  promotee_chain_marine,

  nr_promotee_chains
} promotee_chain_selector_type;

extern PieNam promotee_chain[nr_promotee_chains][PieceCount];
extern PieNam current_promotion_of_moving[maxply+1];

/* Initialise the set of promotion pieces for the current twin
 */
void init_promotion_pieces(void);

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
