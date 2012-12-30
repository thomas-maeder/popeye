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

typedef enum
{
  fate_dont_know,
  fate_obsolete,
  fate_still_used,
  fate_deallocated
} fate_type;

static fate_type fate[max_nr_slices];

static void optimise_stop(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;
  slice_index const condition = slices[si].next2;
  boolean save_move_to_leaf;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->move_to_leaf = false;

  stip_traverse_structure_children_pipe(si,st);

  save_move_to_leaf = state->move_to_leaf;
  stip_traverse_structure_conditional_pipe_tester(si,st);
  state->move_to_leaf = save_move_to_leaf;

  TraceValue("%u",state->move_to_leaf);
  TraceValue("%u",state->length-slack_length);
  TraceValue("%u\n",fate[condition]);

  if (st->context==stip_traversal_context_help
      && (!state->move_to_leaf
          || (state->length-slack_length)%2==0))
  {
    if (fate[condition]==fate_obsolete)
    {
      fate[condition] = fate_deallocated;
      dealloc_slices(condition);
    }
    if (slices[si].tester!=no_slice)
      pipe_substitute(slices[si].tester,alloc_pipe(STOhneschachStopIfCheck));
    pipe_substitute(si,alloc_pipe(STOhneschachStopIfCheck));
  }

  state->move_to_leaf = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_slices(slice_index si)
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

static void visit_stop(slice_index si, stip_structure_traversal *st)
{
  optimisation_state_type * const state = st->param;
  slice_index const condition = slices[si].next2;
  fate_type save_fate;
  boolean save_move_to_leaf;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->move_to_leaf = false;

  stip_traverse_structure_children_pipe(si,st);

  save_move_to_leaf = state->move_to_leaf;
  save_fate = fate[condition];

  stip_traverse_structure_conditional_pipe_tester(si,st);

  fate[condition] = save_fate;
  state->move_to_leaf = save_move_to_leaf;

  TraceValue("%u",state->move_to_leaf);
  TraceValue("%u\n",state->length-slack_length);

  if (st->context==stip_traversal_context_help
      && (!state->move_to_leaf
          || (state->length-slack_length)%2==0))
  {
    if (fate[condition]==fate_dont_know)
      fate[condition] = fate_obsolete;
  }
  else
    fate[condition] = fate_still_used;

  TraceValue("%u\n",fate[condition]);

  state->move_to_leaf = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_slice_fates(slice_index si)
{
  stip_structure_traversal st;
  optimisation_state_type state = { false, slack_length };
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    fate[i] = fate_dont_know;

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,STReadyForHelpMove,&remember_length);
  stip_structure_traversal_override_single(&st,STOhneschachStopIfCheckAndNotMate,&visit_stop);
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

  determine_slice_fates(si);
  optimise_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
