#include "conditions/powertransfer.h"
#include "conditions/circe/circe.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/check.h"
#include "position/position.h"

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
void powertransfer_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!TSTFLAG(being_solved.spec[curr_generation->departure],Royal))
  {
    square const sq_rebirth = circe_regular_rebirth_square(move_generation_current_walk,
                                                           curr_generation->departure,
                                                           advers(trait[nbply]));

    TraceWalk(move_generation_current_walk);
    TraceSquare(curr_generation->departure);
    TraceEnumerator(Side,trait[nbply]);
    TraceSquare(sq_rebirth);
    TraceEOL();

    if (!is_square_empty(sq_rebirth))
      move_generation_current_walk = get_walk_of_piece_on_square(sq_rebirth);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for PowerTransfer
 * @param si identifies root slice of stipulation
 */
void powertransfer_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,
                                                nr_sides,
                                                STPowerTransferMovesForPieceGenerator);
  solving_test_check_playing_moves(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
