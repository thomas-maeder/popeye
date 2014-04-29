#if !defined(PIECES_NEUTRAL_HALF_H)
#define PIECES_NEUTRAL_HALF_H

#include "solving/machinery/solve.h"
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
void half_neutral_recolorer_solve(slice_index si);

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void solving_insert_half_neutral_recolorers(slice_index si);

#endif
