#include "output/plaintext/line/line.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "trace.h"

#include <assert.h>

static void instrument_goal_reached_tester(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    Goal const goal = slices[si].u.goal_tester.goal;
    slice_index const prototype = alloc_line_writer_slice(goal);
    leaf_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*root_slice==no_slice)
    *root_slice = si;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_end_of_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* if we start another series branch, si is part of an intro series;
   * restart move counting after forking */
  if (branch_find_slice(STSeriesAdapter,fork)!=no_slice)
  {
    slice_index const marker
        = alloc_output_plaintext_line_end_of_intro_series_marker_slice();
    pipe_link(marker,fork);
    slices[si].u.fork.fork = marker;
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors line_slice_inserters[] =
{
  { STEndOfBranch,       &instrument_end_of_branch       },
  { STGoalReachedTester, &instrument_goal_reached_tester },
  { STHelpAdapter,       &instrument_root                },
  { STSeriesAdapter,     &instrument_root                }
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

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&output_plaintext_slice_determining_starter);
  stip_structure_traversal_override(&st,
                                    line_slice_inserters,
                                    nr_line_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
