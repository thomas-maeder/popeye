#include "solving/non_king_move_generator.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/temporary_hacks.h"
#include "position/position.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STNonKingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_non_king_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STNonKingMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean advance_departure_square(square const **next_square_to_try)
{
  while (true)
  {
    square const sq_departure = **next_square_to_try;
    if (sq_departure==0)
      break;
    else
    {
      ++*next_square_to_try;

      if (TSTFLAG(being_solved.spec[sq_departure],trait[nbply])
        /* don't use king_square[side] - it may be a royal square occupied
         * by a non-royal piece! */
             && !TSTFLAG(being_solved.spec[sq_departure],Royal))
      {
        generate_moves_for_piece(sq_departure);
        return true;
      }
    }
  }

  return false;
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
void non_king_move_generator_solve(slice_index si)
{
  square const *next_square_to_try = boardnum;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_result = immobility_on_next_move;

  nextply(SLICE_STARTER(si));

  while (solve_result<slack_length
         && advance_departure_square(&next_square_to_try))
    pipe_solve_delegate(si);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
