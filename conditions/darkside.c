#include "conditions/darkside.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "position/effects/side_change.h"
#include "position/effects/utils.h"
#include "position/position.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
void darkside_side_changer_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[movement].type==move_effect_piece_movement)
  {
    Flags const spec_moving = move_effect_journal[movement].u.piece_movement.movingspec;
    if (!TSTFLAG(spec_moving,Royal))
    {
   	  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
   	  unsigned int const row_arrival = (sq_arrival-square_a1)/onerow;
	  if (trait[nbply]==White)
	  {
	    if (row_arrival>=nr_rows_on_board/2)
	      move_effect_journal_do_side_change(move_effect_reason_darkside,sq_arrival);
	  }
	  else
      {
	    if (row_arrival<nr_rows_on_board/2)
		  move_effect_journal_do_side_change(move_effect_reason_darkside,sq_arrival);
	  }
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with Darkside
 */
void solving_insert_darkside(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDarksideSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
