#include "conditions/series_capture.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_movement.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Instrument the solving machinery with Series Capture
 * @param si identifies entry slice into solving machinery
 */
void solving_instrument_series_capture(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSeriesCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void series_capture_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    if (post_move_am_i_iterating())
    {
      ++nbply;
      numecoup const curr = CURRMOVE_OF_PLY(nbply);
      move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
      square const sq_capture = move_gen_top->capture;
      square const sq_departure = move_gen_top->departure;
      square const sq_arrival = move_gen_top->arrival;
      --nbply;

      if (!is_no_capture(sq_capture))
        move_effect_journal_do_piece_removal(move_effect_reason_series_capture,
                                             sq_capture);
      move_effect_journal_do_piece_movement(move_effect_reason_series_capture,
                                            sq_departure,
                                            sq_arrival);
      post_move_iteration_solve_delegate(si);
      ++nbply;

      pop_move();
      if (encore())
        --nbply;
      else
      {
        post_move_iteration_end();
        finply();
      }
    }
    else
    {
      post_move_iteration_solve_delegate(si);
      if (solve_result==this_move_is_illegal)
        post_move_iteration_cancel();
      else
      {
        move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
        siblingply(SLICE_STARTER(si));
        generate_moves_for_piece(move_effect_journal[movement].u.piece_movement.to);
        --nbply;
      }
    }
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
