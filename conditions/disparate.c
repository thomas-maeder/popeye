#include "conditions/disparate.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/observation.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <stdlib.h>

/* Can a piece on a particular square can move according to Disparate chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean disparate_can_piece_move(square sq)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (nbply>2
      && trait[nbply]!=trait[parent_ply[nbply]]
      && abs(e[sq])==abs(pjoue[parent_ply[nbply]]))
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean avoid_undisparate_observation(square sq_observer,
                                             square sq_landing,
                                             square sq_observee)
{
  boolean result = false;
  Side const save_trait = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  /* the following does not suffice if we have neutral kings,
     but we have no chance to recover the information who is to
     move from sq_departure, sq_arrival and sq_capture.
     TLi
  */
  /* will this do for neutral Ks? */
  if ((TSTFLAG(some_pieces_flags,Neutral)) && king_square[White]!=initsquare && TSTFLAG(spec[king_square[White]],Neutral))
    trait[nbply] = neutral_side;
  else if (sq_observee==king_square[Black])
    trait[nbply] = White;
  else if (sq_observee==king_square[White])
    trait[nbply] = Black;
  else
    trait[nbply] = e[sq_observer]<0 ? Black : White;

  result = disparate_can_piece_move(sq_observer);

  trait[nbply] = save_trait;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  return result;
}

/* Inialise solving in Disparate chess
 */
void disparate_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&avoid_undisparate_observation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
