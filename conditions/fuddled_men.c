#include "conditions/fuddled_men.h"
#include "solving/pipe.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/king_capture_avoider.h"
#include "stipulation/move.h"

static PieceIdType fuddled[nr_sides] = { NullPieceId, NullPieceId };

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
void fuddled_men_bookkeeper_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  Flags const spec_moving = move_effect_journal[movement].u.piece_movement.movingspec;
  Side const side_moving = SLICE_STARTER(si);
  PieceIdType const save_fuddled = fuddled[side_moving];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  fuddled[side_moving] = GetPieceId(spec_moving);

  TraceEnumerator(Side,side_moving);
  TraceValue("%u",fuddled[side_moving]);
  TraceEOL();

  pipe_solve_delegate(si);

  fuddled[side_moving] = save_fuddled;

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
void fuddled_men_generate_moves_for_piece(slice_index si)
{
  Side const side_generating = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_generating);
  TraceValue("%u",fuddled[side_generating]);
  TraceValue("%u",GetPieceId(being_solved.spec[curr_generation->departure]));
  TraceEOL();

  if (GetPieceId(being_solved.spec[curr_generation->departure])
      !=fuddled[side_generating])
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation according to Fuddled Men
 * @return true iff the observation is valid
 */
boolean fuddled_men_inverse_validate_observation(slice_index si)
{
  boolean result;
  square const pos_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  PieceIdType id_observer = GetPieceId(being_solved.spec[pos_observer]);
  Side const side_observing = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_observing);
  TraceSquare(pos_observer);
  TraceValue("%u",fuddled[side_observing]);
  TraceValue("%u",id_observer);
  TraceEOL();

  if (fuddled[side_observing]==id_observer)
    result = false;
  else
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery for CAST
 * @param si identifies root slice of stipulation
 */
void fuddled_men_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,nr_sides,STFuddledMenMovesForPieceGenerator);
  stip_instrument_moves(si,STFuddledMenBookkeeper);
  stip_instrument_check_validation(si,nr_sides,STFuddledMenInverseValidateObservation);

  solving_insert_king_capture_avoiders(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
