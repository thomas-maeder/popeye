#include "output/plaintext/tree/zugzwang_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/ply.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/message.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Allocate a STOutputPlainTextZugzwangWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_tree_zugzwang_writer_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlainTextZugzwangWriter);

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
void output_plaintext_tree_zugzwang_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (parent_ply[nbply]==ply_retro_move)
  {
    /* option postkey is set - write "threat:" or "zugzwang" on a new line
     */
    Message2(stdout,NewLine);
    if (TraceFile)
      Message2(TraceFile,NewLine);
  }

  pipe_solve_delegate(si);

  /* We don't signal "Zugzwang" after the last attacking move of a
   * self play variation */
  if (solve_nr_remaining>=next_move_has_solution
      && solve_result==MOVE_HAS_NOT_SOLVED_LENGTH())
  {
    fputc(' ',stdout);
    Message2(stdout,Zugzwang);
    if (TraceFile)
    {
      fputc(' ',TraceFile);
      Message2(TraceFile,Zugzwang);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
