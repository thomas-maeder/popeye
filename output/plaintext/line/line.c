#include "output/plaintext/line/line.h"
#include "pypipe.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "trace.h"

#include <assert.h>

static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  Goal const * const goal = st->param;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (goal->type!=no_goal)
    pipe_append(slices[si].prev,alloc_line_writer_slice(*goal));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_reached_tester(slice_index si,
                                           stip_structure_traversal *st)
{
  Goal * const goal = st->param;
  Goal const save_goal = *goal;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goal = slices[si].u.goal_reached_tester.goal;
  stip_traverse_structure_children(si,st);
  *goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move_inverter(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_output_plaintext_line_move_inversion_counter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_series_fork(slice_index si,
                                   stip_structure_traversal *st)
{
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index next = slices[to_goal].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[to_goal].type==STProxy);
  while (slices[next].type==STProxy
         || slices[next].type==STSeriesHashed)
    next = slices[next].u.pipe.next;

  if (slices[next].type==STSeriesMove
      || slices[next].type==STSeriesFork)
  {
    slice_index const marker
        = alloc_output_plaintext_line_end_of_intro_series_marker_slice();
    pipe_set_successor(marker,to_goal);
    slices[si].u.branch_fork.towards_goal = marker;
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors line_slice_inserters[] =
{
  { STSeriesFork,                     &instrument_series_fork         },
  { STGoalReachedTester,              &instrument_goal_reached_tester },
  { STLeaf,                           &instrument_leaf                },
  { STMoveInverterRootSolvableFilter, &instrument_move_inverter       },
  { STMoveInverterSolvableFilter,     &instrument_move_inverter       },
  { STHelpRoot,                       &instrument_root                },
  { STSeriesRoot,                     &instrument_root                }
};

enum
{
  nr_line_slice_inserters = (sizeof line_slice_inserters
                             / sizeof line_slice_inserters[0])
};

/* Instrument the stipulation structure with slices that implement
 * plaintext line mode output.
 */
void stip_insert_output_plaintext_line_slices(void)
{
  stip_structure_traversal st;
  Goal state = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,
                                line_slice_inserters,nr_line_slice_inserters,
                                &state);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
