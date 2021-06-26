#include "solving/king_capture_avoider.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "solving/machinery/twin.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

typedef struct
{
    twin_id_type own_king_capture_possible;
    twin_id_type opponent_king_capture_possible;
} insertion_state;

static insertion_state root_state;

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side without king
 */
void king_capture_avoiders_avoid_own(void)
{
  root_state.own_king_capture_possible = twin_id;
}

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side's opponent without king
 */
void king_capture_avoiders_avoid_opponent(void)
{
  root_state.opponent_king_capture_possible = twin_id;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  insertion_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->own_king_capture_possible==twin_id)
  {
    slice_index const prototype = alloc_pipe(STOwnKingCaptureAvoider);
    move_insert_slices(si,st->context,&prototype,1);
  }

  if (state->opponent_king_capture_possible==twin_id)
  {
    slice_index const prototype = alloc_pipe(STOpponentKingCaptureAvoider);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_goal_with_potential_king_capture(slice_index si,
                                                      stip_structure_traversal *st)
{
  insertion_state * const state = st->param;
  twin_id_type const save_own_king_capture_possible = state->own_king_capture_possible;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->own_king_capture_possible = twin_id;
  stip_traverse_structure_children(si,st);
  state->own_king_capture_possible = save_own_king_capture_possible;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with king capture avoiders
 * @param si identifies root slice of the solving machinery
 */
void solving_insert_king_capture_avoiders(slice_index si)
{
  insertion_state state = root_state;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STGoalCounterMateReachedTester,
                                           &remember_goal_with_potential_king_capture);
  stip_structure_traversal_override_single(&st,
                                           STGoalDoubleMateReachedTester,
                                           &remember_goal_with_potential_king_capture);
  stip_structure_traversal_override_single(&st,
                                           STBrunnerDefenderFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STKingCaptureLegalityTester,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STMoveLegalityTester,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_king_captured(Side side)
{
  boolean result = false;

  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  for (curr = base; curr<top; ++curr)
    if (move_effect_journal[curr].type==move_effect_king_square_movement
        && move_effect_journal[curr].u.king_square_movement.side==side)
      result = move_effect_journal[curr].u.king_square_movement.to==initsquare;

  return result;
}

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
void own_king_capture_avoider_solve(slice_index si)
{
  Side const starter = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,is_king_captured(starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
void opponent_king_capture_avoider_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,is_king_captured(advers(SLICE_STARTER(si))));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
