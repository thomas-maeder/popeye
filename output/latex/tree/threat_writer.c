#include "output/latex/tree/threat_writer.h"
#include "output/latex/latex.h"
#include "output/latex/message.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/battle_play/threat.h"
#include "solving/pipe.h"
#include "utilities/table.h"
#include "debugging/trace.h"

/* Allocate a STOutputLaTeXThreatWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_output_latex_tree_threat_writer_slice(FILE *file)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXThreatWriter);
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
void output_latex_tree_threat_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (table_length(threats[parent_ply[parent_ply[nbply]]])==0)
  {
    ply const save_nbply = nbply;
    output_latex_message(SLICE_U(si).writer.file,Threat);
    nbply = parent_ply[nbply];
    output_plaintext_write_dummy_move_effects(&output_latex_engine,
                                              SLICE_U(si).writer.file,
                                              &output_latex_symbol_table);
    nbply = save_nbply;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
