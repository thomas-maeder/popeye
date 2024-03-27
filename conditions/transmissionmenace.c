#include "conditions/transmissionmenace.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/check.h"

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
void transmissionmenace_generate_moves_for_piece(slice_index si)
{
  numecoup const base = MOVEBASE_OF_PLY(nbply+1);
  piece_walk_type p;
  Side const observed_side = trait[nbply];
  Side const observing_side = advers(observed_side);
  piece_walk_type observing_walks[nr_piece_walks];
  unsigned int nr_observing_walks = 0;
  unsigned int curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  for (p = King+1; p<nr_piece_walks; ++p)
    if (piece_walk_exists[p])
    {
      nextply(observing_side);
      push_observation_target(curr_generation->departure);
      observing_walk[nbply] = p;
      if (fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed_specific[trait[nbply]],
                                                  EVALUATE(observation_geometry)))
      {
        assert(nr_observing_walks<nr_piece_walks);
        observing_walks[nr_observing_walks] = p;
        ++nr_observing_walks;
      }
      finply();
    }

  for (curr = 0; curr<nr_observing_walks; ++curr)
  {
    move_generation_current_walk = observing_walks[curr];
    pipe_solve_delegate(si);
  }

  remove_duplicate_moves_of_single_piece(base);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for TransmissionMenace
 * @param si identifies root slice of stipulation
 */
void transmissionmenace_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,
                                                nr_sides,
                                                STTransmissionMenaceMovesForPieceGenerator);
  solving_test_check_playing_moves(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
