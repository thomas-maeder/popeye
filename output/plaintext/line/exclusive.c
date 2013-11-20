#include "output/plaintext/line/exclusive.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "conditions/exclusive.h"
#include "output/plaintext/line/line_writer.h"
#include "debugging/trace.h"
#include "pymsg.h"

#include <assert.h>

/* Allocate a STExclusiveChessUndecidableWriterLine slice
 * @return identifier of the allocated slice
 */
slice_index alloc_exclusive_chess_undecidable_writer_line_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STExclusiveChessUndecidableWriterLine);

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
stip_length_type exclusive_chess_undecidable_writer_line_solve(slice_index si,
                                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_current_move_in_table(exclusive_chess_undecidable_continuations[parent_ply[nbply]]))
  {
    output_plaintext_line_write_line(goal_mate);
    StdChar(' ');
    Message(ExclusiveRefutedUndecidable);
    result = previous_move_is_illegal;
  }
  else
  {
    result = solve(slices[si].next1,n);
    if (result==previous_move_has_solved
        && exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]]<2
        && table_length(exclusive_chess_undecidable_continuations[parent_ply[nbply]])+exclusive_chess_nr_continuations_reaching_goal[parent_ply[nbply]]>1)
    {
      StdChar(' ');
      Message(ChecklessUndecidable);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
