#include "output/latex/tree/end_of_solution_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Allocate a STOutputLaTeXTreeEndOfSolutionWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_latex_tree_end_of_solution_writer_slice(FILE *file)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXTreeEndOfSolutionWriter);
  SLICE_U(result).writer.file = file;

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
void output_latex_tree_end_of_solution_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  TraceValue("%u",slack_length);
  TraceValue("%u",solve_result);
  TraceValue("%u\n",MOVE_HAS_SOLVED_LENGTH());
  if (move_has_solved())
  {
    TraceText("has solved\n");
    fputs("\n",SLICE_U(si).writer.file);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
