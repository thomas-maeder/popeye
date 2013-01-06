#include "output/plaintext/tree/zugzwang_writer.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "output/plaintext/tree/check_writer.h"
#include "debugging/trace.h"

/* Allocate a STZugzwangWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_zugzwang_writer_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STZugzwangWriter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type zugzwang_writer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (nbply==2)
    /* option postkey is set - write "threat:" or "zugzwang" on a new
     * line
     */
    Message(NewLine);

  result = solve(next,n);

  /* We don't signal "Zugzwang" after the last attacking move of a
   * self play variation */
  if (n>slack_length && result==n+2)
  {
    StdChar(blank);
    Message(Zugzwang);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
