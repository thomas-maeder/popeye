#include "output/latex/problem.h"
#include "output/latex/latex.h"
#include "output/latex/twinning.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "solving/duplex.h"
#include "solving/machinery/twin.h"
#include "stipulation/slice_insertion.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

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
void output_latex_problem_intro_writer_solve(slice_index si)
{
  slice_index const file_owner = SLICE_NEXT2(si);
  FILE * const file = SLICE_U(file_owner).writer.file;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  LaTeXMeta(file);
  LaTeXOptions();
  LaTeXWritePieces(file);
  LaTeXStipulation(file);
  LaTeXConditions(file);
  LaTexOpenSolution(file);

  pipe_solve_delegate(si);

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void output_latex_problem_writer_solve(slice_index si)
{
  slice_index const file_owner = SLICE_NEXT2(si);
  FILE * const file = SLICE_U(file_owner).writer.file;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (file==0)
    pipe_solve_delegate(si);
  else
  {
    slice_index const prototypes[] =
    {
        output_latex_alloc_twin_intro_writer_builder(file_owner),
        alloc_output_latex_writer(STOutputLaTeXInstrumentSolversBuilder,file_owner),
        alloc_output_latex_writer(STOutputLaTeXProblemIntroWriter,file_owner)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);

    LaTeXBeginDiagram(file);

    pipe_solve_delegate(si);

    LaTexCloseSolution(file);
    LaTeXFlushTwinning(file);
    LaTeXCo(si,file);
    LaTeXEndDiagram(file);
    LaTeXHfill(file);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
