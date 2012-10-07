#if !defined(CONDITION_GHOST_CHESS_H)
#define CONDITION_GHOST_CHESS_H

#include "solving/solve.h"

/* This module implements Ghost Chess.
 */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ghost_chess_ghost_rememberer_solve(slice_index si,
                                                     stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_ghost_chess(slice_index si);

#endif
