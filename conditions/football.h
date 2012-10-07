#if !defined(SOLVING_CONDITIONS_FOOTBALL_H)
#define SOLVING_CONDITIONS_FOOTBALL_H

#include "solving/solve.h"

/* This module implements Football Chess */

extern piece current_football_substitution[maxply+1];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type football_chess_substitutor_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_football_chess(slice_index si);

#endif
