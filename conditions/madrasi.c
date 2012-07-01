#include "conditions/madrasi.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pydata.h"

#include "debugging/trace.h"

#include <stdlib.h>

static boolean is_ep_paralysed_on(piece p,
                                  square sq,
                                  checkfunction_t *checkfunc)
{
  /* Returns true if a pawn who has just crossed the square sq is
     paralysed by a piece p due to the ugly Madrasi-ep-rule by a
     pawn p.
     ---------------------------------------------------------
     Dear inventors of fairys:
     Make it as sophisticated and inconsistent as possible!
     ---------------------------------------------------------

     Checkfunc must be the corresponding checking function.

     pawn just moved        p       checkfunc
     --------------------------------------
     white pawn     pn      pioncheck
     black pawn     pb      pioncheck
     white berolina pawn  pbn     pbcheck
     black berolina pawn  pbb     pbcheck
  */

  ply const ply_dblstp = nbply-1;

  return (ep[ply_dblstp]==sq || ep2[ply_dblstp]==sq)
          && nbpiece[p]>0
          && (*checkfunc)(sq,
                          p,
                          flaglegalsquare ? legalsquare : eval_ortho);
}

static boolean is_ep_paralysed(piece p, square sq)
{
  boolean result = false;

  /* The ep capture needs special handling. */
  switch (p)
  {
    case pb: /* white pawn */
      if (is_ep_paralysed_on(pn, sq+dir_down,pioncheck))
        result = true;
      break;

    case pn: /* black pawn */
      if (is_ep_paralysed_on(pb, sq+dir_up,pioncheck))
        result = true;
      break;

    case pbb: /* white berolina pawn */
      if (is_ep_paralysed_on(pbn,sq+dir_down+dir_right,pbcheck)
          || is_ep_paralysed_on(pbn,sq+dir_down+dir_left,pbcheck))
        result = true;
      break;

    case pbn: /* black berolina pawn */
      if (is_ep_paralysed_on(pbb,sq+dir_up+dir_left,pbcheck)
          || is_ep_paralysed_on(pbb,sq+dir_up+dir_right,pbcheck))
        result = true;
      /* NB: Super (Berolina) pawns cannot neither be captured
       * ep nor capture ep themselves.
       */
      break;

    default:
      break;
  }

  return result;
}

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
    piece p = e[sq];

    if (TSTFLAG(spec[sq],Neutral))
      initialise_neutrals(advers(neutral_side));

    if (is_ep_paralysed(p,sq))
      result = true;
    else if (nbpiece[-p]==0)
      result = false;
    else
      result = (*checkfunctions[abs(p)])(sq,
                                         -p,
                                         (flaglegalsquare ? legalsquare : eval_ortho));

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
  piece p = e[sq];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (!rex_mad && (sq==king_square[White] || sq==king_square[Black]))
    result = true; /* nothing */
  else
  {
    if (TSTFLAG(spec[sq],Neutral))
      p = -p;

    if (is_ep_paralysed(p,sq))
      result = false;
    else if (nbpiece[-p]==0)
      result = true;
    else
      result = !(*checkfunctions[abs(p)])(sq,
                                          -p,
                                          (flaglegalsquare ? legalsquare : eval_ortho));
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
boolean eval_madrasi(square sq_departure, square sq_arrival, square sq_capture)
{
  if (flaglegalsquare
      && !legalsquare(sq_departure,sq_arrival,sq_capture)) {
    return false;
  }
  else {
    return (!madrasi_is_observed(sq_departure)
            &&  (!CondFlag[BGL] || eval_2(sq_departure,sq_arrival,sq_capture)));
    /* is this just appropriate for BGL? in verifieposition eval_2 is set when madrasi is true,
       but never seems to be used here or in libre */
  }
}
