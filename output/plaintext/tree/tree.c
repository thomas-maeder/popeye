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

typedef struct
{
  stip_length_type length;
  stip_length_type min_length;
} writer_insertion_state;

static void remember_length(slice_index si, stip_structure_traversal *st)
{
  writer_insertion_state * const state = st->param;
  stip_length_type const save_length = state->length;
  stip_length_type const save_min_length = state->min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->length = slices[si].u.branch.length;
  state->min_length = slices[si].u.branch.min_length;
  stip_traverse_structure_children(si,st);
  state->length = save_length;
  state->min_length = save_min_length;

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

static void insert_continuation_writers(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  remember_length(si,st);

  {
    slice_index const prototypes[] =
    {
      alloc_continuation_writer_slice(),
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

static void instrument_ready_for_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prototypes[] =
    {
      alloc_variation_writer_slice(length-1,min_length-1),
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_output_plaintext_tree_decoration_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }
  remember_length(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_try_solver(slice_index si, stip_structure_traversal *st)
{
  writer_insertion_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_try_writer(),
      alloc_refutation_writer_slice(),
      alloc_variation_writer_slice(state->length-1,state->min_length-1),
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

static void instrument_defense_adapter(slice_index si,
                                       stip_structure_traversal *st)
{
  writer_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  remember_length(si,st);

  if (state->length==0)
  {
    /* we are solving post key play - instrument accordingly */

    slice_index const prototypes[] =
    {
      alloc_output_plaintext_tree_check_writer_slice(),
      alloc_refuting_variation_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };

    /* start at predecessor - end of phase writer is inserted before defense
     * root*/
    battle_branch_insert_slices(slices[si].prev,prototypes,nr_prototypes);
  }

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
  pipe_append(si,alloc_end_of_phase_writer_slice());

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
      alloc_goal_writer_slice(slices[si].u.goal_writer.goal)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    leaf_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  {
    /* the following hack is (currently) required to suppress the output of
     * short mates in selfmate */
    slice_index const variation_writer = branch_find_slice(STVariationWriter,si);
    if (variation_writer!=no_slice)
    {
      slices[variation_writer].u.branch.length = slack_length_battle;
      slices[variation_writer].u.branch.min_length = slack_length_battle-2;
    }
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors writer_inserters[] =
{
  { STMoveInverter,          &instrument_move_inverter     },
  { STReadyForAttack,        &insert_continuation_writers  },
  { STTrySolver,             &instrument_try_solver        },
  { STThreatSolver,          &instrument_threat_solver     },
  { STPostKeyPlaySuppressor, &stip_structure_visitor_noop  },
  { STDefenseAdapter,        &instrument_defense_adapter   },
  { STReadyForDefense,       &instrument_ready_for_defense },
  { STGoalReachedTesting,    &instrument_goal_testing      },
  { STHelpAdapter,           &stip_structure_visitor_noop  },
  { STSeriesAdapter,         &stip_structure_visitor_noop  }
};

enum
{
  nr_writer_inserters = sizeof writer_inserters / sizeof writer_inserters[0]
};

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


static structure_traversers_visitors root_writer_inserters[] =
{
  { STAttackAdapter,  &instrument_attack_adapter   },
  { STDefenseAdapter, &stip_structure_visitor_noop },
  { STHelpAdapter,    &stip_structure_visitor_noop }
};

enum
{
  nr_root_writer_inserters = (sizeof root_writer_inserters
                              / sizeof root_writer_inserters[0])
};

/* Insert the writer slices
 * @param si identifies slice where to start
 */
static void insert_writer_slices(slice_index si)
{
  writer_insertion_state state = { 0, 0 };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,writer_inserters,nr_writer_inserters);
  stip_traverse_structure(si,&st);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    root_writer_inserters,
                                    nr_root_writer_inserters);
  stip_traverse_structure(si,&st);

  {
    slice_index const continuation_writer = branch_find_slice(STContinuationWriter,si);
    slice_index const prev = slices[continuation_writer].prev;
    assert(continuation_writer!=no_slice);
    assert(prev!=no_slice);
    if (slices[prev].type==STKeyWriter || slices[prev].type==STTryWriter)
      pipe_remove(continuation_writer);
  }

  insert_illegal_selfcheck_writer(si);

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

  state->goal = slices[si].u.goal_writer.goal;
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
  { STContinuationWriter,             &remove_continuation_writer_if_unused },
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
  insert_writer_slices(si);
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
