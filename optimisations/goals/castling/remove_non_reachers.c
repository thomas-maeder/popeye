#include "optimisations/goals/castling/remove_non_reachers.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "debugging/trace.h"

/* Allocate a STCastlingRemoveNonReachers slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_remove_non_reachers_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCastlingRemoveNonReachers);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type castling_remove_non_reachers_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  numecoup i;
  numecoup new_top = current_move[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = current_move[nbply-1]+1; i<=current_move[nbply]; ++i)
  {
    square const sq_capture = move_generation_stack[i].capture;

    if (sq_capture==kingside_castling || sq_capture==queenside_castling)
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }
  }

  current_move[nbply] = new_top;

  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
