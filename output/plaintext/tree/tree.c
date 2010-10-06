#include "output/plaintext/tree/tree.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/continuation_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/variation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/tree/move_inversion_counter.h"
#include "platform/beep.h"
#include "trace.h"

#include <assert.h>


/* Are we writing postkey play?
 */
typedef enum
{
  output_suppressed,
  output_postkeyplay_exclusively,
  output_included
} output_state_type;

/* Do we have to insert an STEndOfSolutionWriter slice?
 */
typedef enum
{
  end_of_solution_writer_not_inserted,
  end_of_solution_writer_inserted
} end_of_solution_writer_insertion_state_type;

typedef struct
{
    Goal reached_goal;
    output_state_type output_state;
    end_of_solution_writer_insertion_state_type end_state;
} instrumentation_state;

static void instrument_goal_reached_tester(slice_index si,
                                           stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  Goal const save_reached_goal = state->reached_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->reached_goal = slices[si].u.goal_reached_tester.goal;
  stip_traverse_structure_children(si,st);
  state->reached_goal = save_reached_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_mate_reached_tester(slice_index si,
                                                stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  Goal const save_reached_goal = state->reached_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->reached_goal.type = goal_mate;
  stip_traverse_structure_children(si,st);
  state->reached_goal = save_reached_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void instrument_goal_stalemate_reached_tester(slice_index si,
                                              stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  Goal const save_reached_goal = state->reached_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->reached_goal.type = goal_stale;
  stip_traverse_structure_children(si,st);
  state->reached_goal = save_reached_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_target_reached_tester(slice_index si,
                                                  stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  Goal const save_reached_goal = state->reached_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->reached_goal.type = goal_target;
  state->reached_goal.target = slices[si].u.goal_reached_tester.goal.target;
  stip_traverse_structure_children(si,st);
  state->reached_goal = save_reached_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->reached_goal.type!=no_goal)
    pipe_append(slices[si].prev,alloc_goal_writer_slice(state->reached_goal));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_ready_for_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  instrumentation_state const * const state = st->param;

  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->output_state!=output_suppressed
      && state->reached_goal.type==no_goal)
    pipe_append(slices[si].prev,
                alloc_output_plaintext_tree_check_writer_attacker_filter_slice(length,min_length));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_threat_solver(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const writer = alloc_zugzwang_writer_slice();
    pipe_set_successor(writer,slices[si].u.threat_solver.threat_start);
    slice_set_predecessor(writer,si);
    slices[si].u.threat_solver.threat_start = writer;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_ready_for_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  instrumentation_state const * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->output_state!=output_suppressed
      && length>slack_length_battle)
    pipe_append(si,
                alloc_output_plaintext_tree_check_writer_defender_filter_slice(length,min_length));

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_move_filtered(slice_index si,
                                             stip_structure_traversal *st)
{
  instrumentation_state const * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->output_state!=output_suppressed)
  {
    if (state->output_state==output_postkeyplay_exclusively)
      pipe_append(si,alloc_refuting_variation_writer_slice(length,min_length));
    pipe_append(si,alloc_variation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_continuation_solver(slice_index si,
                                           stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_continuation_writer_slice(length,min_length));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_try_solver(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_try_writer());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_attack_move_played(slice_index si,
                                          stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->end_state==end_of_solution_writer_not_inserted
      && state->output_state!=output_postkeyplay_exclusively)
  {
    state->end_state = end_of_solution_writer_inserted;
    stip_traverse_structure_children(si,st);
    state->end_state = end_of_solution_writer_not_inserted;

    pipe_append(si,alloc_end_of_solution_writer_slice());
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_root(slice_index si,
                                    stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  output_state_type const save_output_state = state->output_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  state->output_state = output_postkeyplay_exclusively;
  stip_traverse_structure_children(si,st);
  state->output_state = save_output_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_refutations_collector(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_refutation_writer_slice());

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
  pipe_append(si,alloc_output_plaintext_tree_move_inversion_counter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void activate_output(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  output_state_type const save_output_state = state->output_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->output_state = output_included;
  stip_traverse_structure_children(si,st);
  state->output_state = save_output_state;

  pipe_append(si,alloc_key_writer());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void suppress_output(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state * const state = st->param;
  output_state_type const save_output_state = state->output_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->output_state = output_suppressed;
  stip_traverse_structure_children(si,st);
  state->output_state = save_output_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_setplay_fork(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index end_of_phase = alloc_end_of_phase_writer_slice();
    pipe_link(end_of_phase,slices[si].u.branch_fork.towards_goal);
    slices[si].u.branch_fork.towards_goal = end_of_phase;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors tree_slice_inserters[] =
{
  { STSetplayFork,                    &instrument_setplay_fork                 },
  { STGoalReachedTester,              &instrument_goal_reached_tester          },
  { STGoalMateReachedTester,          &instrument_goal_mate_reached_tester     },
  { STGoalStalemateReachedTester,     &instrument_goal_stalemate_reached_tester},
  { STGoalTargetReachedTester,        &instrument_goal_target_reached_tester   },
  { STLeaf,                           &instrument_leaf                         },
  { STMoveInverterRootSolvableFilter, &instrument_move_inverter                },
  { STMoveInverterSolvableFilter,     &instrument_move_inverter                },
  { STAttackMovePlayed,               &instrument_attack_move_played           },
  { STDefenseRoot,                    &instrument_defense_root                 },
  { STContinuationSolver,             &instrument_continuation_solver          },
  { STTrySolver,                      &instrument_try_solver                   },
  { STThreatSolver,                   &instrument_threat_solver                },
  { STDefenseMoveFiltered,            &instrument_defense_move_filtered        },
  { STRefutationsCollector,           &instrument_refutations_collector        },
  { STSeriesRoot,                     &stip_structure_visitor_noop             },
  { STDefenseDealtWith,               &instrument_ready_for_attack             },
  { STAttackDealtWith,                &instrument_ready_for_defense            },
  { STSolutionSolver,                 &activate_output                         },
  { STPostKeyPlaySuppressor,          &suppress_output                         }
};

enum
{
  nr_tree_slice_inserters = (sizeof tree_slice_inserters
                             / sizeof tree_slice_inserters[0])
};

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 * @param si identifies slice where to start
 */
void stip_insert_output_plaintext_tree_slices(slice_index si)
{
  stip_structure_traversal st;
  instrumentation_state state =
      { { no_goal, initsquare },
        output_suppressed,
        end_of_solution_writer_not_inserted
      };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    tree_slice_inserters,
                                    nr_tree_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static attack_type pending_decoration = attack_regular;

/* Write a possibly pending move decoration
 */
void output_plaintext_tree_write_pending_move_decoration(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (pending_decoration)
  {
    case attack_try:
      StdString(" ?");
      break;

    case attack_key:
      StdString(" !");
      if (OptFlag[beep])
        produce_beep();
      break;

    default:
      break;
  }

  pending_decoration = attack_regular;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the decoration (! or ?) for the first move if appropriate
 * @param type identifies decoration to be added
 */
void output_plaintext_tree_remember_move_decoration(attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  pending_decoration = type;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a move
 */
void output_plaintext_tree_write_move(void)
{
  unsigned int const move_depth = nbply+output_plaintext_tree_nr_move_inversions;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  Message(NewLine);

  sprintf(GlobalStr,"%*c%3u.",4*move_depth-8,blank,move_depth/2);
  StdString(GlobalStr);
  if (move_depth%2==1)
    StdString("..");

  output_plaintext_write_move(nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
