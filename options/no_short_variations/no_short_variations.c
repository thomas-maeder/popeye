#include "options/no_short_variations/no_short_variations.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

static void append_no_short_variations(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_no_short_variations_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const no_short_variations_filter_inserters[] =
{
  &stip_traverse_structure_children,    /* STProxy */
  &stip_traverse_structure_children,    /* STAttackMove */
  &stip_traverse_structure_children,    /* STDefenseMove */
  &stip_traverse_structure_children,    /* STHelpMove */
  &stip_traverse_structure_children,    /* STHelpFork */
  &stip_traverse_structure_children,    /* STSeriesMove */
  &stip_traverse_structure_children,    /* STSeriesFork */
  &stip_structure_visitor_noop,         /* STGoalReachedTester */
  &stip_structure_visitor_noop,         /* STLeaf */
  &stip_traverse_structure_children,    /* STReciprocal */
  &stip_traverse_structure_children,    /* STQuodlibet */
  &stip_traverse_structure_children,    /* STNot */
  &stip_traverse_structure_children,    /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,    /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,    /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,    /* STAttackRoot */
  &stip_traverse_structure_children,    /* STDefenseRoot */
  &stip_traverse_structure_children,    /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,    /* STContinuationSolver */
  &stip_traverse_structure_children,    /* STContinuationWriter */
  &stip_traverse_structure_children,    /* STBattlePlaySolver */
  &stip_traverse_structure_children,    /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,    /* STThreatSolver */
  &stip_traverse_structure_children,    /* STZugzwangWriter */
  &stip_traverse_structure_children,    /* STThreatEnforcer */
  &stip_traverse_structure_children,    /* STThreatCollector */
  &stip_traverse_structure_children,    /* STRefutationsCollector */
  &stip_traverse_structure_children,    /* STVariationWriter */
  &stip_traverse_structure_children,    /* STRefutingVariationWriter */
  &stip_traverse_structure_children,    /* STNoShortVariations */
  &stip_traverse_structure_children,    /* STAttackHashed */
  &stip_traverse_structure_children,    /* STHelpRoot */
  &stip_traverse_structure_children,    /* STHelpShortcut */
  &stip_traverse_structure_children,    /* STHelpHashed */
  &stip_traverse_structure_children,    /* STSeriesRoot */
  &stip_traverse_structure_children,    /* STSeriesShortcut */
  &stip_traverse_structure_children,    /* STParryFork */
  &stip_traverse_structure_children,    /* STSeriesHashed */
  &stip_traverse_structure_children,    /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,    /* STSelfCheckGuardSolvableFilter */
  &append_no_short_variations,          /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,    /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,    /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,    /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,    /* STDirectDefenderFilter */
  &stip_traverse_structure_children,    /* STReflexRootFilter */
  &stip_traverse_structure_children,    /* STReflexHelpFilter */
  &stip_traverse_structure_children,    /* STReflexSeriesFilter */
  &stip_traverse_structure_children,    /* STReflexAttackerFilter */
  &stip_traverse_structure_children,    /* STReflexDefenderFilter */
  &stip_traverse_structure_children,    /* STSelfDefense */
  &stip_traverse_structure_children,    /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,    /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,    /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,    /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,    /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,    /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,    /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,    /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,    /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,    /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,    /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,    /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,    /* STMaxFlightsquares */
  &stip_traverse_structure_children,    /* STDegenerateTree */
  &stip_traverse_structure_children,    /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,    /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,    /* STMaxThreatLength */
  &stip_traverse_structure_children,    /* STNo_short_variationsRootDefenderFilter */
  &stip_traverse_structure_children,    /* STNo_short_variationsDefenderFilter */
  &stip_traverse_structure_children,    /* STNoShortVariations */
  &stip_traverse_structure_children,    /* STNo_short_variationsSeriesFilter */
  &stip_traverse_structure_children,    /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,    /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,    /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,    /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,    /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,    /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,    /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,    /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,    /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,    /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,    /* STRefutationWriter */
  &stip_traverse_structure_children,    /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,    /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,    /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,    /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,    /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,    /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children     /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Instrument a stipulation with STNo_short_variationsSeriesFilter slices
 */
void stip_insert_no_short_variations_filters(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&no_short_variations_filter_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
