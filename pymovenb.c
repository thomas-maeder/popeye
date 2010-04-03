#include "pymovenb.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* number of current move at root level
 */
static unsigned int MoveNbr;

/* number of first move at root level to be considered
 */
static unsigned int RestartNbr;

/* Reset the restart number setting.
 */
void reset_restart_number(void)
{
  RestartNbr = 0;
  MoveNbr = 1;
}

unsigned int get_restart_number(void)
{
  return RestartNbr;
}

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
boolean read_restart_number(char const *optionValue)
{
  boolean result = false;

  char *end;
  unsigned long const restartNbrRequested = strtoul(optionValue,&end,10);
  if (optionValue!=end && restartNbrRequested<=UINT_MAX)
  {
    RestartNbr = (unsigned int)restartNbrRequested;
    result = true;
  }

  return result;
}

/* Increase the current move number; write the previous move number
 * provided it is above the number where the user asked us to restart
 * solving.
 */
static void IncrementMoveNbr(void)
{
  if (MoveNbr>=RestartNbr)
  {
    sprintf(GlobalStr,"\n%3u  (", MoveNbr);
    StdString(GlobalStr);
    ecritcoup(nbply,no_goal);

    if (!flag_regression)
    {
      StdString("   ");
      PrintTime();
    }

    StdString(")");
  }

  ++MoveNbr;
}

/* Allocate a STRestartGuardRootDefenderFilter slice
 * @return allocated slice
 */
static slice_index alloc_restart_guard_root_defender_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuardRootDefenderFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRestartGuardHelpFilter slice
 * @return allocated slice
 */
static slice_index alloc_restart_guard_help_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuardHelpFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRestartGuard slice
 * @return allocated slice
 */
static slice_index alloc_restart_guard_series_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuardSeriesFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defending side can successfully defend
 */
boolean restart_guard_root_defend(slice_index si, stip_length_type n_min)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  IncrementMoveNbr();

  if (MoveNbr<=RestartNbr)
    result = true;
  else
    result = defense_root_defend(slices[si].u.pipe.next,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  IncrementMoveNbr();

  TraceValue("%u",MoveNbr);
  TraceValue("%u\n",RestartNbr);
  if (MoveNbr<=RestartNbr)
    result = false;
  else
    result = help_solve_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  IncrementMoveNbr();

  if (MoveNbr<=RestartNbr)
    result = false;
  else
    result = series_solve_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void restart_guards_inserter_attack_root(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(si,alloc_restart_guard_root_defender_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void restart_guards_inserter_help(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(si,alloc_restart_guard_help_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void restart_guards_inserter_series(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(si,alloc_restart_guard_series_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const restart_guards_inserters[] =
{
  &stip_traverse_structure_children,      /* STProxy */
  &stip_structure_visitor_noop,         /* STAttackMove */
  &stip_structure_visitor_noop,         /* STDefenseMove */
  &restart_guards_inserter_help, /* STHelpMove */
  &stip_structure_visitor_noop,         /* STHelpFork */
  &restart_guards_inserter_series, /* STSeriesMove */
  &stip_structure_visitor_noop,         /* STSeriesFork */
  &stip_structure_visitor_noop,         /* STLeafDirect */
  &stip_structure_visitor_noop,         /* STLeafHelp */
  &stip_structure_visitor_noop,         /* STLeafForced */
  &stip_traverse_structure_children,      /* STReciprocal */
  &stip_traverse_structure_children,      /* STQuodlibet */
  &stip_traverse_structure_children,      /* STNot */
  &stip_traverse_structure_children,      /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,      /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,      /* STMoveInverterSeriesFilter */
  &restart_guards_inserter_attack_root, /* STAttackRoot */
  &stip_structure_visitor_noop,         /* STBattlePlaySolutionWriter */
  &stip_structure_visitor_noop,         /* STPostKeyPlaySolutionWriter */
  &stip_structure_visitor_noop,         /* STContinuationWriter */
  &stip_structure_visitor_noop,         /* STTryWriter */
  &stip_structure_visitor_noop,         /* STThreatWriter */
  &stip_structure_visitor_noop,         /* STThreatEnforcer */
  &stip_structure_visitor_noop,         /* STRefutationsCollector */
  &stip_structure_visitor_noop,         /* STVariationWriter */
  &stip_structure_visitor_noop,         /* STRefutingVariationWriter */
  &stip_structure_visitor_noop,         /* STNoShortVariations */
  &stip_traverse_structure_children,      /* STAttackHashed */
  &stip_traverse_structure_children,      /* STHelpRoot */
  &pipe_traverse_next,           /* STHelpShortcut */
  &stip_traverse_structure_children,      /* STHelpHashed */
  &stip_traverse_structure_children,      /* STSeriesRoot */
  &pipe_traverse_next,           /* STSeriesShortcut */
  &stip_traverse_structure_children,      /* STParryFork */
  &stip_traverse_structure_children,      /* STSeriesHashed */
  &stip_traverse_structure_children,      /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,      /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STDirectDefenseRootSolvableFilter */
  &stip_traverse_structure_children,      /* STDirectDefense */
  &stip_traverse_structure_children,      /* STReflexHelpFilter */
  &stip_traverse_structure_children,      /* STReflexSeriesFilter */
  &stip_traverse_structure_children,      /* STReflexRootSolvableFilter */
  &stip_traverse_structure_children,      /* STReflexAttackerFilter */
  &stip_traverse_structure_children,      /* STReflexDefenderFilter */
  &stip_traverse_structure_children,      /* STSelfDefense */
  &stip_traverse_structure_children,      /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,      /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,      /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,      /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,      /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,      /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,      /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,      /* STMaxFlightsquares */
  &stip_traverse_structure_children,      /* STDegenerateTree */
  &stip_traverse_structure_children,      /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,      /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,      /* STMaxThreatLength */
  &stip_traverse_structure_children,      /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,      /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,      /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,      /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,      /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,      /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,      /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children       /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument stipulation with STRestartGuard slices
 */
void stip_insert_restart_guards(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&restart_guards_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
