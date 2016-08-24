#include "stipulation/modifier.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/machinery/twin.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Instrument the machinery with a stipulation modifier slice
 * @param si where to start the instrumentation
 * @param type slice type of the stipulation modifier slice
 */
void stipulation_modifier_instrument(slice_index si, slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(type);
    SLICE_NEXT2(prototype) = no_slice;
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void notify_modifier(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const stipulation_root_hook = st->param;

  SLICE_NEXT2(si) = *stipulation_root_hook;

  stip_traverse_structure_children(si,st);
}

/* Notifiy all stipulation modifier slices that the stipulation has changed
 * (i.e. that they have to do their modification work)
 * @param start identifies the slice where to start notifying
 * @param stipulation_root_hook entry slice into the new stipulation
 */
void stipulation_modifiers_notify(slice_index start,
                                  slice_index stipulation_root_hook)
{
  stip_structure_traversal st;
  stip_structure_traversal_init(&st,&stipulation_root_hook);
  stip_structure_traversal_override_single(&st,
                                           STQuodlibetStipulationModifier,
                                           &notify_modifier);
  stip_structure_traversal_override_single(&st,
                                           STGoalIsEndStipulationModifier,
                                           &notify_modifier);
  stip_structure_traversal_override_single(&st,
                                           STWhiteToPlayStipulationModifier,
                                           &notify_modifier);
  stip_structure_traversal_override_single(&st,
                                           STPostKeyPlayStipulationModifier,
                                           &notify_modifier);
  stip_structure_traversal_override_single(&st,
                                           STStipulationStarterDetector,
                                           &notify_modifier);
  stip_traverse_structure(start,&st);
}

/* Retrieve (from a stipulation modifier slice) the entry slice into the
 * stipulation to be modified
 * @param si identifies the stipulation modifier slice
 * @return the entry slice into the stipulation to be modified
 * @note resets the information about the stipulation to be modified
 */
slice_index stipulation_modifier_to_be_modified(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = SLICE_NEXT2(si);
  SLICE_NEXT2(si) = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
