#include "optimisations/orthodox_mating_moves/king_contact_move_generator.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/move_diff_code.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type orthodox_mating_king_contact_generator_solve(slice_index si,
                                                              stip_length_type n)
{
  stip_length_type result;
  Side const moving = slices[si].starter;
  Side const mated = advers(moving);
  square const sq_mated_king = king_square[mated];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length+1);

  curr_generation->departure = king_square[moving];

  if (curr_generation->departure!=sq_mated_king)
  {
    vec_index_type k;
    for (k = vec_queen_start; k<=vec_queen_end; k++)
    {
      curr_generation->arrival = curr_generation->departure+vec[k];
      if ((is_square_empty(curr_generation->arrival)
          || TSTFLAG(spec[curr_generation->arrival],mated))
          && move_diff_code[abs(sq_mated_king-curr_generation->arrival)]<=1+1)
        push_move();
    }
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
