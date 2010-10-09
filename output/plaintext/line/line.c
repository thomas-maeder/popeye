#include "output/plaintext/line/line.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "trace.h"

#include <assert.h>

typedef struct
{
    slice_index root_slice;
    Goal goal;
} line_slices_insertion_state;

static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  line_slices_insertion_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->goal.type!=no_goal)
  {
    assert(state->root_slice!=no_slice);
    pipe_append(slices[si].prev,
                alloc_line_writer_slice(state->root_slice,state->goal));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_reached_tester(slice_index si,
                                           stip_structure_traversal *st)
{
  line_slices_insertion_state * const state = st->param;
  Goal const save_goal = state->goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal = slices[si].u.goal_reached_tester.goal;
  stip_traverse_structure_children(si,st);
  state->goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void instrument_goal_non_target_reached_tester(slice_index si,
                                               stip_structure_traversal *st)
{
  line_slices_insertion_state * const state = st->param;
  Goal const save_goal = state->goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal.type = goal_mate+(slices[si].type-STGoalMateReachedTester);
  stip_traverse_structure_children(si,st);
  state->goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_target_reached_tester(slice_index si,
                                                  stip_structure_traversal *st)
{
  line_slices_insertion_state * const state = st->param;
  Goal const save_goal = state->goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal.type = goal_target;
  state->goal.target = slices[si].u.goal_reached_tester.goal.target;
  stip_traverse_structure_children(si,st);
  state->goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move_inverter(slice_index si,
                                     stip_structure_traversal *st)
{
  line_slices_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->root_slice==no_slice)
    state->root_slice = si;

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_output_plaintext_line_move_inversion_counter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root(slice_index si, stip_structure_traversal *st)
{
  line_slices_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->root_slice==no_slice)
    state->root_slice = si;

  stip_traverse_structure_children(si,st);
  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_series_fork(slice_index si,
                                   stip_structure_traversal *st)
{
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* if we start another "real" series branch, si is part of an intro
   * series; restart move counting after forking */
  if (branch_find_slice(STSeriesFork,to_goal)!=no_slice)
  {
    slice_index const marker
        = alloc_output_plaintext_line_end_of_intro_series_marker_slice();
    pipe_link(marker,to_goal);
    slices[si].u.branch_fork.towards_goal = marker;
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors line_slice_inserters[] =
{
  { STSeriesFork,                       &instrument_series_fork                    },
  { STGoalReachedTester,                &instrument_goal_reached_tester            },
  { STGoalMateReachedTester,            &instrument_goal_non_target_reached_tester },
  { STGoalStalemateReachedTester,       &instrument_goal_non_target_reached_tester },
  { STGoalDoubleStalemateReachedTester, &instrument_goal_non_target_reached_tester },
  { STGoalTargetReachedTester,          &instrument_goal_target_reached_tester     },
  { STGoalCheckReachedTester,           &instrument_goal_non_target_reached_tester },
  { STGoalCaptureReachedTester,         &instrument_goal_non_target_reached_tester },
  { STGoalSteingewinnReachedTester,     &instrument_goal_non_target_reached_tester },
  { STGoalEnpassantReachedTester,       &instrument_goal_non_target_reached_tester },
  { STGoalDoubleMateReachedTester,      &instrument_goal_non_target_reached_tester },
  { STGoalCounterMateReachedTester,     &instrument_goal_non_target_reached_tester },
  { STGoalCastlingReachedTester,        &instrument_goal_non_target_reached_tester },
  { STGoalAutoStalemateReachedTester,   &instrument_goal_non_target_reached_tester },
  { STLeaf,                             &instrument_leaf                           },
  { STMoveInverterRootSolvableFilter,   &instrument_move_inverter                  },
  { STMoveInverterSolvableFilter,       &instrument_move_inverter                  },
  { STHelpRoot,                         &instrument_root                           },
  { STSeriesRoot,                       &instrument_root                           }
};

enum
{
  nr_line_slice_inserters = (sizeof line_slice_inserters
                             / sizeof line_slice_inserters[0])
};

/* Instrument the stipulation structure with slices that implement
 * plaintext line mode output.
 * @param si identifies slice where to start
 */
void stip_insert_output_plaintext_line_slices(slice_index si)
{
  stip_structure_traversal st;
  line_slices_insertion_state state = { no_slice, { no_goal, initsquare } };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    line_slice_inserters,
                                    nr_line_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
