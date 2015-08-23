/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
**
** 1995/10/31 TLi  Original
**
**************************** End of List ******************************/

#if !defined(PYPROOF_H)
#define PYPROOF_H

#include "stipulation/stipulation.h"
#include "position/position.h"
#include "solving/move_effect_journal.h"

extern boolean change_moving_piece;

extern position proofgames_start_position;
extern position proofgames_target_position;

/* a=>b: save the current piece places for the start position
 */
void ProofSaveStartPosition(void);

/* Compare two positions
 * @return true iff the current position is equal to the target
 *              position
 */
boolean ProofIdentical(void);

/* solve a proofgame stipulation
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
void proof_solve(slice_index si);

/* solve an A=>B stipulation
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
void atob_solve(slice_index si);

/* verify that the proof or A=>B goal is unique
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
void proof_verify_unique_goal_solve(slice_index si);

/* Restore the current position to the target position of a proofgame or A=>B stipulation
 * @param entry address of move effect journal entry that represents taking the
 *              restored snapshot
 */
void move_effect_journal_undo_snapshot_proofgame_target_position(move_effect_journal_entry_type const *entry);

#endif
