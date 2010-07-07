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

/* Insert a STStopOnShortSolutionsHelpFilter slice
 */
static void insert_stoponshortsolutions_help_root(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsHelpFilter)
  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(si,alloc_stoponshortsolutions_help_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STHelpFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsHelpFilter slice
 */
static void insert_stoponshortsolutions_help_move(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsHelpFilter)
  {
    stip_length_type const length = slices[si].u.branch.length-1;
    stip_length_type const min_length = slices[si].u.branch.min_length-1;
    pipe_append(si,alloc_stoponshortsolutions_help_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STHelpFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsSeriesFilter slice
 */
static
void insert_stoponshortsolutions_series_root(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsSeriesFilter)
  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(si,alloc_stoponshortsolutions_series_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STSeriesFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsSeriesFilter slice
 */
static
void insert_stoponshortsolutions_series_move(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type!=STStopOnShortSolutionsSeriesFilter)
  {
    stip_length_type const length = slices[si].u.branch.length-1;
    stip_length_type const min_length = slices[si].u.branch.min_length-1;
    pipe_append(si,alloc_stoponshortsolutions_series_filter(length,min_length));
  }

  {
    slice_index const fork = branch_find_slice(STSeriesFork,si);
    if (fork!=no_slice)
      insert_filters(slices[fork].u.branch_fork.towards_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const stoponshortsolutions_filter_inserters[] =
{
  &stip_traverse_structure_children,        /* STProxy */
  &stip_traverse_structure_children,        /* STAttackMove */
  &stip_traverse_structure_children,        /* STDefenseMove */
  &insert_stoponshortsolutions_help_move,   /* STHelpMove */
  &stip_traverse_structure_children,        /* STHelpFork */
  &insert_stoponshortsolutions_series_move, /* STSeriesMove */
  &stip_traverse_structure_children,        /* STSeriesFork */
  &stip_structure_visitor_noop,             /* STGoalReachedTester */
  &stip_traverse_structure_children,        /* STReciprocal */
  &stip_traverse_structure_children,        /* STQuodlibet */
  &stip_traverse_structure_children,        /* STNot */
  &stip_traverse_structure_children,        /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,        /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,        /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,        /* STAttackRoot */
  &stip_traverse_structure_children,        /* STDefenseRoot */
  &stip_traverse_structure_children,        /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,        /* STContinuationSolver */
  &stip_traverse_structure_children,        /* STContinuationWriter */
  &stip_traverse_structure_children,        /* STBattlePlaySolver */
  &stip_traverse_structure_children,        /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,        /* STThreatSolver */
  &stip_traverse_structure_children,        /* STZugzwangWriter */
  &stip_traverse_structure_children,        /* STThreatEnforcer */
  &stip_traverse_structure_children,        /* STThreatCollector */
  &stip_traverse_structure_children,        /* STRefutationsCollector */
  &stip_traverse_structure_children,        /* STVariationWriter */
  &stip_traverse_structure_children,        /* STRefutingVariationWriter */
  &stip_traverse_structure_children,        /* STNoShortVariations */
  &stip_traverse_structure_children,        /* STAttackHashed */
  &insert_stoponshortsolutions_help_root,   /* STHelpRoot */
  &stip_traverse_structure_children,        /* STHelpShortcut */
  &stip_traverse_structure_children,        /* STHelpHashed */
  &insert_stoponshortsolutions_series_root, /* STSeriesRoot */
  &stip_traverse_structure_children,        /* STSeriesShortcut */
  &stip_traverse_structure_children,        /* STParryFork */
  &stip_traverse_structure_children,        /* STSeriesHashed */
  &stip_traverse_structure_children,        /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,        /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,        /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,        /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,        /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,        /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STDirectDefenderFilter */
  &stip_traverse_structure_children,        /* STReflexRootFilter */
  &stip_traverse_structure_children,        /* STReflexHelpFilter */
  &stip_traverse_structure_children,        /* STReflexSeriesFilter */
  &stip_traverse_structure_children,        /* STReflexAttackerFilter */
  &stip_traverse_structure_children,        /* STReflexDefenderFilter */
  &stip_traverse_structure_children,        /* STSelfDefense */
  &stip_traverse_structure_children,        /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,        /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,        /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,        /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,        /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,        /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STMaxFlightsquares */
  &stip_traverse_structure_children,        /* STDegenerateTree */
  &stip_traverse_structure_children,        /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,        /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,        /* STMaxThreatLength */
  &stip_traverse_structure_children,        /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,        /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,        /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,        /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsSeriesFilter */
  &stip_structure_visitor_noop,             /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_structure_visitor_noop,             /* STStopOnShortSolutionsHelpFilter */
  &stip_structure_visitor_noop,             /* STStopOnShortSolutionsSeriesFilter */
  &stip_structure_visitor_noop,             /* STEndOfPhaseWriter */
  &stip_structure_visitor_noop,             /* STEndOfSolutionWriter */
  &stip_structure_visitor_noop,             /* STRefutationWriter */
  &stip_structure_visitor_noop,             /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_structure_visitor_noop              /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
};

/* Insert STStopOnShortSolutions*Filter starting at a slice
 * @param si identifies slice where to start
 * @note this is an indirectly recursive function
 */
static void insert_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&stoponshortsolutions_filter_inserters,0);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STStopOnShortSolutionsRootSolvableFilter slice before some slice
 */
static void insert_root_solvable_filter(slice_index si,
                                        stip_structure_traversal *st)
{
  boolean * const inserted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_stoponshortsolutions_root_solvable_filter());
  insert_filters(si);

  *inserted = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
stip_structure_visitor const stoponshortsolutions_initialiser_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
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
  &insert_root_solvable_filter,      /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &insert_root_solvable_filter,      /* STSeriesRoot */
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
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentHelpFilter */
  &stip_traverse_structure_children, /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &stip_traverse_structure_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &stip_traverse_structure_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STEndOfPhaseWriter */
  &stip_traverse_structure_children, /* STEndOfSolutionWriter */
  &stip_traverse_structure_children, /* STRefutationWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children  /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
};

/* Instrument a stipulation with STStopOnShortSolutions*Filter slices
 * @return true iff the option stoponshort applies
 */
boolean stip_insert_stoponshortsolutions_filters(void)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,
                                &stoponshortsolutions_initialiser_inserters,
                                &result);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
