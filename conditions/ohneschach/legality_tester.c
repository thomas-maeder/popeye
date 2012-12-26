#include "conditions/ohneschach/legality_tester.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "debugging/trace.h"

#include <assert.h>

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
boolean immobile(Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ohneschach_check_guard_solve() may invoke itself recursively. Protect
   * ourselves from infinite recursion. */
  if (nbply>250)
    FtlMsg(ChecklessUndecidable);

  result = solve(slices[temporary_hack_immobility_tester[side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
    boolean instrumenting_goal_immobile;
    boolean move_to_leaf;
    stip_length_type length;
} instrumentation_state_type;

static void instrument_simple(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;
  boolean const save_instrumenting_goal_immobile = state->instrumenting_goal_immobile;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->instrumenting_goal_immobile = false;
  state->move_to_leaf = false;

  stip_traverse_structure_children(si,st);

  state->instrumenting_goal_immobile = save_instrumenting_goal_immobile;

  TraceValue("%u\n",state->instrumenting_goal_immobile);
  switch (st->context)
  {
    case stip_traversal_context_attack:
    case stip_traversal_context_defense:
      {
        slice_index const prototype = alloc_pipe(state->instrumenting_goal_immobile
                                                 ? STOhneschachCheckGuard
                                                 : STOhneschachCheckGuardDefense);
        branch_insert_slices_contextual(si,st->context,&prototype,1);
      }
      break;

    case stip_traversal_context_help:
      if (state->move_to_leaf                     /* branch ends with this move */
          && (state->length-slack_length)%2==1    /* move might reach goal */
          && !state->instrumenting_goal_immobile) /* goal does not immobilise */
      {
        /* check is not illegal if it is mate - but we have to make sure to notice */
        slice_index const prototype = alloc_pipe(STOhneschachCheckGuardDefense);
        branch_insert_slices_contextual(si,st->context,&prototype,1);
      }
      else
      {
        slice_index const prototype = alloc_pipe(STOhneschachCheckGuard);
        branch_insert_slices_contextual(si,st->context,&prototype,1);
      }
      break;

    default:
      assert(0);
      break;
  }

  state->move_to_leaf = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_goal_immobile(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;
  boolean const save_instrumenting_goal_immobile = state->instrumenting_goal_immobile;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const tester = branch_find_slice(STGoalReachedTester,
                                                 slices[si].next2,
                                                 st->context);
    goal_type const goal = slices[tester].u.goal_handler.goal.type;
    assert(tester!=no_slice);
    state->instrumenting_goal_immobile = (goal==goal_mate
                                          || goal==goal_stale
                                          || goal==goal_mate_or_stale);
    stip_traverse_structure_binary_operand1(si,st);
    state->instrumenting_goal_immobile = save_instrumenting_goal_immobile;
  }

  stip_traverse_structure_binary_operand2(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_length(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;
  stip_length_type const save_length = state->length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->length = slices[si].u.branch.length;
  stip_traverse_structure_children(si,st);
  state->length = save_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_move_to_leaf(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  state->move_to_leaf = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_ohneschach_legality_testers(slice_index si)
{
  stip_structure_traversal st;
  instrumentation_state_type state = { false, false, slack_length };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,STReadyForHelpMove,&remember_length);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoalImmobile,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoal,&remember_goal_immobile);
  stip_structure_traversal_override_single(&st,STNotEndOfBranchGoal,&instrument_simple);
  stip_structure_traversal_override_single(&st,STTrue,&remember_move_to_leaf);
  stip_structure_traversal_override_single(&st,STOhneschachSuspender,&stip_structure_visitor_noop);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
