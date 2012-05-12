#include "output/plaintext/tree/tree.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/battle_play/check_detector.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/move_inversion_counter.h"
#include "solving/trivial_end_filter.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/threat_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "debugging/trace.h"

#include <assert.h>

static void insert_zugzwang_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
      alloc_zugzwang_writer_slice(),
      alloc_threat_writer_slice(),
      alloc_move_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices_behind_proxy(slices[si].next2,prototypes,nr_prototypes,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_writer_for_move_in_parent(slice_index si,
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
    branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototypes[] =
    {
      alloc_move_writer_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }
  else if (st->context==stip_traversal_context_attack)
  {
    slice_index const prototypes[] =
    {
      alloc_move_writer_slice(),
      alloc_check_detector_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    attack_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.goal_handler.goal.type!=no_goal)
  {
    slice_index const prototype = alloc_goal_writer_slice(slices[si].u.goal_handler.goal);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
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

static structure_traversers_visitors const regular_writer_inserters[] =
{
  { STDefenseAdapter,    &insert_writer_for_move_in_parent },
  { STHelpAdapter,       &stip_structure_visitor_noop      },
  { STMoveInverter,      &insert_move_inversion_counter    },
  { STThreatSolver,      &insert_zugzwang_writer           },
  { STPlaySuppressor,    &stip_structure_visitor_noop      },
  { STMove,              &insert_move_writer               },
  { STGoalReachedTester, &insert_goal_writer               }
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
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    regular_writer_inserters,
                                    nr_regular_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_try_writer(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_substitute(si,alloc_try_writer());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refutation_intro_writer(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_refutations_intro_writer_slice();
    defense_branch_insert_slices_behind_proxy(slices[si].next2,&prototype,1,si);
  }

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
    defense_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const try_writer_inserters[] =
{
  { STSetplayFork,            &stip_traverse_structure_children_pipe },
  { STKeyWriter,              &substitute_try_writer                 },
  { STRefutationsSolver,      &insert_refutation_intro_writer        },
  { STRefutationsIntroWriter, &insert_refutation_writer              },
  { STEndOfBranchGoal,        &stip_traverse_structure_children_pipe }
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
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    try_writer_inserters,
                                    nr_try_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_end_of_solution_writer(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_top)
  {
    slice_index const prototype = alloc_end_of_solution_writer_slice();
    branch_insert_slices(si,&prototype,1);
  }

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
  stip_traverse_structure_children(si,st);
  *is_postkey_play = save_is_postkey_play;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_key_writer(slice_index si, stip_structure_traversal *st)
{
  boolean const * const is_postkey_play = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!*is_postkey_play)
  {
    slice_index const prototype = alloc_key_writer();
    defense_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_postkey_writers(slice_index si, stip_structure_traversal *st)
{
  boolean const * const is_postkey_play = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*is_postkey_play && st->context==stip_traversal_context_defense)
  {
    slice_index const prototypes[] =
    {
      /* indicate check in the diagram position */
      alloc_check_detector_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

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

  if (st->context==stip_traversal_context_defense)
    stip_traverse_structure_children(si,st);
  else if (*is_postkey_play)
  {
    slice_index const prototype = alloc_refuting_variation_writer_slice();
    attack_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_key_writer_goal(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototype = alloc_key_writer();
    defense_branch_insert_slices_behind_proxy(slices[si].next2,
                                              &prototype,1,
                                              si);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void get_fork_of_my_own(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_defense)
    slices[si].next2 = stip_deep_copy(slices[si].next2);

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const root_writer_inserters[] =
{
  { STSetplayFork,        &stip_traverse_structure_children_pipe },
  { STHelpAdapter,        &stip_structure_visitor_noop           },
  { STAttackAdapter,      &insert_end_of_solution_writer         },
  { STDefenseAdapter,     &remember_postkey_play                 },
  { STEndOfBranchGoal,    &insert_key_writer_goal                },
  { STNotEndOfBranchGoal, &insert_refuting_variation_writer      },
  { STEndOfBranch,        &get_fork_of_my_own                    }, /* saves some moves in capzug stipulations*/
  { STNotEndOfBranch,     &insert_postkey_writers                },
  { STReadyForDefense,    &insert_key_writer                     }
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
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_binary,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_end_of_branch,
                                                &stip_traverse_structure_children_pipe);
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

static structure_traversers_visitors const goal_writer_slice_inserters[] =
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
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
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
