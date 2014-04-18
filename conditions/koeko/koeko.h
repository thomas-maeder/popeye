#if !defined(CONDITIONS_KOEKO_H)
#define CONDITIONS_KOEKO_H

#include "solving/machinery/solve.h"
#include "utilities/boolean.h"

/* This module implements the condition Koeko */

typedef boolean (*nocontactfunc_t)(square);

extern nocontactfunc_t koeko_nocontact;

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void koeko_legality_tester_solve(slice_index si);

/* Initialise solving in Koeko
 * @param si identifies the root slice of the stipulation
 */
void koeko_initialise_solving(slice_index si);

boolean nokingcontact(square ia);
boolean nowazircontact(square ia);
boolean noferscontact(square ia);
boolean noknightcontact(square ia);
boolean nocamelcontact(square ia);
boolean noalfilcontact(square ia);
boolean nozebracontact(square ia);
boolean nodabbabacontact(square ia);
boolean nogiraffecontact(square ia);
boolean noantelopecontact(square ia);

#endif
