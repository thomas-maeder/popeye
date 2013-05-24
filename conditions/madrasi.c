#include "conditions/madrasi.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pydata.h"
#include "solving/en_passant.h"
#include "solving/observation.h"
#include "conditions/einstein/en_passant.h"

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
     white berolina pawn  pbn     berolina_pawn_check
     black berolina pawn  pbb     berolina_pawn_check
  */

  ply const ply_dblstp = parent_ply[nbply];
  Side const side = p>0 ? White : Black;

  return (en_passant_is_capture_possible_to(sq) || einstein_ep[ply_dblstp]==sq)
          && number_of_pieces[side][abs(p)]>0
          && (*checkfunc)(sq,p,&validate_observation_geometry);
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
      if (is_ep_paralysed_on(pbn,sq+dir_down+dir_right,berolina_pawn_check)
          || is_ep_paralysed_on(pbn,sq+dir_down+dir_left,berolina_pawn_check))
        result = true;
      break;

    case pbn: /* black berolina pawn */
      if (is_ep_paralysed_on(pbb,sq+dir_up+dir_left,berolina_pawn_check)
          || is_ep_paralysed_on(pbb,sq+dir_up+dir_right,berolina_pawn_check))
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
    piece const p = e[sq];
    Side const side = p>0 ? White : Black;

    if (TSTFLAG(some_pieces_flags,Neutral))
      initialise_neutrals(advers(neutral_side));

    if (is_ep_paralysed(p,sq))
      result = true;
    else if (number_of_pieces[advers(side)][abs(p)]==0)
      result = false;
    else
      result = (*checkfunctions[abs(p)])(sq,-p,&validate_observation_geometry);

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
    else
    {
      Side const side = p>0 ? White : Black;
      if (number_of_pieces[advers(side)][abs(p)]==0)
        result = true;
      else
        result = !(*checkfunctions[abs(p)])(sq,-p,&validate_observation_geometry);
    }
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
