#include "output/plaintext/tree/exclusive.h"
#include "conditions/exclusive.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/exclusive.h"
#include "pymsg.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STExclusiveChessUndecidableWriterTree slice
 * @return identifier of the allocated slice
 */
slice_index alloc_exclusive_chess_undecidable_writer_tree_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STExclusiveChessUndecidableWriterTree);

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
stip_length_type exclusive_chess_undecidable_writer_tree_solve(slice_index si,
                                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_current_move_in_table(undefined_moves_after_current_move[parent_ply[nbply]]))
  {
    result = move_writer_solve(si,n);
    assert(result==n+2);
    StdChar(blank);
    Message(ChecklessUndecidable);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
