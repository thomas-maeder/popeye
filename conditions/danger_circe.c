#include "conditions/danger_circe.h"
#include "conditions/circe/circe.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "solving/check.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

static boolean is_unobserved(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  result = !is_square_observed_general(advers(trait[nbply]),sq_departure,EVALUATE(observer));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Beamten Chess
 * @return true iff the observation is valid
 */
boolean danger_circe_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!is_unobserved(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void danger_circe_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_observed_general(advers(trait[nbply]),
                                 curr_generation->departure,
                                 EVALUATE(observer)))
  {
    piece_walk_type const walk_moving = being_solved.board[curr_generation->departure];
    curr_generation->arrival = circe_regular_rebirth_square(walk_moving,
                                                            curr_generation->departure,
                                                            advers(trait[nbply]));

    if (is_square_empty(curr_generation->arrival))
      push_move_no_capture();
    else if (TSTFLAG(being_solved.spec[curr_generation->arrival],advers(trait[nbply])))
      push_move_regular_capture();
  }

  pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Danger Circe
 * @param si identifies root slice of solving machinery
 */
void danger_circe_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,nr_sides,STDangerCirceMovesForPieceGenerator);

//  stip_instrument_observation_validation(si,nr_sides,STBeamtenMovesForPieceGenerator);
//  stip_instrument_check_validation(si,nr_sides,STBeamtenMovesForPieceGenerator);
//  stip_instrument_check_validation(si,
//                                   nr_sides,
//                                   STValidateCheckMoveByPlayingCapture);
  solving_test_check_playing_moves(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
