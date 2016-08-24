#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

typedef struct
{
  slice_type interval_start;
  slice_type hook_type;
  slice_index joint;
  slice_index prototype;
  slice_type avoided_type;
  slice_type joint_type;
} insertion_state_type;

static void insert_fork(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = copy_slice(state->prototype);
  slice_index const avoided = alloc_pipe(state->avoided_type);

  SLICE_NEXT2(prototype) = proxy;
  pipe_link(proxy,avoided);

  assert(state->joint!=no_slice);
  link_to_branch(avoided,state->joint);

  circe_insert_slices(si,st->context,&prototype,1);
}

static void instrument(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  slice_index const save_landing = state->joint;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->joint = no_slice;

  stip_traverse_structure_children(si,st);

  insert_fork(si,st);
  state->joint = save_landing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_joint(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->joint==no_slice);
  state->joint = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_insertion(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    insertion_state_type * const state = st->param;
    stip_structure_traversal st_nested;

    assert(SLICE_TYPE(si)==state->interval_start);

    state->joint = no_slice;

    stip_structure_traversal_init_nested(&st_nested,st,state);
    /* do this first - joint_type may be ==STLandingAfterCirceRebirth and should
     * take precedence if it is: */
    stip_structure_traversal_override_single(&st_nested,STCirceDoneWithRebirth,&stip_structure_visitor_noop);
    stip_structure_traversal_override_single(&st_nested,state->hook_type,&instrument);
    stip_structure_traversal_override_single(&st_nested,state->joint_type,&remember_joint);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with a type of "Circe rebirth avoiders" (i.e.
 * slices that may detour around Circe rebirth under a certain condition;
 * STCaptureFork is an example).
 * @param si identifies root slice of the solving machinery
 * @param interval_start start of the slices interval where to instrument
 * @param hook_type insertion is tried at each slice of this type
 * @param prototype copies of this are inserted
 * @param avoided_type type of proxy inserted on the "rebirth avoided" path
 * @param joint_type type of proxy where the two paths meet again
 * @note circe_insert_rebirth_avoider() assumes ownership of prototype
 */
void circe_insert_rebirth_avoider(slice_index si,
                                  slice_type interval_start,
                                  slice_type hook_type,
                                  slice_type prototype,
                                  slice_type avoided_type,
                                  slice_type joint_type)
{
  stip_structure_traversal st;
  insertion_state_type state = { interval_start, hook_type, no_slice, prototype, avoided_type, joint_type };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceEnumerator(slice_type,hook_type);
  TraceEnumerator(slice_type,SLICE_TYPE(prototype));
  TraceEnumerator(slice_type,avoided_type);
  TraceEnumerator(slice_type,joint_type);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,interval_start,&start_insertion);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceFutileCapturesRemover,
                                           &stip_structure_visitor_noop);
  stip_traverse_structure(si,&st);

  dealloc_slice(prototype);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
