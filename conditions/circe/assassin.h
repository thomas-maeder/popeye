#if !defined(CONDITIONS_CIRCE_ASSASSIN_H)
#define CONDITIONS_CIRCE_ASSASSIN_H

/* Implementation of condition Circe Assassin
 */

#include "solving/solve.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean assassin_circe_check_tester_is_in_check(slice_index si,
                                                Side side_in_check);

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
stip_length_type circe_assassin_assassinate_solve(slice_index si,
                                                  stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void assassin_circe_initalise_solving(slice_index si);

#endif
