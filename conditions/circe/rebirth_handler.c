#include "conditions/circe/rebirth_handler.h"
#include "conditions/circe/capture_fork.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

pilecase current_circe_rebirth_square;
piece current_circe_reborn_piece[maxply+1];
Flags current_circe_reborn_spec[maxply+1];

/* Execute a Circe rebirth.
 * This is a helper function for alternative Circe types
 */
void circe_do_rebirth(move_effect_reason_type reason)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_addition(reason,
                                        current_circe_rebirth_square[nbply],
                                        current_circe_reborn_piece[nbply],
                                        current_circe_reborn_spec[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_determine_reborn_piece_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_circe_reborn_piece[nbply] = pprise[nbply];
  current_circe_reborn_spec[nbply] = pprispec[nbply];

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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_rebirth_handler_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_arrival = move_gen_top->arrival;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  Flags const spec_pi_moving = spec[sq_arrival];
  piece const pi_arriving = e[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (CondFlag[couscous])
    current_circe_rebirth_square[nbply] = (*circerenai)(pi_arriving,
                                                        spec_pi_moving,
                                                        sq_capture,
                                                        sq_departure,
                                                        sq_arrival,
                                                        advers(slices[si].starter));
  else
    current_circe_rebirth_square[nbply] = (*circerenai)(current_circe_reborn_piece[nbply],
                                                        current_circe_reborn_spec[nbply],
                                                        sq_capture,
                                                        sq_departure,
                                                        sq_arrival,
                                                        slices[si].starter);

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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_place_reborn_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[current_circe_rebirth_square[nbply]]==vide)
    circe_do_rebirth(move_effect_reason_circe_rebirth);
  else
    current_circe_rebirth_square[nbply] = initsquare;

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

/* Use an alternative type of slices for determining the reborn piece
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_circe_determine_reborn_piece(slice_index si,
                                               slice_type substitute)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,substitute,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&substitute);
  stip_structure_traversal_override_single(&st,STCirceDetermineRebornPiece,&replace);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STCirceRebirthHandler);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
