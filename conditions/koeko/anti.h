#if !defined(CONDITIONS_KOEKO_ANTI_H)
#define CONDITIONS_KOEKO_ANTI_H

#include "solving/solve.h"
#include "conditions/koeko/koeko.h"

/* This module implements the condition Anti-Koeko */

extern nocontactfunc_t antikoeko_nocontact;

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
stip_length_type antikoeko_legality_tester_solve(slice_index si, stip_length_type n);

/* Inialise solving in Anti-Koeko
 * @param si identifies the root slice of the stipulation
 */
void antikoeko_initialise_solving(slice_index si);

#endif
