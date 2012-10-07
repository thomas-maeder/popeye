#if !defined(CONDITIONS_EXCHANGE_CASTLING_H)
#define CONDITIONS_EXCHANGE_CASTLING_H

#include "solving/solve.h"

/* This module implements the condition Exchange Castling */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type exchange_castling_move_player_solve(slice_index si,
                                                      stip_length_type n);

/* Instrument slices with Castling Chess slices
 */
void stip_insert_exchange_castling(slice_index si);

#endif
