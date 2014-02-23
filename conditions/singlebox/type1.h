#if !defined(CONDITIONS_SINGLEBOX_TYPE1_H)
#define CONDITIONS_SINGLEBOX_TYPE1_H

/* Implementation of condition Singlebox Type 1
 */

#include "solving/solve.h"
#include "conditions/conditions.h"

extern ConditionNumberedVariantType SingleBoxType;

/* Determine whether the move just played is legal according to Singlebox Type 1
 * @return true iff the move is legal
 */
boolean singlebox_type1_illegal(void);

/* Initialise solving in Singlebox Type 1
 * @param si identifies root slice of stipulation
 */
void singlebox_type1_initialise_solving(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type singlebox_type1_legality_tester_solve(slice_index si,
                                                        stip_length_type n);

#endif
