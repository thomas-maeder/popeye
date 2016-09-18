#include "conditions/beamten.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

static boolean is_unobserved_beamter(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[sq_departure],Beamtet))
    result = !is_square_observed_general(advers(trait[nbply]),sq_departure,EVALUATE(observer));
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Beamten Chess
 * @return true iff the observation is valid
 */
boolean beamten_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!is_unobserved_beamter(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void beamten_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!is_unobserved_beamter(current_generation))
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Beamten Chess
 * @param si identifies root slice of solving machinery
 */
void beamten_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STBeamtenMovesForPieceGenerator);

  stip_instrument_observation_validation(si,nr_sides,STBeamtenMovesForPieceGenerator);
  stip_instrument_check_validation(si,nr_sides,STBeamtenMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
