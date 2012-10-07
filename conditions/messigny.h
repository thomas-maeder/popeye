#if !defined(CONDITIONS_MESSIGNY_H)
#define CONDITIONS_MESSIGNY_H

/* This module contains the implementation of Messigny Chess */

#include "solving/solve.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_messigny(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type messigny_move_player_solve(slice_index si, stip_length_type n);

#endif
