#include "stipulation/move_player.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STMovePlayer slice.
 * @return index of allocated slice
 */
slice_index alloc_move_player_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMovePlayer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void play_move(void)
{
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  assert(sq_arrival!=nullsquare);

  pjoue[nbply] = e[sq_departure];
  pprise[nbply] = e[sq_capture];
  pprispec[nbply] = spec[sq_capture];

  if (e[sq_capture]==vide)
    move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                          sq_departure,sq_arrival);
  else
  {
    move_effect_journal_index_type const
      removal = move_effect_journal_do_piece_removal(move_effect_reason_regular_capture,
                                                     sq_capture);
    move_effect_journal_index_type const
      movement = move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                                       sq_departure,sq_arrival);
    move_effect_journal_link_capture_to_movement(removal,movement);
  }
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
stip_length_type move_player_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  play_move();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
