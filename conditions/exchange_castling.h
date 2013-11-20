#if !defined(CONDITIONS_EXCHANGE_CASTLING_H)
#define CONDITIONS_EXCHANGE_CASTLING_H

#include "pieces/pieces.h"
#include "solving/solve.h"

/* This module implements the condition Exchange Castling */

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
stip_length_type exchange_castling_move_player_solve(slice_index si,
                                                      stip_length_type n);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void exchange_castling_generate_moves_for_piece(slice_index si, PieNam p);

/* Instrument the solving machinery with Castling Chess slices
 * @param si identifies the root slice of the solving machinery
 */
void exchange_castling_initialise_solving(slice_index si);

#endif
