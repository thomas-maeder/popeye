#if !defined(CONDITIONS_CIRCE_PARACHUTE_H)
#define CONDITIONS_CIRCE_PARACHUTE_H

/* Implementation of condition Circe Assassin
 */

#include "solving/solve.h"
#include "solving/move_effect_journal.h"

enum { circe_parachute_covered_capacity = 32 };

extern move_effect_journal_index_type circe_parachute_covered_pieces[circe_parachute_covered_capacity];

extern unsigned int circe_parachute_nr_covered_pieces;

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
stip_length_type circe_parachute_remember_solve(slice_index si,
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
stip_length_type circe_volcanic_remember_solve(slice_index si,
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
stip_length_type circe_parachute_uncoverer_solve(slice_index si,
                                                 stip_length_type n);

void move_effect_journal_undo_circe_parachute_remember(move_effect_journal_index_type curr);
void move_effect_journal_redo_circe_parachute_remember(move_effect_journal_index_type curr);

void move_effect_journal_undo_circe_volcanic_remember(move_effect_journal_index_type curr);
void move_effect_journal_redo_circe_volcanic_remember(move_effect_journal_index_type curr);

void move_effect_journal_undo_circe_parachute_uncover(move_effect_journal_index_type curr);
void move_effect_journal_redo_circe_parachute_uncover(move_effect_journal_index_type curr);

#endif
