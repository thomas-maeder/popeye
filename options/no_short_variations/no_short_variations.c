#include "options/no_short_variations/no_short_variations.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

static boolean insert_no_short_variations(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const filter = alloc_no_short_variations_slice(length,
                                                               min_length);
    pipe_link(prev,filter);
    pipe_link(filter,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const no_short_variations_filter_inserters[] =
{
  &slice_traverse_children,    /* STProxy */
  &slice_traverse_children,    /* STAttackMove */
  &slice_traverse_children,    /* STDefenseMove */
  &slice_traverse_children,    /* STHelpMove */
  &slice_traverse_children,    /* STHelpFork */
  &slice_traverse_children,    /* STSeriesMove */
  &slice_traverse_children,    /* STSeriesFork */
  &slice_operation_noop,       /* STLeafDirect */
  &slice_operation_noop,       /* STLeafHelp */
  &slice_operation_noop,       /* STLeafForced */
  &slice_traverse_children,    /* STReciprocal */
  &slice_traverse_children,    /* STQuodlibet */
  &slice_traverse_children,    /* STNot */
  &slice_traverse_children,    /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,    /* STMoveInverterSolvableFilter */
  &slice_traverse_children,    /* STMoveInverterSeriesFilter */
  &slice_traverse_children,    /* STAttackRoot */
  &slice_traverse_children,    /* STBattlePlaySolutionWriter */
  &slice_traverse_children,    /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,    /* STContinuationWriter */
  &slice_traverse_children,    /* STTryWriter */
  &slice_traverse_children,    /* STThreatWriter */
  &slice_traverse_children,    /* STDefenseRoot */
  &slice_traverse_children,    /* STThreatEnforcer */
  &slice_traverse_children,    /* STRefutationsCollector */
  &insert_no_short_variations, /* STVariationWriter */
  &slice_traverse_children,    /* STRefutingVariationWriter */
  &slice_traverse_children,    /* STNoShortVariations */
  &slice_traverse_children,    /* STAttackHashed */
  &slice_traverse_children,    /* STHelpRoot */
  &slice_traverse_children,    /* STHelpShortcut */
  &slice_traverse_children,    /* STHelpHashed */
  &slice_traverse_children,    /* STSeriesRoot */
  &slice_traverse_children,    /* STSeriesShortcut */
  &slice_traverse_children,    /* STParryFork */
  &slice_traverse_children,    /* STSeriesHashed */
  &slice_traverse_children,    /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,    /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,    /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,    /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,    /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,    /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,    /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,    /* STDirectDefense */
  &slice_traverse_children,    /* STReflexHelpFilter */
  &slice_traverse_children,    /* STReflexSeriesFilter */
  &slice_traverse_children,    /* STReflexAttackerFilter */
  &slice_traverse_children,    /* STReflexDefenderFilter */
  &slice_traverse_children,    /* STSelfAttack */
  &slice_traverse_children,    /* STSelfDefense */
  &slice_traverse_children,    /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,    /* STRestartGuardHelpFilter */
  &slice_traverse_children,    /* STRestartGuardSeriesFilter */
  &slice_traverse_children,    /* STIntelligentHelpFilter */
  &slice_traverse_children,    /* STIntelligentSeriesFilter */
  &slice_traverse_children,    /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,    /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,    /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,    /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,    /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,    /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,    /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,    /* STMaxFlightsquares */
  &slice_traverse_children,    /* STDegenerateTree */
  &slice_traverse_children,    /* STMaxNrNonTrivial */
  &slice_traverse_children,    /* STMaxThreatLength */
  &slice_traverse_children,    /* STNo_short_variationsRootDefenderFilter */
  &slice_traverse_children,    /* STNo_short_variationsDefenderFilter */
  &slice_traverse_children,    /* STNoShortVariations */
  &slice_traverse_children,    /* STNo_short_variationsSeriesFilter */
  &slice_traverse_children,    /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,    /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,    /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,    /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,    /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,    /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children     /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument a stipulation with STNo_short_variationsSeriesFilter slices
 */
void stip_insert_no_short_variations_filters(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&no_short_variations_filter_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
