#include "conditions/anticirce/magic_square.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "position/effects/utils.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
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
void magic_square_anticirce_relevant_side_adapter_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!TSTFLAG(context->relevant_spec,Royal))
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    if (TSTFLAG(sq_spec(pos),MagicSq))
    {
      context->relevant_side = SLICE_STARTER(si);
      context->rebirth_as = advers(context->rebirth_as);
    }
  }

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with magic square side changers
 */
void magic_square_type2_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  circe_instrument_solving(si,
                           STAnticirceConsideringRebirth,
                           STCirceDeterminingRebirth,
                           alloc_pipe(STMagicSquareType2AnticirceRelevantSideAdapter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
