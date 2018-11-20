#include "output/latex/goal_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "output/plaintext/tree/tree.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* This module provides the STOutputLaTeXGoalWriter slice type.
 * Slices of this type write the goal at the end of a variation
 */


static char const *latex_goal_end_marker[nr_goals] =
{
  " \\#"
  , " ="
  , " =="
  , " z"
  , " +"
  , " x"
  , " \\%"
  , ""
  , " \\#\\#"
  , " \\#\\#!"
  , ""
  , " !="
  , ""
  , ""
  , ""
  , ""
  , ""
  , " dia"
  , " a=>b"
  , " \\#="
  , " c81"
  , ""
  , " !\\#"
  , ""
};

/* Allocate a STOutputLaTeXGoalWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_output_latex_goal_writer_slice(Goal goal, FILE *file)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputLaTeXGoalWriter);
  SLICE_U(result).goal_writer.goal = goal;
  SLICE_U(result).goal_writer.file = file;

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
void output_latex_goal_writer_solve(slice_index si)
{
  Goal const goal = SLICE_U(si).goal_writer.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  fprintf(SLICE_U(si).goal_writer.file,"%s",latex_goal_end_marker[goal.type]);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
