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

static boolean can_piece_move(square sq)
{
  PieNam const p = get_walk_of_piece_on_square(sq);
  boolean result = true;
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
      result = !(*checkfunctions[eiffel_piece])(sq,
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

  if (can_piece_move(sq_departure))
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean avoid_observation_by_paralysed(square sq_observer,
                                              square sq_landing,
                                              square sq_observee)
{
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  boolean result = true;
  PieNam eiffel_piece;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  eiffel_piece = get_paralyser(p);

  if (eiffel_piece!=Empty)
  {
    Side const eiffel_side = advers(trait[nbply]);
    if (number_of_pieces[eiffel_side][eiffel_piece]>0)
    {
      nextply(eiffel_side);
      result = !(*checkfunctions[eiffel_piece])(sq_observer,
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

/* Inialise the solving machinery with Eiffel Chess
 * @param si identifies root slice of solving machinery
 */
void eiffel_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STEiffelMovesForPieceGenerator);

  register_observer_validator(&avoid_observation_by_paralysed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
