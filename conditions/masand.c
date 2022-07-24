#include "conditions/masand.h"
#include "pieces/attributes/neutral/neutral.h"
#include "position/effects/side_change.h"
#include "position/effects/utils.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/move.h"
#include "solving/pipe.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

static square current_observer_pos[maxply+1];

/* Validate an observation or observer by making sure it's the checking piece
 * @param si identifies the validator slice
 */
boolean masand_enforce_observer(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observer = current_observer_pos[parent_ply[nbply]];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = ((sq_observer==initsquare || sq_observer==sq_departure)
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean observed(square on_this, square by_that)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(on_this);
  TraceSquare(by_that);
  TraceFunctionParamListEnd();

  current_observer_pos[parent_ply[nbply]] = by_that;

  result = is_square_observed_general(trait[nbply],on_this,EVALUATE(observation));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_recoloring_candidate(square observer_pos, square observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(observer_pos);
  TraceSquare(observee);
  TraceFunctionParamListEnd();

  result = (!is_square_empty(observee)
            && !TSTFLAG(being_solved.spec[observee],Royal)
            && observee!=observer_pos
            && !is_piece_neutral(being_solved.spec[observee])
            && observed(observee,observer_pos));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void change_observed(square observer_pos)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceSquare(observer_pos);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (is_recoloring_candidate(observer_pos,*bnp))
      move_effect_journal_do_side_change(move_effect_reason_masand,*bnp);

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
void masand_recolorer_solve(slice_index si)
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
    if (is_in_check(opponent) && observed(being_solved.king_square[opponent],pos))
      change_observed(pos);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_masand(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMasandRecolorer);
  stip_instrument_observation_validation(si,nr_sides,STMasandEnforceObserver);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean has_already_been_recolored(square s)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  for (curr = base; curr<top; ++curr)
    if (move_effect_journal[curr].type==move_effect_side_change
        && move_effect_journal[curr].reason==move_effect_reason_masand
        && move_effect_journal[curr].u.side_change.on==s)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void change_observed_once(square observer_pos)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceSquare(observer_pos);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (is_recoloring_candidate(observer_pos,*bnp)
        && !has_already_been_recolored(*bnp))
      move_effect_journal_do_side_change(move_effect_reason_masand,*bnp);

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
void masand_generalised_recolorer_solve(slice_index si)
{
  Side const side_delivering_check = SLICE_STARTER(si);
  Side const side_in_check = advers(side_delivering_check);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_in_check(side_in_check))
  {
    square const *pos;
    for (pos = boardnum; *pos; ++pos)
      if (TSTFLAG(being_solved.spec[*pos],side_delivering_check)
          && observed(being_solved.king_square[side_in_check],*pos))
        change_observed_once(*pos);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_masand_generalised(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMasandGeneralisedRecolorer);
  stip_instrument_observation_validation(si,nr_sides,STMasandEnforceObserver);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
