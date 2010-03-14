#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/stoponshortsolutions/root_solvable_filter.h"
#include "optimisations/stoponshortsolutions/help_filter.h"
#include "optimisations/stoponshortsolutions/series_filter.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

static boolean short_solution_found_in_problem;
static boolean short_solution_found_in_phase;

/* Inform the stoponshortsolutions module that a short solution has
 * been found
 */
void short_solution_found(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  short_solution_found_in_problem = true;
  short_solution_found_in_phase = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the internal state to "no short solution found"
 */
void reset_short_solution_found_in_problem(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  short_solution_found_in_problem = false;
  short_solution_found_in_phase = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Has a short solution been found in the current problem?
 */
boolean has_short_solution_been_found_in_problem(void)
{
  boolean const result = short_solution_found_in_problem;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Reset the internal state to "no short solution found" in the
 * current phase
 */
void reset_short_solution_found_in_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  short_solution_found_in_phase = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the internal state to "no short solution found" in the
 * current phase
 */
boolean has_short_solution_been_found_in_phase(void)
{
  boolean const result = short_solution_found_in_phase;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert STStopOnShortSolutions*Filter starting at a slice
 * @param si identifies slice where to start
 * @note this is an indirectly recursive function
 */
static void insert_filters(slice_index si);

/* Insert a STStopOnShortSolutionsHelpFilter slice after the toplevel
 * STHelpMove slice
 */
static void insert_stoponshortsolutions_help_filter(slice_index si,
                                                    slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(slices[si].prev,
                alloc_stoponshortsolutions_help_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STHelpFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsSeriesFilter slice after the toplevel
 * STSeriesMove slice
 */
static void insert_stoponshortsolutions_series_filter(slice_index si,
                                                      slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(slices[si].prev,
                alloc_stoponshortsolutions_series_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STSeriesFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const stoponshortsolutions_filter_inserters[] =
{
  &slice_traverse_children,                   /* STProxy */
  &slice_traverse_children,                   /* STAttackMove */
  &slice_traverse_children,                   /* STDefenseMove */
  &insert_stoponshortsolutions_help_filter,   /* STHelpMove */
  &insert_stoponshortsolutions_help_filter,   /* STHelpFork */
  &insert_stoponshortsolutions_series_filter, /* STSeriesMove */
  &insert_stoponshortsolutions_series_filter, /* STSeriesFork */
  &slice_operation_noop,                      /* STLeafDirect */
  &slice_operation_noop,                      /* STLeafHelp */
  &slice_operation_noop,                      /* STLeafForced */
  &slice_traverse_children,                   /* STReciprocal */
  &slice_traverse_children,                   /* STQuodlibet */
  &slice_traverse_children,                   /* STNot */
  &slice_traverse_children,                   /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,                   /* STMoveInverterSolvableFilter */
  &insert_stoponshortsolutions_series_filter, /* STMoveInverterSeriesFilter */
  &slice_traverse_children,                   /* STAttackRoot */
  &slice_traverse_children,                   /* STBattlePlaySolutionWriter */
  &slice_traverse_children,                   /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,                   /* STContinuationWriter */
  &slice_traverse_children,                   /* STTryWriter */
  &slice_traverse_children,                   /* STThreatWriter */
  &slice_traverse_children,                   /* STThreatEnforcer */
  &slice_traverse_children,                   /* STRefutationsCollector */
  &slice_traverse_children,                   /* STVariationWriter */
  &slice_traverse_children,                   /* STRefutingVariationWriter */
  &slice_traverse_children,                   /* STNoShortVariations */
  &slice_traverse_children,                   /* STAttackHashed */
  &slice_traverse_children,                   /* STHelpRoot */
  &insert_stoponshortsolutions_help_filter,   /* STHelpShortcut */
  &insert_stoponshortsolutions_help_filter,   /* STHelpHashed */
  &slice_traverse_children,                   /* STSeriesRoot */
  &insert_stoponshortsolutions_series_filter, /* STSeriesShortcut */
  &slice_traverse_children,                   /* STParryFork */
  &insert_stoponshortsolutions_series_filter, /* STSeriesHashed */
  &slice_traverse_children,                   /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,                   /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,                   /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,                   /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,                   /* STSelfCheckGuardDefenderFilter */
  &insert_stoponshortsolutions_help_filter,   /* STSelfCheckGuardHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,                   /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children,                   /* STDirectDefense */
  &insert_stoponshortsolutions_help_filter,   /* STReflexHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STReflexSeriesFilter */
  &slice_traverse_children,                   /* STReflexRootSolvableFilter */
  &slice_traverse_children,                   /* STReflexAttackerFilter */
  &slice_traverse_children,                   /* STReflexDefenderFilter */
  &slice_traverse_children,                   /* STSelfAttack */
  &slice_traverse_children,                   /* STSelfDefense */
  &slice_traverse_children,                   /* STRestartGuardRootDefenderFilter */
  &insert_stoponshortsolutions_help_filter,   /* STRestartGuardHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STRestartGuardSeriesFilter */
  &insert_stoponshortsolutions_help_filter,   /* STIntelligentHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STIntelligentSeriesFilter */
  &insert_stoponshortsolutions_help_filter,   /* STGoalReachableGuardHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,                   /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,                   /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,                   /* STKeepMatingGuardDefenderFilter */
  &insert_stoponshortsolutions_help_filter,   /* STKeepMatingGuardHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,                   /* STMaxFlightsquares */
  &slice_traverse_children,                   /* STDegenerateTree */
  &slice_traverse_children,                   /* STMaxNrNonTrivial */
  &slice_traverse_children,                   /* STMaxThreatLength */
  &slice_traverse_children,                   /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,                   /* STMaxTimeDefenderFilter */
  &insert_stoponshortsolutions_help_filter,   /* STMaxTimeHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STMaxTimeSeriesFilter */
  &slice_traverse_children,                   /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,                   /* STMaxSolutionsRootDefenderFilter */
  &insert_stoponshortsolutions_help_filter,   /* STMaxSolutionsHelpFilter */
  &insert_stoponshortsolutions_series_filter, /* STMaxSolutionsSeriesFilter */
  &slice_operation_noop,                      /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_operation_noop,                      /* STStopOnShortSolutionsHelpFilter */
  &slice_operation_noop                       /* STStopOnShortSolutionsSeriesFilter */
};

/* Insert STStopOnShortSolutions*Filter starting at a slice
 * @param si identifies slice where to start
 * @note this is an indirectly recursive function
 */
static void insert_filters(slice_index si)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&stoponshortsolutions_filter_inserters,0);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsRootSolvableFilter slice before some slice
 */
static void insert_root_solvable_filter(slice_index si, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_stoponshortsolutions_root_solvable_filter());
  insert_filters(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const stoponshortsolutions_initialiser_inserters[] =
{
  &slice_traverse_children,     /* STProxy */
  &slice_traverse_children,     /* STAttackMove */
  &slice_traverse_children,     /* STDefenseMove */
  &slice_traverse_children,     /* STHelpMove */
  &slice_traverse_children,     /* STHelpFork */
  &slice_traverse_children,     /* STSeriesMove */
  &slice_traverse_children,     /* STSeriesFork */
  &insert_root_solvable_filter, /* STLeafDirect */
  &insert_root_solvable_filter, /* STLeafHelp */
  &slice_traverse_children,     /* STLeafForced */
  &slice_traverse_children,     /* STReciprocal */
  &slice_traverse_children,     /* STQuodlibet */
  &slice_traverse_children,     /* STNot */
  &slice_traverse_children,     /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,     /* STMoveInverterSolvableFilter */
  &slice_traverse_children,     /* STMoveInverterSeriesFilter */
  &insert_root_solvable_filter, /* STAttackRoot */
  &slice_traverse_children,     /* STBattlePlaySolutionWriter */
  &slice_traverse_children,     /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,     /* STContinuationWriter */
  &slice_traverse_children,     /* STTryWriter */
  &slice_traverse_children,     /* STThreatWriter */
  &slice_traverse_children,     /* STThreatEnforcer */
  &slice_traverse_children,     /* STRefutationsCollector */
  &slice_traverse_children,     /* STVariationWriter */
  &slice_traverse_children,     /* STRefutingVariationWriter */
  &slice_traverse_children,     /* STNoShortVariations */
  &slice_traverse_children,     /* STAttackHashed */
  &insert_root_solvable_filter, /* STHelpRoot */
  &slice_traverse_children,     /* STHelpShortcut */
  &slice_traverse_children,     /* STHelpHashed */
  &insert_root_solvable_filter, /* STSeriesRoot */
  &slice_traverse_children,     /* STSeriesShortcut */
  &slice_traverse_children,     /* STParryFork */
  &slice_traverse_children,     /* STSeriesHashed */
  &slice_traverse_children,     /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,     /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,     /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,     /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,     /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,     /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,     /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,     /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children,     /* STDirectDefense */
  &slice_traverse_children,     /* STReflexHelpFilter */
  &slice_traverse_children,     /* STReflexSeriesFilter */
  &slice_traverse_children,     /* STReflexRootSolvableFilter */
  &slice_traverse_children,     /* STReflexAttackerFilter */
  &slice_traverse_children,     /* STReflexDefenderFilter */
  &slice_traverse_children,     /* STSelfAttack */
  &slice_traverse_children,     /* STSelfDefense */
  &slice_traverse_children,     /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,     /* STRestartGuardHelpFilter */
  &slice_traverse_children,     /* STRestartGuardSeriesFilter */
  &slice_traverse_children,     /* STIntelligentHelpFilter */
  &slice_traverse_children,     /* STIntelligentSeriesFilter */
  &slice_traverse_children,     /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,     /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,     /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,     /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,     /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,     /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,     /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,     /* STMaxFlightsquares */
  &slice_traverse_children,     /* STDegenerateTree */
  &slice_traverse_children,     /* STMaxNrNonTrivial */
  &slice_traverse_children,     /* STMaxThreatLength */
  &slice_traverse_children,     /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,     /* STMaxTimeDefenderFilter */
  &slice_traverse_children,     /* STMaxTimeHelpFilter */
  &slice_traverse_children,     /* STMaxTimeSeriesFilter */
  &slice_traverse_children,     /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,     /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,     /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,     /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,     /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,     /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children      /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument a stipulation with STStopOnShortSolutions*Filter slices
 */
void stip_insert_stoponshortsolutions_filters(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&stoponshortsolutions_initialiser_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
