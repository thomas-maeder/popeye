#include "output/latex/line/line.h"
#include "conditions/conditions.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "output/output.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/trivial_end_filter.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/slice_insertion.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/line/exclusive.h"
#include "output/latex/line/line_writer.h"
#include "output/latex/goal_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <limits.h>

static void instrument_suppressor(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();


  {
    FILE *file = st->param;
    Goal const goal = { no_goal, initsquare };
    slice_index const proto = alloc_output_latex_line_writer_slice(goal,file);
    slice_insertion_insert(SLICE_PREV(si),&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_reached_tester(slice_index si,
                                           stip_structure_traversal *st)
{
  FILE *file = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    Goal const goal = SLICE_U(si).goal_handler.goal;
    slice_index const prototype = alloc_output_latex_line_writer_slice(goal,file);
    help_branch_insert_slices(si,&prototype,1);
  }

  {
    slice_index const prototype = alloc_output_latex_goal_writer_slice(SLICE_U(si).goal_handler.goal,
                                                                       file);
    help_branch_insert_slices(si,&prototype,1);
  }

  if (CondFlag[ohneschach])
  {
    slice_index const prototype = alloc_ohneschach_detect_undecidable_goal_slice();
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void min_distance_to_goal_battle(slice_index si,
                                        stip_structure_traversal *st)
{
  stip_length_type * const min_distance_to_goal = st->param;
  stip_length_type const min_dist = SLICE_U(si).branch.length-slack_length;

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
  stip_length_type const min_dist = SLICE_U(si).branch.length-slack_length;

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

static structure_traversers_visitor const min_distance_to_goal_finders[] =
{
  { STAttackAdapter,     &min_distance_to_goal_battle },
  { STDefenseAdapter,    &min_distance_to_goal_battle },
  { STHelpAdapter,       &min_distance_to_goal_help   },
  { STGoalReachedTester, &min_distance_to_goal_goal   }
};

enum
{
  nr_min_distance_to_goal_finders = sizeof min_distance_to_goal_finders
                                    / sizeof min_distance_to_goal_finders[0]
};

/* Determine minimum distance (in half moves) to goal
 * @param end_of_branch identifies end of branch slice
 * @param st parent traversal
 * @return minimum distance
 */
static stip_length_type min_distance_to_goal(slice_index end_of_branch,
                                             stip_structure_traversal *st)
{
  stip_length_type result = UINT_MAX;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",end_of_branch);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init_nested(&st_nested,st,&result);
  stip_structure_traversal_override(&st_nested,
                                    min_distance_to_goal_finders,
                                    nr_min_distance_to_goal_finders);
  stip_traverse_structure_end_of_branch_next_branch(end_of_branch,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_end_of_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index const fork = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* the value used here is a bit arbitrary */
  if (min_distance_to_goal(si,st)>2)
  {
    slice_index const marker
        = alloc_output_plaintext_line_end_of_intro_series_marker_slice();
    pipe_link(marker,fork);
    SLICE_NEXT2(si) = marker;
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (CondFlag[exclusive])
  {
    slice_index const prototype = alloc_exclusive_chess_undecidable_writer_line_slice();
    move_insert_slices(si,st->context,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_inversion_counter(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->level!=structure_traversal_level_nested)
  {
    slice_index const prototype = alloc_pipe(STOutputPlaintextMoveInversionCounter);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_inversion_counter_setplay(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->level!=structure_traversal_level_nested)
  {
    slice_index const prototype = alloc_pipe(STOutputPlaintextMoveInversionCounterSetPlay);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor regular_inserters[] =
{
  { STMoveInverter,        &insert_move_inversion_counter         },
  { STMoveInverterSetPlay, &insert_move_inversion_counter_setplay },
  { STPlaySuppressor,      &instrument_suppressor                 },
  { STGoalReachedTester,   &instrument_goal_reached_tester        },
  { STMove,                &instrument_move                       }
};

enum
{
  nr_regular_inserters = sizeof regular_inserters / sizeof regular_inserters[0]
};

/* Instrument the stipulation structure with slices that implement
 * LaTeX line mode output.
 * @param si identifies slice where to start
 */
void solving_insert_output_latex_line_slices(slice_index si, FILE *file)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,file);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_end_of_branch,
                                                  &instrument_end_of_branch);
  stip_structure_traversal_override(&st,regular_inserters,nr_regular_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
