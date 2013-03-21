#include "conditions/eiffel.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/observation.h"
#include "pydata.h"

#include "debugging/trace.h"

#include <stdlib.h>

static piece get_paralyser(piece p)
{
  piece result = vide;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  switch (p)
  {
    case pb:
      result = dn;
      break;

    case db:
      result = tn;
      break;

    case tb:
      result = fn;
      break;

    case fb:
      result = cn;
      break;

    case cb:
      result = pn;
      break;

    case pn:
      result = db;
      break;

    case dn:
      result = tb;
      break;

    case tn:
      result = fb;
      break;

    case fn:
      result = cb;
      break;

    case cn:
      result = pb;
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
  piece p = e[sq];
  boolean result = true;
  piece eiffel_piece;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq],Neutral))
    p = -p;

  eiffel_piece = get_paralyser(p);

  if (eiffel_piece!=vide)
    result = (nbpiece[eiffel_piece]==0
              || !(*checkfunctions[abs(eiffel_piece)])(sq,
                                                       eiffel_piece,
                                                       observation_geometry_validator));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Can a piece deliver check according to Eiffel chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean eiffel_validate_observation(square sq_observer,
                                    square sq_landing,
                                    square sq_observee)
{
  piece const p = e[sq_observer];
  boolean result = true;
  piece eiffel_piece;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (TSTFLAG(PieSpExFlags,Neutral))
    initialise_neutrals(advers(neutral_side));

  eiffel_piece = get_paralyser(p);

  if (eiffel_piece!=vide)
    result = (nbpiece[eiffel_piece]==0
              || !(*checkfunctions[abs(eiffel_piece)])(sq_observer,
                                                       eiffel_piece,
                                                       observation_geometry_validator));

  if (TSTFLAG(spec[sq_observer],Neutral))
    initialise_neutrals(advers(neutral_side));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
