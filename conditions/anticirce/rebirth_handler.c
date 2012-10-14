#include "conditions/anticirce/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "conditions/anticirce/capture_fork.h"
#include "debugging/trace.h"

#include <assert.h>

piece current_anticirce_reborn_piece[maxply+1];
Flags current_anticirce_reborn_spec[maxply+1];

piece current_anticirce_relevant_piece[maxply+1];
Flags current_anticirce_relevant_spec[maxply+1];
Side current_anticirce_relevant_side[maxply+1];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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

  current_anticirce_reborn_piece[nbply] = e[sq_arrival];
  current_anticirce_reborn_spec[nbply] = spec[sq_arrival];

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
stip_length_type anticirce_determine_relevant_piece_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_anticirce_relevant_piece[nbply] = current_anticirce_reborn_piece[nbply];
  current_anticirce_relevant_spec[nbply] = current_anticirce_reborn_spec[nbply];
  current_anticirce_relevant_side[nbply] = advers(slices[si].starter);

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

  current_anticirce_rebirth_square[nbply] = (*antirenai)(current_anticirce_relevant_piece[nbply],
                                                         current_anticirce_relevant_spec[nbply],
                                                         sq_capture,
                                                         sq_departure,
                                                         sq_arrival,
                                                         current_anticirce_relevant_side[nbply]);
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
stip_length_type anticirce_rebirth_handler_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_anticirce_rebirth_square[nbply]==initsquare)
    result = solve(slices[si].next1,n);
  else if ((!AntiCirCheylan
            && current_anticirce_rebirth_square[nbply]==move_generation_stack[current_move[nbply]].arrival)
           || e[current_anticirce_rebirth_square[nbply]]==vide)
  {
    move_effect_journal_do_piece_removal(move_effect_reason_anticirce_rebirth,
                                         sq_arrival);
    move_effect_journal_do_piece_addition(move_effect_reason_anticirce_rebirth,
                                          current_anticirce_rebirth_square[nbply],
                                          current_anticirce_reborn_piece[nbply],
                                          current_anticirce_reborn_spec[nbply]);
    result = solve(slices[si].next1,n);
  }
  else
  {
    current_anticirce_rebirth_square[nbply] = initsquare;
    result = slack_length-2;
  }

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
void stip_insert_anticirce_rebirth_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceDetermineRebornPiece);
  stip_instrument_moves(si,STAnticirceDetermineRevelantPiece);
  stip_instrument_moves(si,STAnticirceDetermineRebirthSquare);
  stip_instrument_moves(si,STAnticirceRebirthHandler);
  stip_insert_anticirce_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
