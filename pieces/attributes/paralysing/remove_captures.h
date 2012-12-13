#if !defined(PIECES_PARALYSING_REMOVE_CAPTURES_H)
#define PIECES_PARALYSING_REMOVE_CAPTURES_H

#include "py.h"

/* This module removes generated capturing moves by paralysing pieces */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type paralysing_remove_captures_solve(slice_index si,
                                                  stip_length_type n);

#endif
