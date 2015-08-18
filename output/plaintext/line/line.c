#include "output/plaintext/line/line.h"
#include "conditions/conditions.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "output/output.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/structure_traversal.h"
#include "solving/trivial_end_filter.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "output/plaintext/goal_writer.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/line/exclusive.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/line/refuting_variation_writer.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <limits.h>

static void instrument_suppressor(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();


  {
    Goal const goal = { no_goal, initsquare };
    pipe_append(SLICE_PREV(si),alloc_output_plaintext_line_writer_slice(goal));
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
    Goal const goal = SLICE_U(si).goal_handler.goal;
    slice_index const prototype = alloc_output_plaintext_line_writer_slice(goal);
    help_branch_insert_slices(si,&prototype,1);
  }

  {
    slice_index const prototype = alloc_output_plaintext_goal_writer_slice(SLICE_U(si).goal_handler.goal);
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

/* Insert the writer slices
 * @param si identifies slice where to start
 */
static void insert_regular_writer_slices(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
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

static void remember_postkey_play(slice_index si, stip_structure_traversal *st)
{
  boolean * const is_postkey_play = st->param;
  boolean const save_is_postkey_play = *is_postkey_play;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *is_postkey_play = st->level==structure_traversal_level_top;
  stip_traverse_structure_children_pipe(si,st);
  *is_postkey_play = save_is_postkey_play;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refuting_variation_writer(slice_index si,
                                             stip_structure_traversal *st)
{
  boolean const * const is_postkey_play = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*is_postkey_play)
  {
    slice_index const prototype = alloc_output_plaintext_line_refuting_variation_writer_slice();
    attack_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const root_writer_inserters[] =
{
  { STHelpAdapter,        &stip_structure_visitor_noop      },
  { STAttackAdapter,      &stip_structure_visitor_noop      },
  { STDefenseAdapter,     &remember_postkey_play            },
  { STDummyMove,          &stip_structure_visitor_noop      },
  { STIfThenElse,         &stip_traverse_structure_children_binary },
  { STNotEndOfBranchGoal, &insert_refuting_variation_writer }
};

enum
{
  nr_root_writer_inserters = (sizeof root_writer_inserters
                              / sizeof root_writer_inserters[0])
};

/* Insert the writer slices
 * @param si identifies slice where to start
 */
static void insert_root_writer_slices(slice_index si)
{
  stip_structure_traversal st;
  boolean is_postkey_play = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&is_postkey_play);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_binary,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_end_of_branch,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    root_writer_inserters,
                                    nr_root_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with slices that implement
 * plaintext line mode output.
 * @param si identifies slice where to start
 */
void solving_insert_output_plaintext_line_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  insert_regular_writer_slices(si);
  insert_root_writer_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
