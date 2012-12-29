#include "optimisations/ohneschach/immobility_tests_help.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

typedef struct
{
    boolean move_to_leaf;
    stip_length_type length;
} optimisation_state_type;

static void optimise_stop(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->move_to_leaf = false;

  stip_traverse_structure_children(si,st);

  TraceValue("%u",state->move_to_leaf);
  TraceValue("%u\n",state->length-slack_length);

  if (st->context==stip_traversal_context_help
      && (!state->move_to_leaf
          || (state->length-slack_length)%2==0))
    pipe_substitute(si,alloc_pipe(STOhneschachStopIfCheck));

  state->move_to_leaf = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_length(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;
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
  optimisation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  state->move_to_leaf = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise away redundant immobility tests
 * @param si identifies root slice of stipulation
 */
void ohneschach_optimise_away_immobility_tests_help(slice_index si)
{
  stip_structure_traversal st;
  optimisation_state_type state = { false, slack_length };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,STReadyForHelpMove,&remember_length);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheckAndNotMate,&optimise_stop);
  stip_structure_traversal_override_single(&st,STTrue,&remember_move_to_leaf);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
