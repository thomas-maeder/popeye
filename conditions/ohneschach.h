#if !defined(CONDITIONS_OHNESCHACH_H)
#define CONDITIONS_OHNESCHACH_H

/* This module implements the condition Ohneschach
 */

#include "solving/solve.h"
#include "solving/ply.h"

extern boolean ohneschach_undecidable_goal_detected[maxply+1];

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
stip_length_type ohneschach_stop_if_check_and_not_mate_solve(slice_index si,
                                                             stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void ohneschach_insert_check_guards(slice_index si);

#endif
