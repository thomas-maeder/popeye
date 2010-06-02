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
  TraceFunctionParam("%s",token);
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
  TraceValue("->%u\n",nr_solutions_found_in_phase);

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
static void insert_maxsolutions_help_filter(slice_index si,
                                            stip_structure_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[next].prev==si)
    /* we are part of a loop
     */
    pipe_append(si,alloc_maxsolutions_help_filter());
  else
  {
    slice_index const next_pred = slices[next].prev;
    if (slices[next_pred].type==STMaxSolutionsHelpFilter)
      /* we are attached to a loop; a STMaxSolutionsHelpFilter slice
       * has been inserted in the loop before next; attach to it
       */
      pipe_set_successor(si,next_pred);
    else
      /* we are attached to something else; e.g. the help move is at
       * the beginning of set play in series self play
       */
      pipe_append(si,alloc_maxsolutions_help_filter());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STMaxSolutionsSeriesFilter slice after a STSeriesMove slice
 */
static void insert_maxsolutions_series_filter(slice_index si,
                                              stip_structure_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[next].prev==si)
    /* we are part of a loop
     */
    pipe_append(si,alloc_maxsolutions_series_filter());
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
  TraceFunctionResultEnd();
}

/* Insert a STMaxSolutionsRootDefenderFilter slice after a STAttackRoot slice
 */
static
void insert_maxsolutions_root_defender_filter(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_maxsolutions_root_defender_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const maxsolutions_filter_inserters[] =
{
  &stip_traverse_structure_children,         /* STProxy */
  &stip_traverse_structure_children,         /* STAttackMove */
  &stip_traverse_structure_children,         /* STDefenseMove */
  &insert_maxsolutions_help_filter,          /* STHelpMove */
  &stip_traverse_structure_children,         /* STHelpFork */
  &insert_maxsolutions_series_filter,        /* STSeriesMove */
  &stip_traverse_structure_children,         /* STSeriesFork */
  &stip_traverse_structure_children,         /* STLeafDirect */
  &stip_traverse_structure_children,         /* STLeafHelp */
  &stip_structure_visitor_noop,              /* STLeafForced */
  &stip_traverse_structure_children,         /* STReciprocal */
  &stip_traverse_structure_children,         /* STQuodlibet */
  &stip_traverse_structure_children,         /* STNot */
  &stip_traverse_structure_children,         /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,         /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,         /* STMoveInverterSeriesFilter */
  &insert_maxsolutions_root_defender_filter, /* STAttackRoot */
  &stip_traverse_structure_children,         /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,         /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,         /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,         /* STContinuationWriter */
  &stip_traverse_structure_children,         /* STRefutationsWriter */
  &stip_traverse_structure_children,         /* STThreatWriter */
  &stip_traverse_structure_children,         /* STThreatEnforcer */
  &stip_traverse_structure_children,         /* STThreatCollector */
  &stip_traverse_structure_children,         /* STRefutationsCollector */
  &stip_traverse_structure_children,         /* STVariationWriter */
  &stip_traverse_structure_children,         /* STRefutingVariationWriter */
  &stip_traverse_structure_children,         /* STNoShortVariations */
  &stip_traverse_structure_children,         /* STAttackHashed */
  &stip_traverse_structure_children,         /* STHelpRoot */
  &stip_traverse_structure_children,         /* STHelpShortcut */
  &stip_traverse_structure_children,         /* STHelpHashed */
  &stip_traverse_structure_children,         /* STSeriesRoot */
  &stip_traverse_structure_children,         /* STSeriesShortcut */
  &stip_traverse_structure_children,         /* STParryFork */
  &stip_traverse_structure_children,         /* STSeriesHashed */
  &stip_traverse_structure_children,         /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,         /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,         /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,         /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,         /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,         /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,         /* STDirectDefenderFilter */
  &stip_traverse_structure_children,         /* STReflexHelpFilter */
  &stip_traverse_structure_children,         /* STReflexSeriesFilter */
  &stip_traverse_structure_children,         /* STReflexAttackerFilter */
  &stip_traverse_structure_children,         /* STReflexDefenderFilter */
  &stip_traverse_structure_children,         /* STSelfDefense */
  &stip_traverse_structure_children,         /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,         /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,         /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,         /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,         /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,         /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,         /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,         /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children,         /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,         /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,         /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,         /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,         /* STMaxFlightsquares */
  &stip_traverse_structure_children,         /* STDegenerateTree */
  &stip_traverse_structure_children,         /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,         /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,         /* STMaxThreatLength */
  &stip_traverse_structure_children,         /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,         /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,         /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,         /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,         /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,         /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,         /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,         /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,         /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,         /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children          /* STStopOnShortSolutionsSeriesFilter */
};

/* Insert a STMaxSolutionsRootSolvableFilter slice before some slice
 */
static void insert_root_solvable_filter(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,alloc_maxsolutions_root_solvable_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const maxsolutions_initialiser_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &insert_root_solvable_filter,      /* STLeafDirect */
  &insert_root_solvable_filter,      /* STLeafHelp */
  &stip_traverse_structure_children, /* STLeafForced */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &insert_root_solvable_filter,      /* STAttackRoot */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STRefutationsWriter */
  &stip_traverse_structure_children, /* STThreatWriter */
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
  &stip_traverse_structure_children, /* STKeepMatingGuardRootDefenderFilter */
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
  &stip_traverse_structure_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument a stipulation with STMaxSolutions*Filter slices
 */
void stip_insert_maxsolutions_filters(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&maxsolutions_filter_inserters,0);
  stip_traverse_structure(root_slice,&st);

  stip_structure_traversal_init(&st,&maxsolutions_initialiser_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
