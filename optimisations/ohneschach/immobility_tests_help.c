#include "optimisations/ohneschach/immobility_tests_help.h"
#include "stipulation/stipulation.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/pipe.h"
#include "optimisations/ohneschach/stop_if_check.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

typedef struct
{
    boolean move_to_leaf;
    stip_length_type length;
} optimisation_state_type;

static void remember_length(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;
  stip_length_type const save_length = state->length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->length = SLICE_U(si).branch.length;
  stip_traverse_structure_children(si,st);
  state->length = save_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_move_to_leaf(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  state->move_to_leaf = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_move_to_leaf(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  /* current value of move_to_leaf has been consumed or is irrelevant */
  state->move_to_leaf = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void plan_optimisation(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->move_to_leaf = false;

  stip_traverse_structure_children_pipe(si,st);

  TraceValue("%u",state->move_to_leaf);
  TraceValue("%u\n",state->length-slack_length);

  {
    boolean const to_be_optimised = (st->context==stip_traversal_context_help
                                     && (!state->move_to_leaf
                                         || (state->length-slack_length)%2==0));
    ohneschach_stop_if_check_plan_to_optimise_away_stop(si,to_be_optimised);
  }

  state->move_to_leaf = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_slice_fates(slice_index si)
{
  stip_structure_traversal st;
  optimisation_state_type state = { false, slack_length };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,STReadyForHelpMove,&remember_length);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheck,&forget_move_to_leaf);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheckAndNotMate,&plan_optimisation);
  stip_structure_traversal_override_single(&st,STTrue,&remember_move_to_leaf);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise away redundant immobility tests
 * @param si identifies root slice of stipulation
 */
void ohneschach_optimise_away_immobility_tests_help(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  determine_slice_fates(si);
  ohneschach_stop_if_check_execute_optimisations(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
