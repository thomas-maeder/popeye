#include "stipulation/goals/exchange/reached_tester.h"
#include "position/pieceid.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether an exchange goal has just been reached
 */

/* Allocate a system of slices that tests whether exchange has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_exchange_reached_tester_system(void)
{
  slice_index result;
  slice_index exchange_tester;
  Goal const goal = { goal_exchange, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  exchange_tester = alloc_pipe(STGoalExchangeReachedTester);
  pipe_link(exchange_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,exchange_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type goal_exchange_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    Side const just_moved = advers(slices[si].starter);
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    square const sq_diagram = GetPositionInDiagram(movingspec);
    if (GetPositionInDiagram(spec[sq_diagram])==pos
        && TSTFLAG(spec[sq_diagram],just_moved)
        && sq_diagram!=pos)
      result = solve(slices[si].next1,n);
    else
      result = n+2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
