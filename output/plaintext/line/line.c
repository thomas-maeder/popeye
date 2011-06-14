#include "output/plaintext/line/line.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/trivial_end_filter.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

static void instrument_suppressor(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();


  {
    Goal const goal = { no_goal, initsquare };
    pipe_append(slices[si].prev,alloc_line_writer_slice(goal));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

static void min_distance_to_goal_battle(slice_index si,
                                        stip_structure_traversal *st)
{
  stip_length_type * const min_distance_to_goal = st->param;
  stip_length_type const min_dist = slices[si].u.branch.length-slack_length_battle;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_dist<*min_distance_to_goal)
    *min_distance_to_goal = min_dist;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void min_distance_to_goal_help(slice_index si,
                                        stip_structure_traversal *st)
{
  stip_length_type * const min_distance_to_goal = st->param;
  stip_length_type const min_dist = slices[si].u.branch.length-slack_length_help;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_dist<*min_distance_to_goal)
    *min_distance_to_goal = min_dist;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void min_distance_to_goal_series(slice_index si,
                                        stip_structure_traversal *st)
{
  stip_length_type * const min_distance_to_goal = st->param;
  stip_length_type const min_dist = slices[si].u.branch.length-slack_length_series;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_dist<*min_distance_to_goal)
    *min_distance_to_goal = min_dist;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void min_distance_to_goal_goal(slice_index si,
                                        stip_structure_traversal *st)
{
  stip_length_type * const min_distance_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *min_distance_to_goal = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

structure_traversers_visitors const min_distance_to_goal_finders[] =
{
  { STAttackAdapter,     &min_distance_to_goal_battle },
  { STDefenseAdapter,    &min_distance_to_goal_battle },
  { STHelpAdapter,       &min_distance_to_goal_help   },
  { STSeriesAdapter,     &min_distance_to_goal_series },
  { STGoalReachedTester, &min_distance_to_goal_goal   }
};

enum
{
  nr_min_distance_to_goal_finders = sizeof min_distance_to_goal_finders
                                    / sizeof min_distance_to_goal_finders[0]
};

/* Determine minimum distance (in half moves) to goal
 * @param end_of_branch identifies end of branch slice
 * @return minimum distance
 */
static stip_length_type min_distance_to_goal(slice_index end_of_branch)
{
  stip_length_type result = UINT_MAX;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",end_of_branch);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override(&st,
                                    min_distance_to_goal_finders,
                                    nr_min_distance_to_goal_finders);
  stip_traverse_structure(slices[end_of_branch].u.fork.fork,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_end_of_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_distance_to_goal(si)>1)
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

static structure_traversers_visitors regular_inserters[] =
{
  { STEndOfBranch,       &instrument_end_of_branch       },
  { STConstraint,        &stip_traverse_structure_pipe   },
  { STPlaySuppressor,    &instrument_suppressor          },
  { STGoalReachedTester, &instrument_goal_reached_tester },
  { STAttackAdapter,     &instrument_root                },
  { STHelpAdapter,       &instrument_root                },
  { STSeriesAdapter,     &instrument_root                }
};

enum
{
  nr_regular_inserters = sizeof regular_inserters / sizeof regular_inserters[0]
};

static void insert_regular_slices(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&output_plaintext_slice_determining_starter);
  stip_structure_traversal_override(&st,regular_inserters,nr_regular_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_move(slice_index si, stip_structure_traversal *st)
{
  slice_index * const move_slice = st->param;
  slice_index const save_move_slice = *move_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *move_slice = si;
  stip_traverse_structure_children(si,st);
  *move_slice = save_move_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_constraint(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const move_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*move_slice==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const adapter = branch_find_slice(STAttackAdapter,
                                                  slices[si].u.fork.fork);
    if (adapter!=no_slice)
      pipe_append(adapter,alloc_trivial_end_filter_slice());
    stip_traverse_structure_pipe(si,st);
  }


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors root_inserters[] =
{
  { STGoalReachedTester, &instrument_goal_reached_tester },
  { STConstraint,        &instrument_constraint          },
  { STMove,              &remember_move                  }
};

enum
{
  nr_root_inserters = sizeof root_inserters / sizeof root_inserters[0]
};

static void instrument_constraints(slice_index si)
{
  stip_structure_traversal st;
  slice_index move_slice = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&move_slice);
  stip_structure_traversal_override(&st,root_inserters,nr_root_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with slices that implement
 * plaintext line mode output.
 * @param si identifies slice where to start
 */
void stip_insert_output_plaintext_line_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  insert_regular_slices(si);
  instrument_constraints(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
