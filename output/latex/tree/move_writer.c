#include "output/latex/tree/move_writer.h"
#include "output/latex/latex.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/move_inversion_counter.h"
#include "solving/pipe.h"
#include "solving/ply.h"
#include "debugging/trace.h"

/* Allocate a STOutputLaTeXMoveWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_output_latex_tree_move_writer_slice(FILE *file)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXMoveWriter);
  SLICE_U(result).writer.file = file;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int measure_move_depth(ply curr_ply)
{
  ply const parent = parent_ply[curr_ply];

  if (parent==ply_retro_move)
    return output_plaintext_nr_move_inversions;
  else
    return measure_move_depth(parent)+1;
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
void output_latex_tree_move_writer_solve(slice_index si)
{
  unsigned int const move_depth = measure_move_depth(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  fprintf(SLICE_U(si).writer.file,"\n%*c%3u.",(int)(4*move_depth),' ',move_depth/2+1);
  if (move_depth%2==1)
    fputs("..",SLICE_U(si).writer.file);

  output_plaintext_write_move(&output_latex_engine,
                              SLICE_U(si).writer.file,
                              &output_latex_symbol_table);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
