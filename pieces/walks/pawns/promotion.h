#if !defined(PIECES_PAWNS_PROMOTION_H)
#define PIECES_PAWNS_PROMOTION_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"
#include "position/board.h"

/* This module provides implements the promotion of the moving pawn
 */

enum
{
  /* we calculate with:*/
  max_nr_promotions_per_ply = 4
};

/* effects up to this index have been consumed for pawn promotions */
extern move_effect_journal_index_type promotion_horizon;

/* Find the last square occupied by a piece since we last checked.
 * @param base index of move effects that have already been dealt with
 * @return the square; initsquare if there isn't any
 */
square find_potential_promotion_square(move_effect_journal_index_type base);

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
stip_length_type pawn_promoter_solve(slice_index si, stip_length_type n);

/* Instrument slices with promotee markers
 */
void pieces_pawns_promotion_initialise_solving(slice_index si);

#endif
