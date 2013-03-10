#if !defined(CONDITIONS_BGL_H)
#define CONDITIONS_BGL_H

#include "solving/solve.h"

/* This module implements the BGL condition */

extern long int BGL_values[nr_sides][maxply+1];
extern boolean BGL_global;

enum
{
  BGL_infinity = 10000000   /* this will do I expect; e.g. max len = 980 maxply < 1000 */
};

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type bgl_filter_solve(slice_index si, stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_bgl_filters(slice_index si);

boolean eval_BGL(square departure, square arrival, square capture);

#endif
