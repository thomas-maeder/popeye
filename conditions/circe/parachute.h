#if !defined(CONDITIONS_CIRCE_PARACHUTE_H)
#define CONDITIONS_CIRCE_PARACHUTE_H

/* Implementation of condition Circe Assassin
 */

#include "solving/machinery/solve.h"
#include "solving/move_effect_journal.h"

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
void circe_parachute_remember_solve(slice_index si);

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
void circe_volcanic_remember_solve(slice_index si);

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
void circe_volcanic_swapper_solve(slice_index si);

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
void circe_parachute_uncoverer_solve(slice_index si);

void move_effect_journal_undo_circe_parachute_remember(move_effect_journal_entry_type const *entry);
void move_effect_journal_redo_circe_parachute_remember(move_effect_journal_entry_type const *entry);

void move_effect_journal_do_circe_volcanic_remember(move_effect_reason_type reason,
                                                    square sq_rebirth);
void move_effect_journal_undo_circe_volcanic_remember(move_effect_journal_entry_type const *entry);
void move_effect_journal_redo_circe_volcanic_remember(move_effect_journal_entry_type const *entry);

void move_effect_journal_do_circe_volcanic_swap(move_effect_reason_type reason,
                                                square on);
void move_effect_journal_undo_circe_volcanic_swap(move_effect_journal_entry_type const *entry);
void move_effect_journal_redo_circe_volcanic_swap(move_effect_journal_entry_type const *entry);

/* Initialise the solving machinery with Circe Parachute
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval to be initialised
 */
void circe_parachute_initialise_solving(slice_index si,
                                        slice_type interval_start);

/* Initialise the solving machinery with Circe Volcanic
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval to be initialised
 */
void circe_volcanic_initialise_solving(slice_index si,
                                       slice_type interval_start);

#endif
