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

typedef struct
{
    twinning_event_type event;
    boolean continued;
} notification_struct;

static void notify_medium(slice_index si, stip_structure_traversal *st)
{
  notification_struct * const notification = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*SLICE_U(si).twinning_event_handler.handler)(si,
                                                notification->event,
                                                notification->continued);

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Notify the output machinery about a twinning event
 * @param si identifies the slice that detected the twinning (at the same time
 *           to be used as the starting point of any instrumentation)
 * @param stage the twinning event
 * @param continued is the twin continued?
 */
void output_notify_twinning(slice_index si,
                            twinning_event_type event,
                            boolean continued)
{
  stip_structure_traversal st;
  notification_struct e = { event, continued };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",event);
  TraceFunctionParam("%u",continued);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&e);
  stip_structure_traversal_override_single(&st,STOutputPlaintextTwinIntroWriterBuilder,&notify_medium);
  stip_structure_traversal_override_single(&st,STOutputLaTeXTwinningWriterBuilder,&notify_medium);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
