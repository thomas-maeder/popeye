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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type actuated_revolving_centre_solve(slice_index si,
                                                  stip_length_type n);

void undo_centre_revolution(move_effect_journal_index_type curr);
void redo_centre_revolution(move_effect_journal_index_type curr);

#endif
