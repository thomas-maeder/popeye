#if !defined(CONDITIONS_ARC_H)
#define CONDITIONS_ARC_H

/* This module implements the condition Actuated Revolving Centre. */

#include "solving/solve.h"
#include "solving/move_effect_journal.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_actuated_revolving_centre(slice_index si);

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
stip_length_type actuated_revolving_centre_solve(slice_index si,
                                                  stip_length_type n);

void undo_centre_revolution(move_effect_journal_index_type curr);
void redo_centre_revolution(move_effect_journal_index_type curr);

/* Apply revolution to one square
 * @param s the square
 * @return revolved square
 */
square actuated_revolving_centre_revolve_square(square s);

#endif
