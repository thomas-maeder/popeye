#if !defined(CONDITIONS_TAKE_AND_MAKE_H)
#define CONDITIONS_TAKE_AND_MAKE_H

#include "py.h"

/* This module implements the condition Take&Make*/

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type take_and_make_generate_make_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument the solvers with Take&Make
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_take_and_make(slice_index si);

#endif
