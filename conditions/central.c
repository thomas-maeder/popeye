#include "conditions/central.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

static boolean are_we_validating_observer = false;

static boolean is_mover_supported_recursive(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture==king_square[trait[nbply]])
    result = true;
  else
    result = is_square_observed(EVALUATE(observer));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observer according to Central Chess
 * @return true iff the observation is valid
 */
boolean central_validate_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[si].next1);

  if (are_we_validating_observer && result)
  {
    replace_observation_target(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
    result = is_mover_supported_recursive();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_mover_supported(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(!are_we_validating_observer);

  siblingply(trait[nbply]);
  push_observation_target(move_generation_stack[n].departure);
  are_we_validating_observer = true;
  result = is_mover_supported_recursive();
  are_we_validating_observer = false;
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Central Chess
 * @return true iff the observation is valid
 */
boolean central_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (is_mover_supported(CURRMOVE_OF_PLY(nbply))
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void central_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (is_mover_supported(current_generation))
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Central Chess
 * @param si identifies root slice of solving machinery
 */
void central_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STCentralMovesForPieceGenerator);

  stip_instrument_observation_validation(si,nr_sides,STCentralObservationValidator);
  stip_instrument_check_validation(si,nr_sides,STCentralObservationValidator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
