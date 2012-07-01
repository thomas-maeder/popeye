#include "conditions/disparate.h"
#include "pieces/attributes/neutral/initialiser.h"
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

/* Can a piece deliver check according to Disparate chess
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
boolean eval_disparate(square sq_departure, square sq_arrival, square sq_capture)
{
  boolean result = false;
  Side const save_trait = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  /* the following does not suffice if we have neutral kings,
     but we have no chance to recover the information who is to
     move from sq_departure, sq_arrival and sq_capture.
     TLi
  */
  if ((TSTFLAG(PieSpExFlags,Neutral)) && king_square[White]!=initsquare && TSTFLAG(spec[king_square[White]],Neutral))        /* will this do for neutral Ks? */
    trait[nbply] = neutral_side;
  else if (sq_capture==king_square[Black])
    trait[nbply] = White;
  else if (sq_capture==king_square[White])
    trait[nbply] = Black;
  else
    trait[nbply] = e[sq_departure]<0 ? Black : White;

  result = disparate_can_piece_move(sq_departure);

  trait[nbply] = save_trait;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  return result;
}
