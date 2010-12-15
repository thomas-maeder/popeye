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

static void insert_root_slices(slice_index si,
                               stip_length_type length,
                               stip_length_type min_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_end_of_solution_writer_slice(),
      alloc_key_writer(),
      alloc_output_plaintext_tree_check_writer_slice(length,min_length),
      alloc_output_plaintext_tree_decoration_writer_slice(length,min_length)
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

static void instrument_attack_root(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  insert_root_slices(si,length-1,min_length-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_continuation_writers(slice_index si,
                                        stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (length>slack_length_battle)
  {
    slice_index const prototypes[] =
    {
      alloc_continuation_writer_slice(length-1,min_length-1),
      alloc_output_plaintext_tree_check_writer_slice(length-1,min_length-1),
      alloc_output_plaintext_tree_decoration_writer_slice(length-1,min_length-1)
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

static void insert_variation_writers(slice_index si,
                                     stip_length_type length,
                                     stip_length_type min_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_variation_writer_slice(length,min_length),
      alloc_output_plaintext_tree_check_writer_slice(length,min_length),
      alloc_output_plaintext_tree_decoration_writer_slice(length,min_length)
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

static void instrument_defense_move(slice_index si,
                                    stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_variation_writers(slices[si].u.pipe.next,length-1,min_length-1);
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_self_defense(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_variation_writers(slices[si].u.branch_fork.towards_goal,2,0);
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
    stip_length_type const length = 2;
    stip_length_type const min_length = 0;
    slice_index const prototypes[] =
    {
      alloc_continuation_writer_slice(length,min_length),
      alloc_output_plaintext_tree_check_writer_slice(length,min_length)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(slices[si].u.branch_fork.towards_goal,
                                prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_try_solver(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;

    slice_index const prototypes[] =
    {
      alloc_refutation_writer_slice(),
      alloc_variation_writer_slice(length-1,min_length-1),
      alloc_output_plaintext_tree_check_writer_slice(length-1,min_length-1),
      alloc_output_plaintext_tree_decoration_writer_slice(length-1,min_length-1)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    battle_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_try_writer());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_root(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;

    slice_index const prototypes[] =
    {
      alloc_output_plaintext_tree_check_writer_slice(length,min_length),
      alloc_refuting_variation_writer_slice(length-1,min_length-1)
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

static void instrument_setplay_fork(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  insert_illegal_selfcheck_writer(slices[si].u.branch_fork.towards_goal);

  {
    slice_index end_of_phase = alloc_end_of_phase_writer_slice();
    pipe_link(end_of_phase,slices[si].u.branch_fork.towards_goal);
    slices[si].u.branch_fork.towards_goal = end_of_phase;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root_attack_fork(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    stip_length_type const length = 2;
    stip_length_type const min_length = 0;
    insert_root_slices(slices[si].u.branch_fork.towards_goal,
                       length,min_length);
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
  { STDefenseRoot,                    &instrument_defense_root          },
  { STTrySolver,                      &instrument_try_solver            },
  { STThreatSolver,                   &instrument_threat_solver         },
  { STDefenseMove,                    &instrument_defense_move          },
  { STSelfDefense,                    &instrument_self_defense          },
  { STSeriesRoot,                     &stip_structure_visitor_noop      },
  { STAttackRoot,                     &instrument_attack_root           },
  { STAttackMove,                     &insert_continuation_writers      },
  { STAttackMoveToGoal,               &insert_continuation_writers      },
  { STPostKeyPlaySuppressor,          &stip_structure_visitor_noop      },
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    tree_slice_inserters,
                                    nr_tree_slice_inserters);
  stip_traverse_structure(si,&st);

  insert_illegal_selfcheck_writer(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that we are about to deal with a non-target goal (and which one)
 */
static void remember_goal(slice_index si, stip_structure_traversal *st)
{
  Goal * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we are only interested in the first goal checker slice of a branch */
  if (goal->type==no_goal)
  {
    Goal const save_goal = *goal;
    *goal = slices[si].u.goal_writer.goal;
    stip_traverse_structure_children(si,st);
    *goal = save_goal;
  }
  else
    stip_traverse_structure_children(si,st);

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
      && output_plaintext_goal_writer_replaces_check_writer(goal->type))
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
    if (output_plaintext_goal_writer_replaces_check_writer(goal->type))
      pipe_replace(si,writer);
    else
      pipe_append(si,writer);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_writer_slice_inserters[] =
{
  { STGoalReachedTesting,             &remember_goal                   },
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
  Goal goal = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&goal);
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
