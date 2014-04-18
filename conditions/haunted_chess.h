#if !defined(CONDITION_HAUNTED_CHESS_H)
#define CONDITION_HAUNTED_CHESS_H

#include "pieces/pieces.h"
#include "position/underworld.h"
#include "solving/machinery/solve.h"
#include "solving/move_effect_journal.h"

/* This module implements Haunted Chess.
 */

/* Remember the ghost from the current capture
 */
void move_effect_journal_do_remember_ghost(void);
void move_effect_journal_undo_remember_ghost(move_effect_journal_index_type curr);
void move_effect_journal_redo_remember_ghost(move_effect_journal_index_type curr);

/* Forget a ghost "below" a square (typically because it is on the board now)
 */
void move_effect_journal_do_forget_ghost(underworld_index_type const summoned);
void move_effect_journal_undo_forget_ghost(move_effect_journal_index_type curr);
void move_effect_journal_redo_forget_ghost(move_effect_journal_index_type curr);

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
void haunted_chess_ghost_summoner_solve(slice_index si);

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
void haunted_chess_ghost_rememberer_solve(slice_index si);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_haunted_chess(slice_index si);

#endif
