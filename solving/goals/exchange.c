#include "solving/goals/exchange.h"
#include "position/pieceid.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "position/effects/utils.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether an exchange goal has just been reached
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
void goal_exchange_reached_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    Side const just_moved = advers(SLICE_STARTER(si));
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    square const sq_diagram = GetPositionInDiagram(movingspec);
    pipe_this_move_doesnt_solve_if(si,
                                   GetPositionInDiagram(being_solved.spec[sq_diagram])!=pos
                                   || !TSTFLAG(being_solved.spec[sq_diagram],just_moved)
                                   || sq_diagram==pos);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
