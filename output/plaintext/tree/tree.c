#include "output/plaintext/tree/tree.h"
#include "pydata.h"
#include "pypipe.h"
#include "output/plaintext/tree/end_of_phase_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_detector_attacker_filter.h"
#include "output/plaintext/tree/check_detector_defender_filter.h"
#include "output/plaintext/tree/continuation_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/battle_play_solution_writer.h"
#include "output/plaintext/tree/variation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
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

static void instrument_self_defense(slice_index si, stip_structure_traversal *st)
{
  variation_writer_insertion_state_type const
      save_state = variation_writer_insertion_state;
  slice_index const next = slices[si].u.branch_fork.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(next,st);
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (variation_writer_insertion_state==variation_writer_needed)
    pipe_append(slices[si].prev,
                alloc_variation_writer_slice(slack_length_battle,
                                             slack_length_battle));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_attack_move(slice_index si, stip_structure_traversal *st)
{
  variation_writer_insertion_state_type const
      save_state = variation_writer_insertion_state;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  variation_writer_insertion_state = variation_writer_none;
  pipe_traverse_next(si,st);
  variation_writer_insertion_state = save_state;

  if (variation_writer_insertion_state==variation_writer_needed)
  {
    if (postkey_play_state==postkey_play_exclusively)
      pipe_append(slices[si].prev,alloc_refuting_variation_writer_slice(length,
                                                                        min_length));
    pipe_append(slices[si].prev,alloc_variation_writer_slice(length,
                                                             min_length));
  }

  pipe_append(slices[si].prev,
              alloc_output_plaintext_tree_check_detector_attacker_filter_slice(length,min_length));

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

  pipe_append(slices[si].prev,
              alloc_output_plaintext_tree_check_detector_defender_filter_slice(length,min_length));

  variation_writer_insertion_state = variation_writer_needed;
  stip_traverse_structure_children(si,st);
  variation_writer_insertion_state = variation_writer_none;

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

static void instrument_attack_root(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
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

static stip_structure_visitor const tree_slice_inserters[] =
{
  &stip_traverse_structure_children,       /* STProxy */
  &instrument_attack_move,                 /* STAttackMove */
  &instrument_defense_move,                /* STDefenseMove */
  &stip_structure_visitor_noop,            /* STHelpMove */
  &stip_structure_visitor_noop,            /* STHelpFork */
  &stip_structure_visitor_noop,            /* STSeriesMove */
  &stip_structure_visitor_noop,            /* STSeriesFork */
  &instrument_goal_reached_tester,         /* STGoalReachedTester */
  &stip_structure_visitor_noop,            /* STLeaf */
  &instrument_binary,                      /* STReciprocal */
  &instrument_binary,                      /* STQuodlibet */
  &stip_traverse_structure_children,       /* STNot */
  &stip_traverse_structure_children,       /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,       /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,       /* STMoveInverterSeriesFilter */
  &instrument_attack_root,                 /* STAttackRoot */
  &instrument_defense_root,                /* STDefenseRoot */
  &instrument_postkeyplay_suppressor,      /* STPostKeyPlaySuppressor */
  &instrument_continuation_solver,         /* STContinuationSolver */
  &stip_traverse_structure_children,       /* STContinuationWriter */
  &instrument_battle_play_solver,          /* STBattlePlaySolver */
  &stip_traverse_structure_children,       /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,       /* STThreatSolver */
  &stip_traverse_structure_children,       /* STZugzwangWriter */
  &instrument_threat_enforcer,             /* STThreatEnforcer */
  &stip_traverse_structure_children,       /* STThreatCollector */
  &instrument_refutations_collector,       /* STRefutationsCollector */
  &stip_traverse_structure_children,       /* STVariationWriter */
  &stip_traverse_structure_children,       /* STRefutingVariationWriter */
  &stip_traverse_structure_children,       /* STNoShortVariations */
  &stip_traverse_structure_children,       /* STAttackHashed */
  &stip_structure_visitor_noop,            /* STHelpRoot */
  &stip_structure_visitor_noop,            /* STHelpShortcut */
  &stip_structure_visitor_noop,            /* STHelpHashed */
  &stip_structure_visitor_noop,            /* STSeriesRoot */
  &stip_structure_visitor_noop,            /* STSeriesShortcut */
  &stip_traverse_structure_children,       /* STParryFork */
  &stip_traverse_structure_children,       /* STSeriesHashed */
  &stip_traverse_structure_children,       /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,       /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STDirectDefenderFilter */
  &stip_traverse_structure_children,       /* STReflexRootFilter */
  &stip_traverse_structure_children,       /* STReflexHelpFilter */
  &stip_traverse_structure_children,       /* STReflexSeriesFilter */
  &instrument_attack_move,                 /* STReflexAttackerFilter */
  &stip_traverse_structure_children,       /* STReflexDefenderFilter */
  &instrument_self_defense,                /* STSelfDefense */
  &stip_traverse_structure_children,       /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,       /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,       /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,       /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,       /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,       /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,       /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,       /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,       /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,       /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,       /* STMaxFlightsquares */
  &stip_traverse_structure_children,       /* STDegenerateTree */
  &stip_traverse_structure_children,       /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,       /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,       /* STMaxThreatLength */
  &stip_traverse_structure_children,       /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,       /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,       /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,       /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,       /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,       /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,       /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,       /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,       /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,       /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,       /* STRefutationWriter */
  &stip_traverse_structure_children,       /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,       /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children        /* STOutputPlaintextLineLineWriter */
};

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 */
void stip_insert_output_plaintext_tree_slices(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&tree_slice_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
