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
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/trivial_variation_filter.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/tree/move_inversion_counter.h"
#include "platform/beep.h"
#include "trace.h"

#include <assert.h>

static void instrument_move_inverter(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_output_plaintext_tree_move_inversion_counter_slice());
  pipe_append(si,alloc_end_of_phase_writer_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_continuation_writers(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
      alloc_move_writer_slice(),
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_output_plaintext_tree_decoration_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

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
    pipe_set_successor(writer,slices[si].u.fork.fork);
    slice_set_predecessor(writer,si);
    slices[si].u.fork.fork = writer;
  }

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
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_output_plaintext_tree_decoration_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_testing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_goal_writer_slice(slices[si].u.goal_handler.goal)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    leaf_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors regular_writer_inserters[] =
{
  { STMoveInverter,          &instrument_move_inverter     },
  { STReadyForAttack,        &insert_continuation_writers  },
  { STThreatSolver,          &instrument_threat_solver     },
  { STPostKeyPlaySuppressor, &stip_structure_visitor_noop  },
  { STReadyForDefense,       &instrument_ready_for_defense },
  { STGoalReachedTesting,    &instrument_goal_testing      },
  { STHelpAdapter,           &stip_structure_visitor_noop  },
  { STSeriesAdapter,         &stip_structure_visitor_noop  }
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
  stip_structure_traversal_override(&st,
                                    regular_writer_inserters,
                                    nr_regular_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  trivial_varation_filter_insertion_state_global,
  trivial_varation_filter_insertion_state_attack,
  trivial_varation_filter_insertion_state_defense
} trivial_varation_filter_insertion_state_type;

static void trivial_varation_filter_end_of_branch(slice_index si,
                                                  stip_structure_traversal *st)
{
  trivial_varation_filter_insertion_state_type * const state = st->param;
  trivial_varation_filter_insertion_state_type const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  *state = trivial_varation_filter_insertion_state_global;
  stip_traverse_structure(slices[si].u.fork.fork,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void trivial_varation_filter_insert(slice_index si,
                                           stip_structure_traversal *st)
{
  trivial_varation_filter_insertion_state_type const * const state = st->param;
  slice_index variation_writer = no_slice;
  boolean found_goal_tester = false;
  slice_index current = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  trivial_varation_filter_end_of_branch(si,st);

  if (*state==trivial_varation_filter_insertion_state_defense)
  {
    while (slices[current].type!=STLeaf)
    {
      if (slices[current].type==STMoveWriter)
        variation_writer = current;
      else if (slices[current].type==STGoalReachedTesting)
        found_goal_tester = true;

      current = slices[current].u.pipe.next;
    }

    if (found_goal_tester && variation_writer!=no_slice)
      pipe_append(slices[variation_writer].prev,
                  alloc_trivial_variation_filter_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void trivial_varation_filter_insertion_attack(slice_index si,
                                              stip_structure_traversal *st)
{
  trivial_varation_filter_insertion_state_type * const state = st->param;
  trivial_varation_filter_insertion_state_type const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = trivial_varation_filter_insertion_state_attack;
  stip_traverse_structure_children(si,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void trivial_varation_filter_insertion_defense(slice_index si,
                                               stip_structure_traversal *st)
{
  trivial_varation_filter_insertion_state_type * const state = st->param;
  trivial_varation_filter_insertion_state_type const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = trivial_varation_filter_insertion_state_defense;
  stip_traverse_structure_children(si,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors trivial_varation_filter_inserters[] =
{
  { STAttackAdapter,           &trivial_varation_filter_insertion_attack  },
  { STReadyForAttack,          &trivial_varation_filter_insertion_attack  },
  { STDefenseAdapter,          &trivial_varation_filter_insertion_defense },
  { STReadyForDefense,         &trivial_varation_filter_insertion_defense },
  { STEndOfBranchGoal,         &trivial_varation_filter_insert            },
  { STEndOfBranchGoalImmobile, &trivial_varation_filter_insert            },
  { STEndOfBranch,             &trivial_varation_filter_end_of_branch     },
  { STEndOfBranchForced,       &trivial_varation_filter_end_of_branch     }
};

enum
{
  nr_trivial_varation_filter_inserters
  = (sizeof trivial_varation_filter_inserters
     / sizeof trivial_varation_filter_inserters[0])
};

static void insert_trivial_varation_filters(slice_index si)
{
  stip_structure_traversal st;
  trivial_varation_filter_insertion_state_type state = trivial_varation_filter_insertion_state_global;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    trivial_varation_filter_inserters,
                                    nr_trivial_varation_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_try_solver(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_try_writer(),
      alloc_refutation_writer_slice(),
      alloc_move_writer_slice(),
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_output_plaintext_tree_decoration_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors try_writer_inserters[] =
{
  { STSetplayFork, &stip_traverse_structure_pipe },
  { STTrySolver,   &instrument_try_solver        }
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

  {
    slice_index const prototypes[] =
    {
      alloc_end_of_solution_writer_slice(),
      alloc_key_writer(),
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_output_plaintext_tree_decoration_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
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

  {
    /* we are solving post key play */
    slice_index const prototypes[] =
    {
      /* indicate check in the diagram position */
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_refuting_variation_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors root_writer_inserters[] =
{
  { STSetplayFork,    &stip_traverse_structure_pipe },
  { STAttackAdapter,  &instrument_attack_adapter    },
  { STDefenseAdapter, &instrument_defense_adapter   },
  { STHelpAdapter,    &stip_structure_visitor_noop  },
  { STSeriesAdapter,  &stip_structure_visitor_noop  },
  { STNot,            &stip_structure_visitor_noop  }
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
  stip_structure_traversal_override(&st,
                                    root_writer_inserters,
                                    nr_root_writer_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_illegal_selfcheck_writer(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_illegal_selfcheck_writer_slice()
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
  { STGoalReachedTesting,             &remember_goal                        },
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

  insert_regular_writer_slices(si);
  insert_trivial_varation_filters(si);
  insert_try_writers(si);
  insert_root_writer_slices(si);
  insert_illegal_selfcheck_writer(si);
  optimise_leaf_slices(si);

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
