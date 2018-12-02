#include "conditions/oscillating_kings.h"
#include "conditions/conditions.h"
#include "position/king_square.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_effect_journal.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

ConditionLetteredVariantType OscillatingKings[nr_sides];

static void perform_oscillation(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_exchange(move_effect_reason_oscillating_kings,
                                        being_solved.king_square[White],
                                        being_solved.king_square[Black]);

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
void oscillating_kings_type_a_solve(slice_index si)
{
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_king_squares();
  perform_oscillation();
  pipe_solve_delegate(si);

  king_square_horizon = save_horizon;

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
void oscillating_kings_type_b_solve(slice_index si)
{
  Side const starter = SLICE_STARTER(si);
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_king_squares();

  if (is_in_check(starter))
    solve_result = this_move_is_illegal;
  else
  {
    perform_oscillation();
    pipe_solve_delegate(si);
  }

  king_square_horizon = save_horizon;

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
void oscillating_kings_type_c_solve(slice_index si)
{
  Side const starter = SLICE_STARTER(si);
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_king_squares();

  if (is_in_check(advers(starter)))
    perform_oscillation();

  pipe_solve_delegate(si);

  king_square_horizon = save_horizon;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  Side const starter = SLICE_STARTER(si);
  Cond const cond = starter==White ? white_oscillatingKs : black_oscillatingKs;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (CondFlag[cond])
  {
    slice_type type;
    if (OscillatingKings[starter]==ConditionTypeB)
      type = STOscillatingKingsTypeB;
    else if (OscillatingKings[starter]==ConditionTypeC)
      type = STOscillatingKingsTypeC;
    else
      type = STOscillatingKingsTypeA;

    {
      slice_index const prototype = alloc_pipe(type);
      move_insert_slices(si,st->context,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_king_oscillators(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_impose_starter(si,SLICE_STARTER(si));

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
