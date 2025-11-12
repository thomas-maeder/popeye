#include "conditions/immobilio.h"
#include "conditions/circe/circe.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/stipulation.h"

#include "debugging/trace.h"

static boolean is_paralysed(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_departure = move_generation_stack[n].departure;
    piece_walk_type const walk = get_walk_of_piece_on_square(sq_departure);
    square const sq_rebirth = circe_regular_rebirth_square(walk,
                                                           sq_departure,
                                                           advers(trait[nbply]));

    TraceSquare(sq_departure);
    TraceWalk(walk);
    TraceSquare(sq_rebirth);
    TraceEOL();

    result = !is_square_empty(sq_rebirth);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observater according to Immobilio
 * @return true iff the observation is valid
 */
boolean immobilio_validate_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!is_paralysed(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param si identifies generator slice
 */
void immobilio_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!is_paralysed(current_generation))
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Immobilio
 * @param si identifies root slice of solving machinery
 */
void immobilio_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,nr_sides,STImmobilioMovesForPieceGenerator);

  stip_instrument_observer_validation(si,nr_sides,STImmobilioMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
