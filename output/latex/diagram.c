#include "output/latex/diagram.h"
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

/* Allocate a STOutputLaTeXDiagramWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_latex_diagram_writer(FILE *file)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXDiagramWriter);
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
void output_latex_write_diagram(slice_index si)
{
  FILE *file = SLICE_U(si).writer.file;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  LaTeXBeginDiagram(file);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STOutputLaTeXPositionWriterBuilder),
        alloc_pipe(STOutputLaTeXTwinningWriterBuilder)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  pipe_solve_delegate(si);

  LaTexCloseSolution(file);
  LaTeXFlushTwinning(file);
  LaTeXCo(file);
  LaTeXEndDiagram(file);
  LaTeXHfill(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STOutputLaTeXDiagramStartWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_latex_diagram_start_writer(FILE *file)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXDiagramStartWriter);
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
void output_latex_write_diagram_start(slice_index si)
{
  FILE *file = SLICE_U(si).writer.file;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (twin_stage)
  {
    case twin_original_position_no_twins:
    case twin_initial:
    case twin_zeroposition:
      if (twin_duplex_type!=twin_is_duplex)
      {
        LaTeXMeta(file);
        LaTeXOptions();
        LaTeXWritePieces(file);
        LaTeXStipulation(file);
        LaTeXConditions(file);
        LaTexOpenSolution(file);
      }
      pipe_solve_delegate(si);
      break;

    case twin_regular:
    case twin_last:
      pipe_solve_delegate(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
