#include "solving/goals/castling.h"
#include "solving/castling.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a castling goal has just been reached
 */

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
void goal_castling_reached_tester_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,
                                 move_effect_journal[movement].reason
                                 !=move_effect_reason_castling_king_movement);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
