#include "conditions/circe/kamikaze.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "conditions/anticirce/rebirth_handler.h"
#include "conditions/anticirce/capture_fork.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_kamikaze_capture_fork_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_arrival],Kamikaze))
    result = anticirce_capture_fork_solve(si,n);
  else
    result = solve(slices[si].next2,n);

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
stip_length_type circe_kamikaze_rebirth_handler_solve(slice_index si,
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

  current_anticirce_rebirth_square[nbply] = (*circerenai)(current_anticirce_relevant_piece[nbply],
                                                          current_anticirce_relevant_spec[nbply],
                                                          sq_capture,
                                                          sq_departure,
                                                          sq_arrival,
                                                          current_anticirce_relevant_side[nbply]);

  move_effect_journal_do_piece_removal(move_effect_reason_kamikaze_capturer,
                                       sq_arrival);

  if (e[current_anticirce_rebirth_square[nbply]]==vide)
    move_effect_journal_do_piece_addition(move_effect_reason_circe_rebirth,
                                          current_anticirce_rebirth_square[nbply],
                                          current_anticirce_reborn_piece[nbply],
                                          current_anticirce_reborn_spec[nbply]);
  else
    current_anticirce_rebirth_square[nbply] = initsquare;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void replace(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  slices[si].type = STCirceKamikazeCaptureFork;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_kamikaze_rebirth_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceDetermineRebornPiece);
  stip_instrument_moves(si,STAnticirceDetermineRevelantPiece);
  stip_instrument_moves(si,STCirceKamikazeRebirthHandler);
  stip_insert_anticirce_capture_forks(si);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STAnticirceCaptureFork,&replace);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
