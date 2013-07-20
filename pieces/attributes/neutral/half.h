#if !defined(PIECES_NEUTRAL_HALF_H)
#define PIECES_NEUTRAL_HALF_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"

/* This module implements half-neutral pieces */

/* Undo the deneutralisation a half-neutral piece
 * @param curr identifies the deneutralisation effect
 */
void undo_half_neutral_deneutralisation(move_effect_journal_index_type curr);

/* Redo the deneutralisation a half-neutral piece
 * @param curr identifies the deneutralisation effect
 */
void redo_half_neutral_deneutralisation(move_effect_journal_index_type curr);

/* Undo the neutralisation a half-neutral piece
 * @param curr identifies the neutralisation effect
 */
void undo_half_neutral_neutralisation(move_effect_journal_index_type curr);

/* Redo the neutralisation a half-neutral piece
 * @param curr identifies the neutralisation effect
 */
void redo_half_neutral_neutralisation(move_effect_journal_index_type curr);

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
stip_length_type half_neutral_recolorer_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_half_neutral_recolorers(slice_index si);

#endif
