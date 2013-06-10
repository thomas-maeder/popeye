#include "conditions/eiffel.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/observation.h"
#include "pydata.h"

#include "debugging/trace.h"

#include <stdlib.h>

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

/* Can a piece on a particular square can move according to Eiffel chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean eiffel_can_piece_move(square sq)
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
      nextply();
      trait[nbply] = eiffel_side;
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

  if (TSTFLAG(some_pieces_flags,Neutral))
    initialise_neutrals(advers(neutral_side));

  eiffel_piece = get_paralyser(p);

  if (eiffel_piece!=Empty)
  {
    Side const eiffel_side = advers(trait[nbply]);
    if (number_of_pieces[eiffel_side][eiffel_piece]>0)
    {
      nextply();
      trait[nbply] = eiffel_side;
      result = !(*checkfunctions[eiffel_piece])(sq_observer,
                                                eiffel_piece,
                                                &validate_observation_geometry);
      finply();
    }
  }

  if (TSTFLAG(spec[sq_observer],Neutral))
    initialise_neutrals(advers(neutral_side));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Eiffel Chess
 */
void eiffel_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observer_validator(&avoid_observation_by_paralysed);
  register_observation_validator(&avoid_observation_by_paralysed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
