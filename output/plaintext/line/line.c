#include "output/plaintext/line/line.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "trace.h"

#include <assert.h>

static void instrument_goal_reached_testing(slice_index si,
                                            stip_structure_traversal *st)
{
  slice_index const * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_line_writer_slice(*root_slice,
                                                          slices[si].u.goal_writer.goal);
    leaf_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move_inverter(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*root_slice==no_slice)
    *root_slice = si;

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_output_plaintext_line_move_inversion_counter_slice());

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
  { STSeriesFork,         &instrument_series_fork          },
  { STGoalReachedTesting, &instrument_goal_reached_testing },
  { STMoveInverter,       &instrument_move_inverter        },
  { STHelpRoot,           &instrument_root                 },
  { STSeriesAdapter,      &instrument_root                 }
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
  slice_index root_slice = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&root_slice);
  stip_structure_traversal_override(&st,
                                    line_slice_inserters,
                                    nr_line_slice_inserters);
  stip_traverse_structure(si,&st);

  {
    slice_index const prototypes[] =
    {
        alloc_illegal_selfcheck_writer_slice(),
        alloc_end_of_phase_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    root_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
