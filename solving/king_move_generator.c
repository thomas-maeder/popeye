#include "solving/king_move_generator.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/temporary_hacks.h"
#include "position/position.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STKingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_king_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKingMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void king_move_generator_solve(slice_index si)
{
  Side const side_at_move = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[being_solved.king_square[side_at_move]],Royal))
    generate_moves_for_piece(being_solved.king_square[side_at_move]);
  else
  {
    /* - there is no being_solved.king_square, or
     * - being_solved.king_square is a royal square */
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
