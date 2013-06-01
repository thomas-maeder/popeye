#include "conditions/madrasi.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pydata.h"
#include "solving/observation.h"

#include "debugging/trace.h"

#include <stdlib.h>

boolean madrasi_is_observed(square sq)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (!rex_mad && (sq==king_square[White] || sq==king_square[Black]))
    result = false;
  else
  {
    piece const p = e[sq];
    Side const observed_side = p>0 ? White : Black;
    Side const observing_side = advers(observed_side);

    if (TSTFLAG(some_pieces_flags,Neutral))
      initialise_neutrals(advers(neutral_side));

    if (number_of_pieces[observing_side][abs(p)]==0)
      result = false;
    else
    {
      nextply();
      trait[nbply] = observing_side;
      result = (*checkfunctions[abs(p)])(sq,abs(p),&validate_observation_geometry);
      finply();
    }

    if (TSTFLAG(spec[sq],Neutral))
      initialise_neutrals(advers(neutral_side));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Can a piece on a particular square can move according to Madrasi?
 * @param sq position of piece
 * @return true iff the piece can move according to Madrasi
 */
boolean madrasi_can_piece_move(square sq)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (!rex_mad && sq==king_square[trait[nbply]])
    result = true;
  else
  {
    PieNam const candidate = abs(e[sq]);

    trait[nbply] = advers(trait[nbply]);
    result = (number_of_pieces[trait[nbply]][candidate]==0
              || !(*checkfunctions[candidate])(sq,
                                               candidate,
                                               &validate_observation_geometry));
    trait[nbply] = advers(trait[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Can a piece deliver check according to Madrasi
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
static boolean avoid_observation_by_paralysed(square sq_observer,
                                              square sq_landing,
                                              square sq_observee)
{
  return !madrasi_is_observed(sq_observer);
}

/* Inialise solving in Madrasi
 */
void madrasi_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observer_validator(&avoid_observation_by_paralysed);
  register_observation_validator(&avoid_observation_by_paralysed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
