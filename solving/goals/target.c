#include "solving/goals/target.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "position/effects/utils.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
void goal_target_reached_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(CURRMOVE_OF_PLY(nbply)!=nil_coup);

  {
    move_effect_journal_index_type const top = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const sq_eventual_arrival = move_effect_journal_follow_piece_through_other_effects(nbply,moving_id,sq_arrival);
    pipe_this_move_doesnt_solve_if(si,
                                   sq_eventual_arrival
                                   !=SLICE_U(si).goal_handler.goal.target);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
