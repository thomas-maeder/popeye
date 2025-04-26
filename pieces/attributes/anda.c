#include "pieces/attributes/anda.h"
#include "position/effects/flags_change.h"
#include "position/effects/utils.h"
#include "position/position.h"
#include "conditions/masand.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/has_solution_type.h"
#include "solving/check.h"
#include "solving/observation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "pieces/pieces.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

static void change_observer(square observer_pos)
{
  Flags spec = being_solved.spec[observer_pos];

  TraceFunctionEntry(__func__);
  TraceSquare(observer_pos);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec,White))
  {
    if (TSTFLAG(spec,Black))
      CLRFLAG(spec,advers(trait[nbply]));
    else
      SETFLAG(spec,Black);
  }
  else
  {
    assert(TSTFLAG(spec,Black));
    SETFLAG(spec,White);
  }

  move_effect_journal_do_flags_change(move_effect_reason_anda,observer_pos,spec);

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
void anda_recolorer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    Side const opponent = advers(SLICE_STARTER(si));
    if (TSTFLAG(being_solved.spec[pos],Anda)
        && is_in_check(opponent) && masand_observed(being_solved.king_square[opponent],pos))
      change_observer(pos);
  }

  pipe_solve_delegate(si);

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
void anda_inverse_recolorer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    Side const opponent = advers(SLICE_STARTER(si));
    if (TSTFLAG(being_solved.spec[pos],AndaInverse)
        && !(is_in_check(opponent) && masand_observed(being_solved.king_square[opponent],pos)))
      change_observer(pos);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_anda(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAndaRecolorer);
  stip_instrument_observation_validation(si,nr_sides,STMasandEnforceObserver);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_anda_inverse(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAndaInverseRecolorer);
  stip_instrument_observation_validation(si,nr_sides,STMasandEnforceObserver);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
