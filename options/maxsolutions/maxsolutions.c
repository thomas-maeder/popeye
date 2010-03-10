#include "optimisations/maxsolutions/maxsolutions.h"
#include "pypipe.h"
#include "optimisations/maxsolutions/root_solvable_filter.h"
#include "optimisations/maxsolutions/root_defender_filter.h"
#include "optimisations/maxsolutions/help_filter.h"
#include "optimisations/maxsolutions/series_filter.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* number of solutions found in the current phase */
static unsigned int nr_solutions_found_in_phase;

/* maximum number of allowed solutions found in the current phase */
static unsigned int max_nr_solutions_per_phase;

/* has the maximum number of allowed solutions been reached? */
static boolean allowed_nr_solutions_reached;

/* Reset the value of the maxsolutions option
 */
void reset_max_solutions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  max_nr_solutions_per_phase = UINT_MAX;
  allowed_nr_solutions_reached = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Read the value of the maxsolutions option
 * @return true iff the value could be successfully read
 */
boolean read_max_solutions(char const *token)
{
  boolean result;
  char *end;
  unsigned long const value = strtoul(token,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (*end==0 && value<UINT_MAX)
  {
    max_nr_solutions_per_phase = (unsigned int)value;
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Have we found the maxmimum allowed number of solutions since the
 * last invokation of reset_max_solutions()/read_max_solutions()?
 * @true iff we have found the maxmimum allowed number of solutions
 */
boolean max_solutions_reached(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",allowed_nr_solutions_reached);
  TraceFunctionResultEnd();
  return allowed_nr_solutions_reached;
}

/* Reset the number of found solutions
 */
void reset_nr_found_solutions_per_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_solutions_found_in_phase = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Increase the number of found solutions by 1
 */
void increase_nr_found_solutions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++nr_solutions_found_in_phase;

  if (max_nr_solutions_found_in_phase())
    allowed_nr_solutions_reached = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Have we found the maximum allowed number of solutions since the
 * last invokation of reset_nr_found_solutions()? 
 * @return true iff the allowed maximum number of solutions have been found
 */
boolean max_nr_solutions_found_in_phase(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = nr_solutions_found_in_phase>=max_nr_solutions_per_phase;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Insert a STMaxSolutionsHelpFilter slice after a STHelpMove slice
 */
static boolean insert_maxsolutions_help_filter(slice_index si,
                                               slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (slices[next].prev==si)
  {
    /* we are part of a loop
     */
    slice_index const filter = alloc_maxsolutions_help_filter();
    pipe_link(filter,next);
    pipe_link(si,filter);
  }
  else
  {
    /* we are attached to a loop
     */
    slice_index const next_pred = slices[next].prev;
    assert(slices[next_pred].type==STMaxSolutionsHelpFilter);

    /* a STMaxSolutionsHelpFilter slice has been inserted in the
     * loop before next; attach to it
     */
    pipe_set_successor(si,next_pred);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STMaxSolutionsSeriesFilter slice after a STSeriesMove slice
 */
static boolean insert_maxsolutions_series_filter(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (slices[next].prev==si)
  {
    /* we are part of a loop
     */
    slice_index const filter = alloc_maxsolutions_series_filter();
    pipe_link(filter,next);
    pipe_link(si,filter);
  }
  else
  {
    /* we are attached to a loop
     */
    slice_index const next_pred = slices[next].prev;
    assert(slices[next_pred].type==STMaxSolutionsSeriesFilter);

    /* a STMaxSolutionsSeriesFilter slice has been inserted in the
     * loop before next; attach to it
     */
    pipe_set_successor(si,next_pred);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STMaxSolutionsRootDefenderFilter slice after a STAttackRoot slice
 */
static boolean insert_maxsolutions_root_defender_filter(slice_index si,
                                                        slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    slice_index const filter = alloc_maxsolutions_root_defender_filter();
    pipe_link(filter,next);
    pipe_link(si,filter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const maxsolutions_filter_inserters[] =
{
  &slice_traverse_children,           /* STProxy */
  &slice_traverse_children,           /* STAttackMove */
  &slice_traverse_children,           /* STDefenseMove */
  &insert_maxsolutions_help_filter,   /* STHelpMove */
  &slice_traverse_children,           /* STHelpFork */
  &insert_maxsolutions_series_filter, /* STSeriesMove */
  &slice_traverse_children,           /* STSeriesFork */
  &slice_traverse_children,           /* STLeafDirect */
  &slice_traverse_children,           /* STLeafHelp */
  &slice_operation_noop,              /* STLeafForced */
  &slice_traverse_children,           /* STReciprocal */
  &slice_traverse_children,           /* STQuodlibet */
  &slice_traverse_children,           /* STNot */
  &slice_traverse_children,           /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,           /* STMoveInverterSolvableFilter */
  &slice_traverse_children,           /* STMoveInverterSeriesFilter */
  &insert_maxsolutions_root_defender_filter, /* STAttackRoot */
  &slice_traverse_children,           /* STBattlePlaySolutionWriter */
  &slice_traverse_children,           /* STPostKeyPlaySolutionWriter */
  &slice_traverse_children,           /* STContinuationWriter */
  &slice_traverse_children,           /* STTryWriter */
  &slice_traverse_children,           /* STThreatWriter */
  &slice_traverse_children,           /* STThreatEnforcer */
  &slice_traverse_children,           /* STRefutationsCollector */
  &slice_traverse_children,           /* STVariationWriter */
  &slice_traverse_children,           /* STRefutingVariationWriter */
  &slice_traverse_children,           /* STNoShortVariations */
  &slice_traverse_children,           /* STAttackHashed */
  &slice_traverse_children,           /* STHelpRoot */
  &slice_traverse_children,           /* STHelpShortcut */
  &slice_traverse_children,           /* STHelpHashed */
  &slice_traverse_children,           /* STSeriesRoot */
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
  &slice_traverse_children,           /* STDirectDefenseRootSolvableFilter */
  &slice_traverse_children,           /* STDirectDefense */
  &slice_traverse_children,           /* STReflexHelpFilter */
  &slice_traverse_children,           /* STReflexSeriesFilter */
  &slice_traverse_children,           /* STReflexRootSolvableFilter */
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

/* Insert a STMaxSolutionsRootSolvableFilter slice before some slice
 */
static boolean insert_root_solvable_filter(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const filter = alloc_maxsolutions_root_solvable_filter();
    pipe_link(slices[si].prev,filter);
    pipe_link(filter,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const maxsolutions_initialiser_inserters[] =
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

/* Instrument a stipulation with STMaxSolutions*Filter slices
 */
void stip_insert_maxsolutions_filters(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&maxsolutions_filter_inserters,0);
  traverse_slices(root_slice,&st);

  slice_traversal_init(&st,&maxsolutions_initialiser_inserters,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
