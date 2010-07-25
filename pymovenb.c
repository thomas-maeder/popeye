#include "pymovenb.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "pyoutput.h"
#include "output/plaintext/plaintext.h"
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
 * @param si slice index
 */
static void IncrementMoveNbr(slice_index si)
{
  if (MoveNbr>=RestartNbr)
  {
    sprintf(GlobalStr,"\n%3u  (", MoveNbr);
    StdString(GlobalStr);
    output_plaintext_write_move(nbply);
    if (echecc(nbply,slices[si].starter))
      StdString(" +");
    StdChar(blank);

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

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type restart_guard_defend_in_n(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  IncrementMoveNbr(si);

  if (MoveNbr<=RestartNbr)
    result = n+4;
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_help_solve_in_n(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  IncrementMoveNbr(si);

  TraceValue("%u",MoveNbr);
  TraceValue("%u\n",RestartNbr);
  if (MoveNbr<=RestartNbr)
    result = n+2;
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
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_series_solve_in_n(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  IncrementMoveNbr(si);

  if (MoveNbr<=RestartNbr)
    result = n+1;
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

static void restart_guards_inserter_help(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(si,alloc_restart_guard_help_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void restart_guards_inserter_series(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(si,alloc_restart_guard_series_filter());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void restart_guards_inserter_quodlibet(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* this hack prevents move numbers from being printed for the set
   * play */
  stip_traverse_structure(slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const restart_guards_inserters[] =
{
  &stip_traverse_structure_children,    /* STProxy */
  &stip_structure_visitor_noop,         /* STAttackMove */
  &stip_structure_visitor_noop,         /* STDefenseMove */
  &stip_structure_visitor_noop,         /* STDefenseMoveAgainstGoal */
  &restart_guards_inserter_help,        /* STHelpMove */
  &restart_guards_inserter_help,        /* STHelpMoveToGoal */
  &stip_structure_visitor_noop,         /* STHelpFork */
  &restart_guards_inserter_series,      /* STSeriesMove */
  &restart_guards_inserter_series,      /* STSeriesMoveToGoal */
  &stip_structure_visitor_noop,         /* STSeriesFork */
  &stip_structure_visitor_noop,         /* STGoalReachedTester */
  &stip_structure_visitor_noop,         /* STLeaf */
  &stip_traverse_structure_children,    /* STReciprocal */
  &restart_guards_inserter_quodlibet,   /* STQuodlibet */
  &stip_structure_visitor_noop,         /* STNot */
  &stip_traverse_structure_children,    /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,    /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,    /* STMoveInverterSeriesFilter */
  &restart_guards_inserter_attack_root, /* STAttackRoot */
  &stip_structure_visitor_noop,         /* STDefenseRoot */
  &stip_structure_visitor_noop,         /* STPostKeyPlaySuppressor */
  &stip_structure_visitor_noop,         /* STContinuationSolver */
  &stip_structure_visitor_noop,         /* STContinuationWriter */
  &stip_structure_visitor_noop,         /* STBattlePlaySolver */
  &stip_structure_visitor_noop,         /* STBattlePlaySolutionWriter */
  &stip_structure_visitor_noop,         /* STThreatSolver */
  &stip_structure_visitor_noop,         /* STZugzwangWriter */
  &stip_structure_visitor_noop,         /* STThreatEnforcer */
  &stip_structure_visitor_noop,         /* STThreatCollector */
  &stip_structure_visitor_noop,         /* STRefutationsCollector */
  &stip_structure_visitor_noop,         /* STVariationWriter */
  &stip_structure_visitor_noop,         /* STRefutingVariationWriter */
  &stip_structure_visitor_noop,         /* STNoShortVariations */
  &stip_traverse_structure_children,    /* STAttackHashed */
  &stip_traverse_structure_children,    /* STHelpRoot */
  &stip_traverse_structure_pipe,        /* STHelpShortcut */
  &stip_traverse_structure_children,    /* STHelpHashed */
  &stip_traverse_structure_children,    /* STSeriesRoot */
  &stip_traverse_structure_pipe,        /* STSeriesShortcut */
  &stip_traverse_structure_children,    /* STParryFork */
  &stip_traverse_structure_children,    /* STSeriesHashed */
  &stip_traverse_structure_children,    /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,    /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,    /* STSelfCheckGuardAttackerFilter */
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
  &stip_traverse_structure_children,    /* STDefenseEnd */
  &stip_traverse_structure_children,    /* STDefenseFork */
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
  &stip_traverse_structure_children,    /* STMaxNrNonChecks */
  &stip_traverse_structure_children,    /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,    /* STMaxThreatLength */
  &stip_traverse_structure_children,    /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,    /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,    /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,    /* STMaxTimeSeriesFilter */
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
