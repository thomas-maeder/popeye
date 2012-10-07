#if !defined(STIPULATION_MOVE_PLAYER_H)
#define STIPULATION_MOVE_PLAYER_H

#include "stipulation/slice_type.h"
#include "solving/solve.h"

/* This module plays the orthodox part of a move.
 */

void stip_instrument_moves(slice_index si, slice_type type);

/* Allocate a STMovePlayer slice.
 * @return index of allocated slice
 */
slice_index alloc_move_player_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_player_solve(slice_index si, stip_length_type n);

#endif
