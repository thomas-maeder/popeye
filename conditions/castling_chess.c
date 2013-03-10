#include "conditions/castling_chess.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/castling.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

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
stip_length_type castling_chess_move_player_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture>platzwechsel_rochade)
  {
    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;
    square const sq_passed = (sq_departure+sq_arrival) / 2;

    assert(sq_arrival!=nullsquare);

    pjoue[nbply] = e[sq_departure];
    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                          sq_departure,sq_arrival);
    move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                          sq_capture-maxsquare,
                                          sq_passed);

    result = solve(slices[si].next2,n);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_castling_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  insert_alternative_move_players(si,STCastlingChessMovePlayer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
