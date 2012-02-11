#include "output/plaintext/tree/tree.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/battle_play/check_detector.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/move_inversion_counter.h"
#include "solving/trivial_end_filter.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "trace.h"

#include <assert.h>

static void instrument_threat_solver(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_zugzwang_writer_slice();
    battle_branch_insert_slices(slices[si].u.fork.fork,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_adapter_regular(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_nested)
  {
    slice_index const prototypes[] =
    {
      alloc_move_writer_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_ready_for_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_move_writer_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_continuation_writers(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_move_writer_slice(),
      alloc_check_detector_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.goal_handler.goal.type!=no_goal)
  {
    slice_index const prototypes[] =
    {
      alloc_goal_writer_slice(slices[si].u.goal_handler.goal)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    leaf_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_inversion_counter(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_output_plaintext_move_inversion_counter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors regular_writer_inserters[] =
{
  { STMoveInverter,      &insert_move_inversion_counter      },
  { STThreatSolver,      &instrument_threat_solver           },
  { STPlaySuppressor,    &stip_structure_visitor_noop        },
  { STReadyForAttack,    &insert_continuation_writers        },
  { STReadyForDefense,   &instrument_ready_for_defense       },
  { STGoalReachedTester, &instrument_goal_tester             },
  { STDefenseAdapter,    &instrument_defense_adapter_regular },
  { STHelpAdapter,       &stip_structure_visitor_noop        }
};

enum
{
  nr_regular_writer_inserters
  = sizeof regular_writer_inserters / sizeof regular_writer_inserters[0]
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
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_pipe);
  stip_structure_traversal_override(&st,
                                    regular_writer_inserters,
                                    nr_regular_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_key_writer(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_substitute(si,alloc_try_writer());

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refutation_writer(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_refutation_writer_slice();
    defense_branch_insert_slices(slices[si].u.fork.fork,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors try_writer_inserters[] =
{
  { STSetplayFork,       &stip_traverse_structure_pipe },
  { STKeyWriter,         &instrument_key_writer        },
  { STRefutationsSolver, &insert_refutation_writer     }
};

enum
{
  nr_try_writer_inserters
  = sizeof try_writer_inserters / sizeof try_writer_inserters[0]
};

static void insert_try_writers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    try_writer_inserters,
                                    nr_try_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_attack_adapter(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_root)
  {
    slice_index const prototypes[] =
    {
      alloc_end_of_solution_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root_ready_for_defense(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_key_writer();
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_adapter(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_root)
  {
    /* we are solving post key play */
    slice_index const prototypes[] =
    {
      /* indicate check in the diagram position */
      alloc_check_detector_slice(),
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_refuting_variation_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void get_fork_of_my_own(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.fork.fork = stip_deep_copy(slices[si].u.fork.fork);
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors root_writer_inserters[] =
{
  { STSetplayFork,       &stip_traverse_structure_pipe      },
  { STAttackAdapter,     &instrument_attack_adapter         },
  { STDefenseAdapter,    &instrument_defense_adapter        },
  { STEndOfBranch,       &get_fork_of_my_own                },
  { STEndOfBranchGoal,   &get_fork_of_my_own                },
  { STReadyForDefense,   &instrument_root_ready_for_defense },
  { STRefutationsSolver, &stip_traverse_structure_pipe      },
  { STConstraintSolver,  &stip_traverse_structure_pipe      },
  { STHelpAdapter,       &stip_structure_visitor_noop       }
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_pipe);
  stip_structure_traversal_override(&st,
                                    root_writer_inserters,
                                    nr_root_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  Goal goal;
  boolean branch_has_key_writer;
} leaf_optimisation_state_structure;

/* Remember that we are about to deal with a goal (and which one)
 */
static void remember_goal(slice_index si, stip_structure_traversal *st)
{
  leaf_optimisation_state_structure * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->goal.type==no_goal);

  state->goal = slices[si].u.goal_handler.goal;
  stip_traverse_structure_children(si,st);
  state->goal.type = no_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that the current branch has a key writer
 */
static void remember_key_writer(slice_index si, stip_structure_traversal *st)
{
  leaf_optimisation_state_structure * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->goal.type!=no_goal)
  {
    state->branch_has_key_writer = true;
    stip_traverse_structure_children(si,st);
    state->branch_has_key_writer = false;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remove an unused slice dealing with a check that we don't intend to write
 */
static void remove_check_handler_if_unused(slice_index si, stip_structure_traversal *st)
{
  leaf_optimisation_state_structure const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->goal.type!=no_goal
      && output_plaintext_goal_writer_replaces_check_writer(state->goal.type))
    pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remove a continuation writer in a leaf branch where we already have a key
 * writer
 */
static void remove_continuation_writer_if_unused(slice_index si,
                                                 stip_structure_traversal *st)
{
  leaf_optimisation_state_structure const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->branch_has_key_writer)
    pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_writer_slice_inserters[] =
{
  { STGoalReachedTester,              &remember_goal                        },
  { STKeyWriter,                      &remember_key_writer                  },
  { STMoveWriter,                     &remove_continuation_writer_if_unused },
  { STCheckDetector,                  &remove_check_handler_if_unused       },
  { STOutputPlaintextTreeCheckWriter, &remove_check_handler_if_unused       }
};

enum
{
  nr_goal_writer_slice_inserters = (sizeof goal_writer_slice_inserters
                                    / sizeof goal_writer_slice_inserters[0])
};

/* Optimise away superfluous slices in leaf branches
 * @param si identifies slice where to start
 */
static void optimise_leaf_slices(slice_index si)
{
  stip_structure_traversal st;
  leaf_optimisation_state_structure state = { { no_goal, initsquare }, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_pipe);
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
 * @param is_setplay is si part of set play?
 */
void stip_insert_output_plaintext_tree_slices(slice_index si, boolean is_setplay)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  insert_regular_writer_slices(si);
  insert_root_writer_slices(si);
  if (!is_setplay && OptFlag[soltout]) /* this includes OptFlag[solessais] */
    insert_try_writers(si);
  optimise_leaf_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a move
 */
void output_plaintext_tree_write_move(void)
{
  unsigned int const move_depth = nbply+output_plaintext_nr_move_inversions;

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
