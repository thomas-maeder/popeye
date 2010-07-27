#include "output/plaintext/line/line.h"
#include "pypipe.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "trace.h"

#include <assert.h>

static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  Goal const * const goal = st->param;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (goal->type!=no_goal)
    pipe_append(slices[si].prev,alloc_line_writer_slice(*goal));

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
  pipe_append(si,alloc_output_plaintext_line_move_inversion_counter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_root(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(slices[si].prev,alloc_end_of_phase_writer_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_series_fork(slice_index si,
                                   stip_structure_traversal *st)
{
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index next = slices[to_goal].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[to_goal].type==STProxy);
  while (slices[next].type==STProxy
         || slices[next].type==STSeriesHashed)
    next = slices[next].u.pipe.next;

  if (slices[next].type==STSeriesMove
      || slices[next].type==STSeriesFork)
  {
    slice_index const marker
        = alloc_output_plaintext_line_end_of_intro_series_marker_slice();
    pipe_set_successor(marker,to_goal);
    slices[si].u.branch_fork.towards_goal = marker;
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const line_slice_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STAttackMoveToGoal */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STDefenseMoveAgainstGoal */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpMoveToGoal */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesMoveToGoal */
  &instrument_series_fork,           /* STSeriesFork */
  &instrument_goal_reached_tester,   /* STGoalReachedTester */
  &instrument_leaf,                  /* STLeaf */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &instrument_move_inverter,         /* STMoveInverterRootSolvableFilter */
  &instrument_move_inverter,         /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children, /* STAttackRoot */
  &stip_traverse_structure_children, /* STDefenseRoot */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationSolver */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STBattlePlaySolver */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STThreatSolver */
  &stip_traverse_structure_children, /* STZugzwangWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STThreatCollector */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &stip_traverse_structure_children, /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &instrument_root,                  /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &instrument_root,                  /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexRootFilter */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &stip_traverse_structure_children, /* STReflexDefenderFilter */
  &stip_traverse_structure_children, /* STSelfDefense */
  &stip_traverse_structure_children, /* STAttackEnd */
  &stip_traverse_structure_children, /* STAttackFork */
  &stip_traverse_structure_children, /* STDefenseEnd */
  &stip_traverse_structure_children, /* STDefenseFork */
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentHelpFilter */
  &stip_traverse_structure_children, /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateLine */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &stip_traverse_structure_children, /* STMaxNrNonChecks */
  &stip_traverse_structure_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &stip_traverse_structure_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STEndOfPhaseWriter */
  &stip_traverse_structure_children, /* STEndOfSolutionWriter */
  &stip_traverse_structure_children, /* STRefutationWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineCheckDetectorAttackerFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineCheckDetectorDefenderFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Instrument the stipulation structure with slices that implement
 * plaintext line mode output.
 */
void stip_insert_output_plaintext_line_slices(void)
{
  stip_structure_traversal st;
  Goal state = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&line_slice_inserters,&state);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
