#include "conditions/eiffel.h"
#include "pieces/attributes/neutral/initialiser.h"
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

static boolean can_piece_check(square sq)
{
  piece const p = e[sq];
  boolean result = true;
  piece eiffel_piece;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (TSTFLAG(PieSpExFlags,Neutral))
    initialise_neutrals(advers(neutral_side));

  eiffel_piece = get_paralyser(p);

  if (eiffel_piece!=vide)
    result = (nbpiece[eiffel_piece]==0
              || !(*checkfunctions[abs(eiffel_piece)])(sq,
                                                       eiffel_piece,
                                                       (flaglegalsquare
                                                        ? legalsquare
                                                        : eval_ortho)));

  if (TSTFLAG(spec[sq],Neutral))
    initialise_neutrals(advers(neutral_side));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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
                                                       (flaglegalsquare
                                                        ? legalsquare
                                                        : eval_ortho)));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Can a piece deliver check according to Eiffel chess
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
boolean eval_eiffel(square sq_departure, square sq_arrival, square sq_capture)
{
  if (flaglegalsquare
      && !legalsquare(sq_departure,sq_arrival,sq_capture))
    return false;
  else
    return (can_piece_check(sq_departure)
            && (!CondFlag[BGL] || eval_2(sq_departure,sq_arrival,sq_capture)));
    /* is this just appropriate for BGL? in verifieposition eval_2 is set when madrasi is true,
       but never seems to be used here or in libre */
}
