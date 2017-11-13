#include "output/plaintext/tree/tree.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/slice_insertion.h"
#include "solving/trivial_end_filter.h"
#include "solving/ply.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/goal_writer.h"
#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/threat_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/exclusive.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"
#include "options/options.h"
#include "conditions/conditions.h"

#include "debugging/assert.h"

static void insert_zugzwang_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
      alloc_pipe(STOutputPlainTextZugzwangByDummyMoveCheckWriter),
      alloc_output_plaintext_tree_zugzwang_writer_slice(),
      alloc_output_plaintext_tree_threat_writer_slice(),
      alloc_output_plaintext_tree_move_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),prototypes,nr_prototypes,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_writer_for_move_in_parent(slice_index si,
                                             stip_structure_traversal *st)
{
  boolean const * const attack_played = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_nested
      && *attack_played)
  {
    slice_index const prototypes[] =
    {
      alloc_output_plaintext_tree_move_writer_slice(),
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_insert_move_writer_attack(slice_index si,
                                         stip_structure_traversal *st)
{
  slice_index const prototypes[] =
  {
    alloc_output_plaintext_tree_move_writer_slice(),
    alloc_output_plaintext_tree_check_writer_slice()
  };
  enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
  stip_structure_traversal traversal_insertion;
  branch_slice_insertion_state_type state = { 0 };
  attack_branch_prepare_slice_insertion(si,
                                        prototypes,nr_prototypes,
                                        &traversal_insertion,&state);

  /* prevent 2nd operand from writing the same move again */
  stip_structure_traversal_override_single(&traversal_insertion,STAnd,&slice_insertion_visit_pipe);

  stip_traverse_structure(si,&traversal_insertion);

  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  if (CondFlag[exclusive])
  {
    slice_index const prototype = alloc_exclusive_chess_undecidable_writer_tree_slice();
    move_insert_slices(si,st->context,&prototype,1);
  }
}

static void do_insert_move_writer_defense(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index const prototypes[] =
  {
    alloc_output_plaintext_tree_move_writer_slice(),
    alloc_output_plaintext_tree_check_writer_slice()
  };
  enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
  defense_branch_insert_slices(si,prototypes,nr_prototypes);

  if (CondFlag[exclusive])
  {
    slice_index const prototype = alloc_exclusive_chess_undecidable_writer_tree_slice();
    move_insert_slices(si,st->context,&prototype,1);
  }
}

static void insert_move_writer(slice_index si, stip_structure_traversal *st)
{
  boolean * const attack_played = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_defense && *attack_played)
  {
    do_insert_move_writer_defense(si,st);

    *attack_played = false;
    stip_traverse_structure_children_pipe(si,st);
    *attack_played = true;
  }
  else if (st->context==stip_traversal_context_attack)
  {
    do_insert_move_writer_attack(si,st);
    stip_traverse_structure_children_pipe(si,st);
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_writer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_U(si).goal_handler.goal.type!=no_goal)
  {
    slice_index const prototype = alloc_output_plaintext_goal_writer_slice(SLICE_U(si).goal_handler.goal);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  if (CondFlag[ohneschach])
  {
    slice_index const prototype = alloc_ohneschach_detect_undecidable_goal_slice();
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
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

static void insert_writer_remember_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  boolean * const attack_played = st->param;
  boolean const save_attack_played = *attack_played;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *attack_played = true;
  stip_traverse_structure_children_pipe(si,st);
  *attack_played = save_attack_played;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const regular_writer_inserters[] =
{
  { STReadyForAttack,      &insert_writer_remember_attack },
  { STDefenseAdapter,      &insert_writer_for_move_in_parent      },
  { STHelpAdapter,         &stip_structure_visitor_noop           },
  { STThreatSolver,        &insert_zugzwang_writer                },
  { STPlaySuppressor,      &stip_structure_visitor_noop           },
  { STMove,                &insert_move_writer                    },
  { STGoalReachedTester,   &insert_goal_writer                    }
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
  boolean attack_played = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&attack_played);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    regular_writer_inserters,
                                    nr_regular_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const move_inversion_counter_inserters[] =
{
  { STMoveInverter,        &insert_move_inversion_counter         },
  { STMoveInverterSetPlay, &insert_move_inversion_counter_setplay },
  { STPlaySuppressor,      &stip_structure_visitor_noop           },
};

enum
{
  nr_move_inversion_counter_inserters
  = sizeof move_inversion_counter_inserters / sizeof move_inversion_counter_inserters[0]
};

/* Instrument the solving machinery with move inversion counter slices
 * @param si identifies slice where to start
 */
void solving_insert_move_inversion_counter_slices(slice_index si)
{
  stip_structure_traversal st;
  boolean attack_played = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&attack_played);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    move_inversion_counter_inserters,
                                    nr_move_inversion_counter_inserters);
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

  pipe_substitute(si,alloc_output_plaintext_tree_try_writer());

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
    slice_index const prototype = alloc_output_plaintext_tree_refutations_intro_writer_slice();
    defense_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
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
    slice_index const prototype = alloc_output_plaintext_tree_refutation_writer_slice();
    defense_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const try_writer_inserters[] =
{
  { STOutputPlainTextKeyWriter,              &substitute_try_writer                 },
  { STRefutationsSolver,                     &insert_refutation_intro_writer        },
  { STOutputPlainTextRefutationsIntroWriter, &insert_refutation_writer              },
  { STEndOfBranchGoal,                       &stip_traverse_structure_children_pipe },
  { STEndOfBranch,                           &stip_structure_visitor_noop           }
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
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
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

  stip_traverse_structure_children_pipe(si,st);

  if (st->level==structure_traversal_level_top)
  {
    slice_index const prototype = alloc_output_plaintext_end_of_solution_writer_slice();
    slice_insertion_insert(si,&prototype,1);
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
  stip_traverse_structure_children_pipe(si,st);
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
    slice_index const prototype = alloc_output_plaintext_tree_key_writer();
    defense_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

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
      alloc_output_plaintext_tree_check_writer_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children_pipe(si,st);

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
    stip_traverse_structure_children_pipe(si,st);
  else if (*is_postkey_play)
  {
    slice_index const prototype = alloc_output_plaintext_tree_refuting_variation_writer_slice();
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
    slice_index const prototypes[] = {
        alloc_output_plaintext_tree_key_writer()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    stip_structure_traversal st_nested;
    branch_slice_insertion_state_type state = { 0 };

    defense_branch_prepare_slice_insertion_behind_proxy(SLICE_NEXT2(si),
                                               prototypes,nr_prototypes,
                                               si,
                                               &state,
                                               &st_nested);

    /* prevent 1st operand from writing the key mark; 2nd operand will write it */
    stip_structure_traversal_override_single(&st_nested,STAnd,&slice_insertion_visit_binary_skip_next1);

    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);
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
    SLICE_NEXT2(si) = stip_deep_copy(SLICE_NEXT2(si));

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const root_writer_inserters[] =
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

typedef struct
{
  Goal goal;
  boolean branch_has_key_writer;
  boolean branch_towards_goal;
  boolean and_goal;
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

  state->goal = SLICE_U(si).goal_handler.goal;
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
    stip_traverse_structure_children_pipe(si,st);
    state->branch_has_key_writer = false;
  }
  else
    stip_traverse_structure_children_pipe(si,st);

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

  stip_traverse_structure_children_pipe(si,st);

  /* remove check writer if the check is preempted by the goal (e.g. mate)
   * or if the other leg of an STAnd is supposed to write the check */
  if ((state->goal.type!=no_goal
       && output_goal_preempts_check(state->goal.type))
      || state->and_goal)
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

  stip_traverse_structure_children_pipe(si,st);

  if (state->branch_has_key_writer)
    pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_towards_goal(slice_index si, stip_structure_traversal *st)
{
  leaf_optimisation_state_structure * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (SLICE_NEXT2(si)!=no_slice)
  {
    assert(!state->branch_towards_goal);
    state->branch_towards_goal = true;
    stip_traverse_structure_next_branch(si,st);
    state->branch_towards_goal = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_and_goal(slice_index si, stip_structure_traversal *st)
{
  leaf_optimisation_state_structure * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->branch_towards_goal)
  {
    boolean const save_and_goal = state->and_goal;
    state->and_goal = true;
    stip_traverse_structure_children_pipe(si,st);
    state->and_goal = save_and_goal;
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  if (SLICE_NEXT2(si)!=no_slice)
    stip_traverse_structure_next_branch(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const goal_writer_slice_optimisers[] =
{
  { STGoalReachedTester,              &remember_goal                        },
  { STOutputPlainTextKeyWriter,       &remember_key_writer                  },
  { STOutputPlainTextMoveWriter,      &remove_continuation_writer_if_unused },
  { STOutputPlaintextTreeCheckWriter, &remove_check_handler_if_unused       },
  { STEndOfBranchGoal,                &remember_towards_goal                },
  { STAnd,                            &remember_and_goal                    }
};

enum
{
  nr_goal_writer_slice_optimisers = (sizeof goal_writer_slice_optimisers
                                     / sizeof goal_writer_slice_optimisers[0])
};

/* Optimise away superfluous slices in leaf branches
 * @param si identifies slice where to start
 */
static void optimise_leaf_slices(slice_index si)
{
  stip_structure_traversal st;
  leaf_optimisation_state_structure state = {
      { no_goal, initsquare },
      false,
      false,
      false
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    goal_writer_slice_optimisers,
                                    nr_goal_writer_slice_optimisers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 * @param si identifies slice where to start
 * @param is_setplay is si part of set play?
 */
void solving_insert_output_plaintext_tree_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  insert_regular_writer_slices(si);
  insert_root_writer_slices(si);
  if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
    insert_try_writers(si);
  optimise_leaf_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int measure_move_depth(ply curr_ply)
{
  ply const parent = parent_ply[curr_ply];

  if (parent==ply_retro_move)
    return output_plaintext_nr_move_inversions;
  else
    return measure_move_depth(parent)+1;
}

/* Write a move
 */
void output_plaintext_tree_write_move(void)
{
  unsigned int const move_depth = measure_move_depth(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  protocol_fprintf(stdout,"\n%*c%3u.",4*move_depth,' ',move_depth/2+1);
  if (move_depth%2==1)
    protocol_fprintf(stdout,"%s","..");

  output_plaintext_write_move(&output_plaintext_engine,
                              stdout,
                              &output_plaintext_symbol_table);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
