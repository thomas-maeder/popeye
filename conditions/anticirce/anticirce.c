#include "conditions/anticirce/anticirce.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "conditions/anticirce/capture_fork.h"
#include "conditions/magic_square.h"
#include "debugging/trace.h"

#include <assert.h>

piece anticirce_current_reborn_piece[maxply+1];
Flags anticirce_current_reborn_spec[maxply+1];

piece anticirce_current_relevant_piece[maxply+1];
Flags anticirce_current_relevant_spec[maxply+1];
Side anticirce_current_relevant_side[maxply+1];

/* Validate an observation according to Anti-Circe
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean anticirce_validate_observation(square sq_observer,
                                       square sq_landing,
                                       square sq_observee)
{
  boolean result;
  Side const side_observing = e[sq_observer]>vide ? White : Black;
  Side side_observed = advers(side_observing);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (CondFlag[antisuper])
  {
    square const *bnp= boardnum;
    while (!LegalAntiCirceMove(*bnp, sq_observee, sq_observer) && *bnp)
      bnp++;
    result = *bnp && LegalAntiCirceMove(*bnp,sq_observee,sq_observer);
  }
  else
  {
    PieNam* acprompieces;

    if (!TSTFLAG(spec[sq_observer],Royal)
        && TSTFLAG(sq_spec[sq_observee],MagicSq)
        && magic_square_type==magic_square_type2)
      side_observed = advers(side_observed);

    acprompieces= GetPromotingPieces(sq_observer,
                                     e[sq_observer],
                                     advers(side_observed),
                                     spec[sq_observer],
                                     sq_landing,
                                     e[sq_observee]);
    if (acprompieces)
    {
      /* Pawn checking on last rank or football check on a/h file */
      PieNam pprom= acprompieces[vide];
      square    cren;
      do {
        cren= (*antirenai)(pprom, spec[sq_observer], sq_observee, sq_observer, sq_landing, side_observed);
        pprom= acprompieces[pprom];
      } while (!LegalAntiCirceMove(cren, sq_observee, sq_observer) && pprom != Empty);

      result = LegalAntiCirceMove(cren,sq_observee,sq_observer) || pprom!=Empty;
    }
    else
    {
      square cren= (*antirenai)(TSTFLAG(spec[sq_observer], Chameleon)
                                ? champiece(e[sq_observer])
                                : e[sq_observer],
                                spec[sq_observer], sq_observee, sq_observer, sq_landing, side_observed);
      result = LegalAntiCirceMove(cren,sq_observee,sq_observer);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_determine_reborn_piece_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  anticirce_current_reborn_piece[nbply] = e[sq_arrival];
  anticirce_current_reborn_spec[nbply] = spec[sq_arrival];

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_determine_relevant_piece_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  anticirce_current_relevant_piece[nbply] = anticirce_current_reborn_piece[nbply];
  anticirce_current_relevant_spec[nbply] = anticirce_current_reborn_spec[nbply];
  anticirce_current_relevant_side[nbply] = advers(slices[si].starter);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_determine_rebirth_square_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_anticirce_rebirth_square[nbply] = (*antirenai)(anticirce_current_relevant_piece[nbply],
                                                         anticirce_current_relevant_spec[nbply],
                                                         sq_capture,
                                                         sq_departure,
                                                         sq_arrival,
                                                         anticirce_current_relevant_side[nbply]);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void replace(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const substitute = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  slices[si].type = *substitute;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Use an alternative type of slices for determining the piece relevant for
 * determining the rebirth square
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_anticirce_determine_relevant_piece(slice_index si,
                                                     slice_type substitute)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,substitute,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&substitute);
  stip_structure_traversal_override_single(&st,STAnticirceDetermineRevelantPiece,&replace);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceDetermineRebornPiece);
  stip_instrument_moves(si,STAnticirceDetermineRevelantPiece);
  stip_instrument_moves(si,STAnticirceDetermineRebirthSquare);
  stip_insert_anticirce_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
