#include "output/plaintext/tree/tree.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_detector.h"
#include "output/plaintext/tree/continuation_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/battle_play_solution_writer.h"
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
  postkey_play_unknown,
  postkey_play_exclusively,
  postkey_play_included,
} postkey_play_state_type;

static postkey_play_state_type postkey_play_state;

/* The following enumeration type represents the state of variation
 * writer insertion
 */
typedef enum
{
  variation_writer_none,
  variation_writer_needed,
  variation_writer_inserted
} variation_writer_insertion_state_type;

static variation_writer_insertion_state_type variation_writer_insertion_state;

typedef enum
{
  check_detector_defender_filter_unknown,
  check_detector_defender_filter_needed,
  check_detector_defender_filter_inserted
} check_detector_defender_filter_inseration_state_type;

static
check_detector_defender_filter_inseration_state_type
check_detector_defender_filter_inseration_state;

static void instrument_self_defense(slice_index si,
                                    stip_structure_traversal *st)
{
  variation_writer_insertion_state_type const
      save_state = variation_writer_insertion_state;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  variation_writer_insertion_state = save_state;
  stip_traverse_structure(to_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_binary(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (variation_writer_insertion_state==variation_writer_needed)
  {
    variation_writer_insertion_state = variation_writer_inserted;
    stip_traverse_structure(slices[si].u.binary.op1,st);
    stip_traverse_structure(slices[si].u.binary.op2,st);
    variation_writer_insertion_state = variation_writer_needed;

    /* TODO calculate length */
    pipe_append(slices[si].prev,
                alloc_variation_writer_slice(slack_length_battle,
                                             slack_length_battle));
  }
  else
  {
    stip_traverse_structure(slices[si].u.binary.op1,st);
    stip_traverse_structure(slices[si].u.binary.op2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_reached_tester(slice_index si,
                                           stip_structure_traversal *st)
{
  Goal * const goal = st->param;
  Goal const save_goal = *goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goal = slices[si].u.goal_reached_tester.goal;
  stip_traverse_structure_children(si,st);
  *goal = save_goal;

  if (variation_writer_insertion_state==variation_writer_needed)
    pipe_append(slices[si].prev,
                alloc_variation_writer_slice(slack_length_battle,
                                             slack_length_battle));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  Goal const * const goal = st->param;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (goal->type!=no_goal)
    pipe_append(slices[si].prev,alloc_goal_writer_slice(*goal));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_attack_move(slice_index si,
                                   stip_structure_traversal *st)
{
  variation_writer_insertion_state_type const
      save_var_state = variation_writer_insertion_state;
  check_detector_defender_filter_inseration_state_type const
      save_detector_state = check_detector_defender_filter_inseration_state;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  variation_writer_insertion_state = variation_writer_none;
  check_detector_defender_filter_inseration_state
      = check_detector_defender_filter_needed;
  stip_traverse_structure_children(si,st);
  check_detector_defender_filter_inseration_state
      = save_detector_state;
  variation_writer_insertion_state = save_var_state;

  if (variation_writer_insertion_state==variation_writer_needed)
  {
    if (postkey_play_state==postkey_play_exclusively)
      pipe_append(slices[si].prev,
                  alloc_refuting_variation_writer_slice(length,min_length));
    pipe_append(slices[si].prev,
                alloc_variation_writer_slice(length,min_length));

    pipe_append(slices[si].prev,
                alloc_output_plaintext_tree_check_detector_attacker_filter_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_move(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (check_detector_defender_filter_inseration_state
      ==check_detector_defender_filter_needed)
    pipe_append(slices[si].prev,
                alloc_output_plaintext_tree_check_detector_defender_filter_slice(length,min_length));

  variation_writer_insertion_state = variation_writer_needed;
  stip_traverse_structure_children(si,st);
  variation_writer_insertion_state = variation_writer_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_reflex_defender_filter(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (check_detector_defender_filter_inseration_state
      ==check_detector_defender_filter_needed)
  {
    pipe_append(slices[si].prev,
                alloc_output_plaintext_tree_check_detector_defender_filter_slice(length,min_length));
    check_detector_defender_filter_inseration_state
        = check_detector_defender_filter_inserted;
    stip_traverse_structure_children(si,st);
    check_detector_defender_filter_inseration_state
        = check_detector_defender_filter_needed;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_threat_enforcer(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_zugzwang_writer_slice());

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

static void instrument_battle_play_solver(slice_index si,
                                          stip_structure_traversal *st)
{
  postkey_play_state_type const save_postkey_play_state = postkey_play_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  postkey_play_state = postkey_play_included;
  stip_traverse_structure_children(si,st);
  postkey_play_state = save_postkey_play_state;

  pipe_append(si,alloc_battle_play_solution_writer());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_postkeyplay_suppressor(slice_index si,
                                              stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_output_plaintext_tree_check_detector_defender_filter_slice(length,min_length));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_attack_root(slice_index si,
                                   stip_structure_traversal *st)
{
  check_detector_defender_filter_inseration_state_type const
      save_detector_state = check_detector_defender_filter_inseration_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  check_detector_defender_filter_inseration_state
      = check_detector_defender_filter_needed;
  stip_traverse_structure_children(si,st);
  check_detector_defender_filter_inseration_state
      = save_detector_state;

  pipe_append(si,alloc_end_of_solution_writer_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense_root(slice_index si,
                                    stip_structure_traversal *st)
{
  postkey_play_state_type const save_postkey_play_state = postkey_play_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  postkey_play_state = postkey_play_exclusively;
  stip_traverse_structure_children(si,st);
  postkey_play_state = save_postkey_play_state;

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

static void instrument_help_root(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors tree_slice_inserters[] =
{
  { STGoalReachedTester,              &instrument_goal_reached_tester     },
  { STLeaf,                           &instrument_leaf                    },
  { STReciprocal,                     &instrument_binary                  },
  { STQuodlibet,                      &instrument_binary                  },
  { STMoveInverterRootSolvableFilter, &instrument_move_inverter           },
  { STMoveInverterSolvableFilter,     &instrument_move_inverter           },
  { STAttackRoot,                     &instrument_attack_root             },
  { STDefenseRoot,                    &instrument_defense_root            },
  { STPostKeyPlaySuppressor,          &instrument_postkeyplay_suppressor, },
  { STContinuationSolver,             &instrument_continuation_solver     },
  { STBattlePlaySolver,               &instrument_battle_play_solver      },
  { STThreatEnforcer,                 &instrument_threat_enforcer         },
  { STRefutationsCollector,           &instrument_refutations_collector   },
  { STHelpRoot,                       &instrument_help_root               },
  { STSeriesRoot,                     &stip_structure_visitor_noop        },
  { STReflexAttackerFilter,           &instrument_attack_move             },
  { STReflexDefenderFilter,           &instrument_reflex_defender_filter, },
  { STSelfDefense,                    &instrument_self_defense            },
  { STAttackEnd,                      &instrument_attack_move             },
  { STDefenseEnd,                     &instrument_defense_move            }
};

enum
{
  nr_tree_slice_inserters = (sizeof tree_slice_inserters
                             / sizeof tree_slice_inserters[0])
};

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 */
void stip_insert_output_plaintext_tree_slices(void)
{
  stip_structure_traversal st;
  Goal goal = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&goal);
  stip_structure_traversal_override(&st,
                                    tree_slice_inserters,
                                    nr_tree_slice_inserters);
  stip_traverse_structure(root_slice,&st);

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
  {
    sprintf(GlobalStr,"..");
    StdString(GlobalStr);
  }

  output_plaintext_write_move(nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
