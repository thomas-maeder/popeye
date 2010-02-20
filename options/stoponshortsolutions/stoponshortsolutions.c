#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/stoponshortsolutions/root_solvable_filter.h"
#include "optimisations/stoponshortsolutions/help_filter.h"
#include "optimisations/stoponshortsolutions/series_filter.h"
#include "pystip.h"
#include "pypipe.h"
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

/* Insert a STStopOnShortSolutionsHelpFilter slice after the toplevel
 * STBranchHelp slice
 */
static boolean insert_stoponshortsolutions_help_filter(slice_index si,
                                                         slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  stip_length_type const length = slices[si].u.branch.length;
  slice_index const filter = alloc_stoponshortsolutions_help_filter(length);
  pipe_link(filter,next);
  pipe_link(si,filter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STStopOnShortSolutionsSeriesFilter slice after the toplevel
 * STBranchSeries slice
 */
static boolean insert_stoponshortsolutions_series_filter(slice_index si,
                                                         slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  stip_length_type const length = slices[si].u.branch.length;
  slice_index const filter = alloc_stoponshortsolutions_series_filter(length);
  pipe_link(filter,next);
  pipe_link(si,filter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const stoponshortsolutions_filter_inserters[] =
{
  &slice_traverse_children,           /* STProxy */
  &slice_traverse_children,           /* STBranchDirect */
  &slice_traverse_children,           /* STBranchDirectDefender */
  &slice_traverse_children, /* STBranchHelp */
  &slice_traverse_children,           /* STHelpFork */
  &slice_traverse_children, /* STBranchSeries */
  &slice_traverse_children,           /* STSeriesFork */
  &slice_operation_noop,              /* STLeafDirect */
  &slice_operation_noop,              /* STLeafHelp */
  &slice_operation_noop,              /* STLeafForced */
  &slice_traverse_children,           /* STReciprocal */
  &slice_traverse_children,           /* STQuodlibet */
  &slice_traverse_children,           /* STNot */
  &slice_traverse_children,           /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,           /* STMoveInverterSolvableFilter */
  &slice_traverse_children,           /* STMoveInverterSeriesFilter */
  &slice_traverse_children,           /* STDirectRoot */
  &slice_traverse_children,           /* STDirectDefenderRoot */
  &slice_traverse_children,           /* STDirectHashed */
  &insert_stoponshortsolutions_help_filter,           /* STHelpRoot */
  &slice_traverse_children,           /* STHelpShortcut */
  &slice_traverse_children,           /* STHelpHashed */
  &insert_stoponshortsolutions_series_filter,           /* STSeriesRoot */
  &slice_traverse_children,           /* STSeriesShortcut */
  &slice_traverse_children,           /* STParryFork */
  &slice_traverse_children,           /* STSeriesHashed */
  &slice_traverse_children,           /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,           /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,           /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,           /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,           /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,           /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,           /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,           /* STDirectDefense */
  &slice_traverse_children,           /* STReflexHelpFilter */
  &slice_traverse_children,           /* STReflexSeriesFilter */
  &slice_traverse_children,           /* STReflexAttackerFilter */
  &slice_traverse_children,           /* STReflexDefenderFilter */
  &slice_traverse_children,           /* STSelfAttack */
  &slice_traverse_children,           /* STSelfDefense */
  &slice_traverse_children,           /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,           /* STRestartGuardHelpFilter */
  &slice_traverse_children,           /* STRestartGuardSeriesFilter */
  &slice_traverse_children,           /* STIntelligentHelpFilter */
  &slice_traverse_children,           /* STIntelligentSeriesFilter */
  &slice_traverse_children,           /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,           /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,           /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,           /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,           /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,           /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,           /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,           /* STMaxFlightsquares */
  &slice_traverse_children,           /* STDegenerateTree */
  &slice_traverse_children,           /* STMaxNrNonTrivial */
  &slice_traverse_children,           /* STMaxThreatLength */
  &slice_traverse_children,           /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,           /* STMaxTimeDefenderFilter */
  &slice_traverse_children,           /* STMaxTimeHelpFilter */
  &slice_traverse_children,           /* STMaxTimeSeriesFilter */
  &slice_traverse_children,           /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,           /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,           /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,           /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,           /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,           /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children            /* STStopOnShortSolutionsSeriesFilter */
};

/* Insert a STStopOnShortSolutionsRootSolvableFilter slice before some slice
 */
static boolean insert_root_solvable_filter(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const filter = alloc_stoponshortsolutions_root_solvable_filter();
    pipe_link(slices[si].prev,filter);
    pipe_link(filter,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const stoponshortsolutions_initialiser_inserters[] =
{
  &slice_traverse_children,     /* STProxy */
  &slice_traverse_children,     /* STBranchDirect */
  &slice_traverse_children,     /* STBranchDirectDefender */
  &slice_traverse_children,     /* STBranchHelp */
  &slice_traverse_children,     /* STHelpFork */
  &slice_traverse_children,     /* STBranchSeries */
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
  &insert_root_solvable_filter, /* STDirectRoot */
  &slice_traverse_children,     /* STDirectDefenderRoot */
  &slice_traverse_children,     /* STDirectHashed */
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
  &slice_traverse_children,     /* STDirectDefense */
  &slice_traverse_children,     /* STReflexHelpFilter */
  &slice_traverse_children,     /* STReflexSeriesFilter */
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

  slice_traversal_init(&st,&stoponshortsolutions_filter_inserters,0);
  traverse_slices(root_slice,&st);

  slice_traversal_init(&st,&stoponshortsolutions_initialiser_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
