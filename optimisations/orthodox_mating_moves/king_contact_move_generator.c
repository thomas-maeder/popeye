#include "optimisations/orthodox_mating_moves/king_contact_move_generator.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "position/move_diff_code.h"
#include "position/position.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

/* Allocate a STOrthodoxMatingKingContactGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_orthodox_mating_king_contact_generator_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOrthodoxMatingKingContactGenerator);

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
void orthodox_mating_king_contact_generator_solve(slice_index si)
{
  Side const moving = SLICE_STARTER(si);
  Side const mated = advers(moving);
  square const sq_mated_king = being_solved.king_square[mated];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining==slack_length+1);

  curr_generation->departure = being_solved.king_square[moving];

  if (curr_generation->departure!=sq_mated_king)
  {
    vec_index_type k;
    for (k = vec_queen_start; k<=vec_queen_end; k++)
    {
      curr_generation->arrival = curr_generation->departure+vec[k];
      if ((is_square_empty(curr_generation->arrival)
          || TSTFLAG(being_solved.spec[curr_generation->arrival],mated))
          && move_diff_code[abs(sq_mated_king-curr_generation->arrival)]<=1+1)
        push_move();
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
