#include "conditions/eiffel.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "pydata.h"

#include "debugging/trace.h"

static PieNam get_paralyser(PieNam p)
{
  PieNam result = Empty;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  switch (p)
  {
    case Pawn:
      result = Queen;
      break;

    case Queen:
      result = Rook;
      break;

    case Rook:
      result = Bishop;
      break;

    case Bishop:
      result = Knight;
      break;

    case Knight:
      result = Pawn;
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TracePiece(result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_paralysed(square sq)
{
  PieNam const p = get_walk_of_piece_on_square(sq);
  boolean result = false;
  PieNam eiffel_piece;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  eiffel_piece = get_paralyser(p);

  if (eiffel_piece!=Empty)
  {
    Side const eiffel_side = advers(trait[nbply]);
    if (number_of_pieces[eiffel_side][eiffel_piece]>0)
    {
      nextply(eiffel_side);
      result = (*checkfunctions[eiffel_piece])(sq,
                                               eiffel_piece,
                                               &validate_observation_geometry);
      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void eiffel_generate_moves_for_piece(slice_index si,
                                     square sq_departure,
                                     PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!is_paralysed(sq_departure))
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observater according to Eiffel Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean eiffel_validate_observer(slice_index si,
                                 square sq_observer,
                                 square sq_landing,
                                 square sq_observee)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = (!is_paralysed(sq_observer)
            && validate_observer_recursive(slices[si].next1,
                                           sq_observer,
                                           sq_landing,
                                           sq_observee));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise the solving machinery with Eiffel Chess
 * @param si identifies root slice of solving machinery
 */
void eiffel_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STEiffelMovesForPieceGenerator);

  stip_instrument_observer_validation(si,nr_sides,STEiffelObserverTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
