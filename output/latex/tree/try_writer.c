#include "output/latex/tree/try_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/battle_play/try.h"
#include "solving/trivial_end_filter.h"
#include "output/latex/latex.h"
#include "output/latex/tree/key_writer.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STOutputLaTeXTryWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_output_latex_tree_try_writer(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXTryWriter);

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
void output_latex_tree_try_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (table_length(refutations)>0)
  {
    fputs(" ?",LaTeXFile);
    pipe_solve_delegate(si);
  }
  else
    output_latex_tree_key_writer_solve(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
