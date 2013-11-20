#if !defined(CONDITIONS_SAT_H)
#define CONDITIONS_SAT_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"

extern boolean StrictSAT[nr_sides];

extern unsigned int SAT_max_nr_allowed_flights[nr_sides];

/* Undo a Strict SAT state adjustment
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_strict_sat_adjustment(move_effect_journal_index_type curr);

/* Redo a Strict SAT state adjustment
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_strict_sat_adjustment(move_effect_journal_index_type curr);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean sat_check_tester_is_in_check(slice_index si, Side side_in_check);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean strictsat_check_tester_is_in_check(slice_index si, Side side_in_check);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean satxy_check_tester_is_in_check(slice_index si, Side side_in_check);

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
stip_length_type strict_sat_initialiser_solve(slice_index si,
                                              stip_length_type n);

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
stip_length_type strict_sat_updater_solve(slice_index si,
                                           stip_length_type n);

/* Instrument a stipulation for strict SAT
 * @param si identifies root slice of stipulation
 */
void strictsat_initialise_solving(slice_index si);

void sat_initialise_solving(slice_index si);

#endif
