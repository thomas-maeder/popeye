#include "output/output.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "output/plaintext/plaintext.h"
#include "output/latex/latex.h"
#include "solving/machinery/twin.h"
#include "debugging/trace.h"

/* Determine whether the symbol for a reached goal preempts the symbol for a given
 * check (if not, both a possible check and the symbol for the reached goal
 * should be written).
 * @param goal goal written by goal writer
 * @return true iff the check writer should be replaced by the goal writer
 */
boolean output_goal_preempts_check(goal_type goal)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_mate:
    case goal_check:
    case goal_doublemate:
    case goal_countermate:
    case goal_mate_or_stale:
    case goal_stale:
    case goal_dblstale:
    case goal_autostale:
      result = true;
      break;

    default:
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate an STOutputModeSelector slice
 * @param mode output mode to be selected by the allocated slice
 * @return identifier of the allocated slice
 */
slice_index alloc_output_mode_selector(output_mode mode)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(output_mode,mode,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputModeSelector);
  SLICE_U(result).output_mode_selector.mode = mode;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void solving_insert_output_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  output_plaintext_instrument_solving(si);
  output_latex_instrument_solving(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
