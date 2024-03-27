#include "conditions/fuddled_men.h"
#include "position/effects/utils.h"
#include "solving/pipe.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/king_capture_avoider.h"
#include "stipulation/move.h"
#include "debugging/assert.h"

fuddled_state_per_side_type fuddled[nr_sides];

static boolean is_fuddled(Side side, square pos)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  for (i = 0; i<fuddled_men_max_nr_per_side; ++i)
    if (pos==fuddled[side].pos[i])
      result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
void fuddled_men_bookkeeper_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type curr;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  Side const side_moving = SLICE_STARTER(si);
  fuddled_state_per_side_type const save_fuddled = fuddled[side_moving];
  unsigned int nr_fuddled = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (curr = movement; curr<top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_moving_piece_movement:
          case move_effect_reason_castling_king_movement:
          case move_effect_reason_castling_partner:
          {
            square const pos_moving = move_effect_journal[curr].u.piece_movement.to;
            Flags const spec_moving = move_effect_journal[curr].u.piece_movement.movingspec;
            assert(nr_fuddled<fuddled_men_max_nr_per_side);
            fuddled[side_moving].pos[nr_fuddled] = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                                                          GetPieceId(spec_moving),
                                                                                                          pos_moving);
            ++nr_fuddled;
            break;
          }

          default:
            break;
        }
        break;
      }

      default:
        break;
    }

  for (; nr_fuddled<fuddled_men_max_nr_per_side; ++nr_fuddled)
    fuddled[side_moving].pos[nr_fuddled] = initsquare;

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
  TraceSquare(curr_generation->departure);
  TraceEOL();

  if (!is_fuddled(side_generating,curr_generation->departure))
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
  Side const side_observing = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_observing);
  TraceSquare(pos_observer);
  TraceValue("%u",fuddled[side_observing]);
  TraceValue("%u",id_observer);
  TraceEOL();

  if (is_fuddled(side_observing,pos_observer))
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
