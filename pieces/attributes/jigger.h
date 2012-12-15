#if !defined(PIECES_ATTRIBUTES_HURDLE_JIGGER_H)
#define PIECES_ATTRIBUTES_HURDLE_JIGGER_H

#include "py.h"

/* This module implements Jigger pieces */

extern nocontactfunc_t koeko_nocontact;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type jigger_legality_tester_solve(slice_index si, stip_length_type n);

/* Instrument the solvers with support for Jigger pieces
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_jigger(slice_index si);

#endif
