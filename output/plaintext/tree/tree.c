#include "output/plaintext/tree/tree.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/decoration_writer.h"
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

typedef enum
{
  tries_suppressed,
  tries_included
} tries_state_type;

/* Do we have to insert an STEndOfSolutionWriter slice?
 */
typedef enum
{
  end_of_solution_writer_not_inserted,
  end_of_solution_writer_inserted
} end_of_solution_writer_insertion_state_type;

typedef enum
{
  illegal_selfcheck_writer_not_inserted,
  illegal_selfcheck_writer_inserted
} illegal_selfcheck_writer_insertion_state;

typedef struct
{
    output_state_type output_state;
    tries_state_type tries_state;
    end_of_solution_writer_insertion_state_type end_state;
    illegal_selfcheck_writer_insertion_state selfcheck_writer_state;
} non_goal_instrumentation_state;

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

static void instrument_attack_move(slice_index si, stip_structure_traversal *st)
{
  non_goal_instrumentation_state const * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->output_state!=output_suppressed
      && length>slack_length_battle)
  {
    slice_index const prototypes[2] =
    {
      alloc_output_plaintext_tree_check_writer_defender_filter_slice(length-1,
                                                                     min_length-1),
      alloc_output_plaintext_tree_decoration_writer_defender_filter_slice(length-1,
                                                                          min_length-1)
    };
    battle_branch_insert_slices(si,prototypes,2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_variation_writers(non_goal_instrumentation_state state,
                                     slice_index si,
                                     stip_length_type length,
                                     stip_length_type min_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (state.output_state!=output_suppressed)
  {
    if (state.output_state==output_postkeyplay_exclusively)
    {
      slice_index const prototypes[4] =
      {
        alloc_variation_writer_slice(length,min_length),
        alloc_refuting_variation_writer_slice(length,min_length),
        alloc_output_plaintext_tree_check_writer_attacker_filter_slice(length,min_length),
        alloc_output_plaintext_tree_decoration_writer_attacker_filter_slice(length,min_length)
      };
      battle_branch_insert_slices(si,prototypes,4);
    }
    else
    {
      slice_index const prototypes[3] =
      {
        alloc_variation_writer_slice(length,min_length),
        alloc_output_plaintext_tree_check_writer_attacker_filter_slice(length,min_length),
        alloc_output_plaintext_tree_decoration_writer_attacker_filter_slice(length,min_length)
      };
      battle_branch_insert_slices(si,prototypes,3);
    }
  }
  else if (state.tries_state==tries_included)
  {
    slice_index const prototypes[3] =
    {
      alloc_variation_writer_slice(length,min_length),
      alloc_output_plaintext_tree_check_writer_attacker_filter_slice(length,min_length),
      alloc_output_plaintext_tree_decoration_writer_attacker_filter_slice(length,min_length)
    };
    battle_branch_insert_slices(si,prototypes,3);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_move(slice_index si,
                                    stip_structure_traversal *st)
{
  non_goal_instrumentation_state const * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_variation_writers(*state,
                           slices[si].u.pipe.next,
                           length-1,min_length-1);
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_self_defense(slice_index si,
                                    stip_structure_traversal *st)
{
  non_goal_instrumentation_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_variation_writers(*state,
                           slices[si].u.branch_fork.towards_goal,
                           2,0);
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_reflex_attack_branch(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[1] =
    {
      alloc_output_plaintext_tree_check_writer_defender_filter_slice(2,0)
    };
    battle_branch_insert_slices(slices[si].u.branch_fork.towards_goal,
                                prototypes,1);
  }

  stip_traverse_structure_children(si,st);

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
  non_goal_instrumentation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->tries_state = tries_included;
  stip_traverse_structure_children(si,st);
  state->tries_state = tries_suppressed;

  pipe_append(si,alloc_try_writer());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_attack_move_played(slice_index si,
                                          stip_structure_traversal *st)
{
  non_goal_instrumentation_state * const state = st->param;

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
  non_goal_instrumentation_state * const state = st->param;
  output_state_type const save_output_state = state->output_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  state->output_state = output_postkeyplay_exclusively;
  stip_traverse_structure_children(si,st);
  state->output_state = save_output_state;

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;

    slice_index const prototypes[1] =
    {
      alloc_output_plaintext_tree_check_writer_defender_filter_slice(length,
                                                                     min_length)
    };
    battle_branch_insert_slices(si,prototypes,1);
  }

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

static void suppress_output(slice_index si, stip_structure_traversal *st)
{
  non_goal_instrumentation_state * const state = st->param;
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

static void prepend_illegal_selfcheck_writer(slice_index si, stip_structure_traversal *st)
{
  non_goal_instrumentation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->selfcheck_writer_state==illegal_selfcheck_writer_inserted)
    stip_traverse_structure_children(si,st);
  else
  {
    state->selfcheck_writer_state = illegal_selfcheck_writer_inserted;
    stip_traverse_structure_children(si,st);
    state->selfcheck_writer_state = illegal_selfcheck_writer_not_inserted;

    pipe_append(slices[si].prev,alloc_illegal_selfcheck_writer_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root_attack_fork(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_key_writer();
    root_branch_insert_slices(slices[si].u.branch_fork.towards_goal,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors tree_slice_inserters[] =
{
  { STSetplayFork,                    &instrument_setplay_fork          },
  { STRootAttackFork,                 &instrument_root_attack_fork      },
  { STMoveInverterRootSolvableFilter, &instrument_move_inverter         },
  { STMoveInverterSolvableFilter,     &instrument_move_inverter         },
  { STAttackMovePlayed,               &instrument_attack_move_played    },
  { STDefenseRoot,                    &instrument_defense_root          },
  { STContinuationSolver,             &instrument_continuation_solver   },
  { STTrySolver,                      &instrument_try_solver            },
  { STThreatSolver,                   &instrument_threat_solver         },
  { STDefenseMove,                    &instrument_defense_move          },
  { STSelfDefense,                    &instrument_self_defense          },
  { STRefutationsCollector,           &instrument_refutations_collector },
  { STSeriesRoot,                     &stip_structure_visitor_noop      },
  { STAttackRoot,                     &instrument_attack_move           },
  { STAttackMove,                     &instrument_attack_move           },
  { STAttackMoveToGoal,               &instrument_attack_move           },
  { STPostKeyPlaySuppressor,          &suppress_output                  },
  { STSelfCheckGuard,                 &prepend_illegal_selfcheck_writer },
  { STStipulationReflexAttackSolver,  &instrument_reflex_attack_branch  }
};

enum
{
  nr_tree_slice_inserters = (sizeof tree_slice_inserters
                             / sizeof tree_slice_inserters[0])
};

/* Insert the slices that are not related to a goal
 * @param si identifies slice where to start
 */
static void insert_non_goal_slices(slice_index si)
{
  stip_structure_traversal st;
  non_goal_instrumentation_state state =
  {
    output_included,
    tries_suppressed,
    end_of_solution_writer_not_inserted,
    illegal_selfcheck_writer_not_inserted
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    tree_slice_inserters,
                                    nr_tree_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that we are about to deal with a non-target goal (and which one)
 */
static void remember_goal_non_target(slice_index si,
                                     stip_structure_traversal *st)
{
  Goal * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we are only interested in the first goal checker slice of a branch */
  if (goal->type==no_goal)
  {
    Goal const save_goal = *goal;
    goal->type = goal_mate+(slices[si].type-first_goal_tester_slice_type);
    stip_traverse_structure_children(si,st);
    *goal = save_goal;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that we are about to deal with a target goal
 */
static void remember_goal_target(slice_index si, stip_structure_traversal *st)
{
  Goal * const goal = st->param;
  Goal const save_goal = *goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(goal->type==no_goal);
  goal->type = goal_target;
  goal->target = slices[si].u.goal_target_reached_tester.target;
  stip_traverse_structure_children(si,st);
  *goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remove an unused check detector
 */
static void remove_check_detector_if_unused(slice_index si,
                                            stip_structure_traversal *st)
{
  Goal const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (goal->type!=no_goal
      && output_plaintext_tree_goal_writer_replace_check_writer(*goal))
    pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a goal writer (by replacing a check writer if appropriate)
 */
static void insert_goal_writer(slice_index si, stip_structure_traversal *st)
{
  Goal const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (goal->type!=no_goal)
  {
    slice_index const writer = alloc_goal_writer_slice(*goal);
    if (output_plaintext_tree_goal_writer_replace_check_writer(*goal))
      pipe_replace(si,writer);
    else
      pipe_append(si,writer);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_writer_slice_inserters[] =
{
  { STGoalTargetReachedTester,        &remember_goal_target            },
  { STCheckDetector,                  &remove_check_detector_if_unused },
  { STOutputPlaintextTreeCheckWriter, &insert_goal_writer              }
};

enum
{
  nr_goal_writer_slice_inserters = (sizeof goal_writer_slice_inserters
                                    / sizeof goal_writer_slice_inserters[0])
};

/* Insert the slices that are related to a goal
 * @param si identifies slice where to start
 */
static void insert_goal_writer_slices(slice_index si)
{
  stip_structure_traversal st;
  SliceType type;
  Goal goal = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&goal);

  for (type = first_goal_tester_slice_type;
       type<=last_goal_tester_slice_type;
       ++type)
    stip_structure_traversal_override_single(&st,
                                             type,
                                             &remember_goal_non_target);

  stip_structure_traversal_override(&st,
                                    goal_writer_slice_inserters,
                                    nr_goal_writer_slice_inserters);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 * @param si identifies slice where to start
 */
void stip_insert_output_plaintext_tree_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  insert_non_goal_slices(si);
  insert_goal_writer_slices(si);

  {
    slice_index const prototype = alloc_key_writer();
    root_branch_insert_slices(si,&prototype,1);
  }

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
