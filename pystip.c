#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
#include "stipulation/goal_reached_tester.h"
#include "pyrecipr.h"
#include "pyquodli.h"
#include "pybrafrk.h"
#include "pynot.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pydirctg.h"
#include "pymovein.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pydirctg.h"
#include "pypipe.h"
#include "stipulation/operators/binary.h"
#include "stipulation/branch.h"
#include "stipulation/leaf.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/fork.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/shortcut.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/move_to_goal.h"
#include "stipulation/series_play/shortcut.h"
#include "stipulation/series_play/or.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/parry_fork.h"
#include "stipulation/series_play/not_last_move.h"
#include "stipulation/series_play/only_last_move.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME SliceType
#define ENUMERATORS \
  ENUMERATOR(STProxy),                                                  \
    ENUMERATOR(STAttackMove),      /* M-N moves of direct play */       \
    ENUMERATOR(STDefenseMove),                                          \
    ENUMERATOR(STHelpMove),      /* M-N moves of help play */           \
    ENUMERATOR(STHelpFork),        /* decides when play in branch is over */ \
    ENUMERATOR(STSeriesMove),    /* M-N moves of series play */         \
    ENUMERATOR(STSeriesMoveToGoal),    /* last series move reaching goal */ \
    ENUMERATOR(STSeriesNotLastMove),  /* stop solving last series move through wrong track */ \
    ENUMERATOR(STSeriesOnlyLastMove),  /* stop solving unless last series move */ \
    ENUMERATOR(STSeriesFork),      /* decides when play in branch is over */ \
    ENUMERATOR(STSeriesOR),         /* OR series filter */ \
    ENUMERATOR(STGoalReachedTester), /* tests whether a goal has been reached */ \
    ENUMERATOR(STLeaf),            /* leaf slice */                     \
    ENUMERATOR(STReciprocal),      /* logical AND */                    \
    ENUMERATOR(STQuodlibet),       /* logical OR */                     \
    ENUMERATOR(STNot),             /* logical NOT */                    \
    ENUMERATOR(STMoveInverterRootSolvableFilter),    /* inverts side to move */ \
    ENUMERATOR(STMoveInverterSolvableFilter),    /* inverts side to move */ \
    ENUMERATOR(STMoveInverterSeriesFilter),    /* inverts side to move */ \
    ENUMERATOR(STAttackRoot),      /* root level of direct play */      \
    ENUMERATOR(STDefenseRoot),      /* root defense level of battle play */ \
    ENUMERATOR(STPostKeyPlaySuppressor), /* suppresses output of post key play */ \
    ENUMERATOR(STContinuationSolver), /* solves battle play continuations */ \
    ENUMERATOR(STContinuationWriter), /* writes battle play continuations */ \
    ENUMERATOR(STBattlePlaySolver), /* find battle play solutions */    \
    ENUMERATOR(STBattlePlaySolutionWriter), /* write battle play solutions */ \
    ENUMERATOR(STThreatSolver), /* solves threats */                    \
    ENUMERATOR(STZugzwangWriter), /* writes zugzwang if appropriate */  \
    ENUMERATOR(STThreatEnforcer), /* filters out defense that don't defend against the threat(s) */ \
    ENUMERATOR(STThreatCollector), /* collects threats */               \
    ENUMERATOR(STRefutationsCollector), /* collections refutations */   \
    ENUMERATOR(STVariationWriter), /* writes variations */              \
    ENUMERATOR(STRefutingVariationWriter), /* writes refuting variations */ \
    ENUMERATOR(STNoShortVariations), /* filters out short variations */ \
    ENUMERATOR(STAttackHashed),    /* direct play with hash table */    \
    ENUMERATOR(STHelpRoot),        /* root level of help play */        \
    ENUMERATOR(STHelpShortcut),    /* selects branch for solving short solutions */        \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
    ENUMERATOR(STSeriesRoot),      /* root level of series play */      \
    ENUMERATOR(STSeriesShortcut),  /* selects branch for solving short solutions */ \
    ENUMERATOR(STParryFork),       /* parry move in series */           \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */    \
    ENUMERATOR(STSelfCheckGuardRootSolvableFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardSolvableFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardAttackerFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardDefenderFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardHelpFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardSeriesFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STDirectDefenderFilter),   /* direct play, just played attack */ \
    ENUMERATOR(STReflexRootFilter),/* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexHelpFilter),/* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexSeriesFilter),    /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexAttackerFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexDefenderFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSelfDefense),     /* self play, just played defense */ \
    ENUMERATOR(STRestartGuardRootDefenderFilter),    /* write move numbers */ \
    ENUMERATOR(STRestartGuardHelpFilter),    /* write move numbers */   \
    ENUMERATOR(STRestartGuardSeriesFilter),    /* write move numbers */ \
    ENUMERATOR(STIntelligentHelpFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STIntelligentSeriesFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardHelpFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardSeriesFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STIntelligentDuplicateAvoider), /* avoid double solutions in intelligent mode */ \
    ENUMERATOR(STKeepMatingGuardAttackerFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardDefenderFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardHelpFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardSeriesFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonTrivialCounter), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
    ENUMERATOR(STMaxTimeRootDefenderFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeDefenderFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeHelpFilter), /* deals with option maxtime */    \
    ENUMERATOR(STMaxTimeSeriesFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxSolutionsRootSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsRootDefenderFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsHelpFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSeriesFilter), /* deals with option maxsolutions */ \
    ENUMERATOR(STStopOnShortSolutionsRootSolvableFilter), /* deals with option stoponshortsolutions */  \
    ENUMERATOR(STStopOnShortSolutionsHelpFilter), /* deals with option stoponshortsolutions */  \
     ENUMERATOR(STStopOnShortSolutionsSeriesFilter), /* deals with option stoponshortsolutions */  \
    ENUMERATOR(STEndOfPhaseWriter), /* write the end of a phase */  \
    ENUMERATOR(STEndOfSolutionWriter), /* write the end of a solution */  \
    ENUMERATOR(STRefutationWriter), /* writes refutations */  \
    ENUMERATOR(STOutputPlaintextTreeCheckDetectorAttackerFilter), /* plain text output, tree mode: detect checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextTreeCheckDetectorDefenderFilter), /* plain text output, tree mode: detect checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextLineLineWriter), /* plain text output, line mode: write a line */  \
    ENUMERATOR(STOutputPlaintextTreeGoalWriter), /* plain text output, tree mode: write the reached goal */  \
    ENUMERATOR(STOutputPlaintextTreeMoveInversionCounter), /* plain text output, tree mode: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineMoveInversionCounter), /* plain text output, line mode: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineEndOfIntroSeriesMarker), /* handles the end of the intro series */  \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


#define ENUMERATION_TYPENAME branch_level
#define ENUMERATORS \
  ENUMERATOR(toplevel_branch),                  \
    ENUMERATOR(nested_branch)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


Slice slices[max_nr_slices];

slice_index root_slice = no_slice;


#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                             \
  ENUMERATOR(leaf)                              \
    ENUMERATOR(binary)                          \
    ENUMERATOR(pipe)                            \
    ENUMERATOR(branch)                          \
    ENUMERATOR(fork)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

static slice_structural_type highest_structural_type[max_nr_slices] =
{
  slice_structure_pipe,   /* STProxy */
  slice_structure_branch, /* STAttackMove */
  slice_structure_branch, /* STDefenseMove */
  slice_structure_branch, /* STHelpMove */
  slice_structure_fork,   /* STHelpFork */
  slice_structure_branch, /* STSeriesMove */
  slice_structure_branch, /* STSeriesMoveToGoal */
  slice_structure_branch, /* STSeriesNotLastMove */
  slice_structure_branch, /* STSeriesOnlyLastMove */
  slice_structure_fork,   /* STSeriesFork */
  slice_structure_fork,   /* STSeriesOR */
  slice_structure_pipe,   /* STGoalReachedTester */
  slice_structure_leaf,   /* STLeaf */
  slice_structure_binary, /* STReciprocal */
  slice_structure_binary, /* STQuodlibet */
  slice_structure_pipe,   /* STNot */
  slice_structure_pipe,   /* STMoveInverterRootSolvableFilter */
  slice_structure_pipe,   /* STMoveInverterSolvableFilter */
  slice_structure_pipe,   /* STMoveInverterSeriesFilter */
  slice_structure_branch, /* STAttackRoot */
  slice_structure_branch, /* STDefenseRoot */
  slice_structure_branch, /* STPostKeyPlaySuppressor */
  slice_structure_branch, /* STContinuationSolver */
  slice_structure_branch, /* STContinuationWriter */
  slice_structure_branch, /* STBattlePlaySolver */
  slice_structure_branch, /* STBattlePlaySolutionWriter */
  slice_structure_branch, /* STThreatSolver */
  slice_structure_branch, /* STZugzwangWriter */
  slice_structure_branch, /* STThreatEnforcer */
  slice_structure_branch, /* STThreatCollector */
  slice_structure_branch, /* STRefutationsCollector */
  slice_structure_branch, /* STVariationWriter */
  slice_structure_branch, /* STRefutingVariationWriter */
  slice_structure_branch, /* STNoShortVariations */
  slice_structure_branch, /* STAttackHashed */
  slice_structure_branch, /* STHelpRoot */
  slice_structure_branch, /* STHelpShortcut */
  slice_structure_branch, /* STHelpHashed */
  slice_structure_branch, /* STSeriesRoot */
  slice_structure_branch, /* STSeriesShortcut */
  slice_structure_pipe,   /* STParryFork */
  slice_structure_branch, /* STSeriesHashed */
  slice_structure_pipe,   /* STSelfCheckGuardRootSolvableFilter */
  slice_structure_pipe,   /* STSelfCheckGuardSolvableFilter */
  slice_structure_pipe,   /* STSelfCheckGuardAttackerFilter */
  slice_structure_pipe,   /* STSelfCheckGuardDefenderFilter */
  slice_structure_pipe,   /* STSelfCheckGuardHelpFilter */
  slice_structure_pipe,   /* STSelfCheckGuardSeriesFilter */
  slice_structure_fork,   /* STDirectDefenderFilter */
  slice_structure_fork,   /* STReflexRootFilter */
  slice_structure_fork,   /* STReflexHelpFilter */
  slice_structure_fork,   /* STReflexSeriesFilter */
  slice_structure_fork,   /* STReflexAttackerFilter */
  slice_structure_fork,   /* STReflexDefenderFilter */
  slice_structure_fork,   /* STSelfDefense */
  slice_structure_pipe,   /* STRestartGuardRootDefenderFilter */
  slice_structure_pipe,   /* STRestartGuardHelpFilter */
  slice_structure_pipe,   /* STRestartGuardSeriesFilter */
  slice_structure_branch, /* STIntelligentHelpFilter */
  slice_structure_branch, /* STIntelligentSeriesFilter */
  slice_structure_branch, /* STGoalReachableGuardHelpFilter */
  slice_structure_branch, /* STGoalReachableGuardSeriesFilter */
  slice_structure_pipe,   /* STIntelligentDuplicateAvoider */
  slice_structure_pipe,   /* STKeepMatingGuardAttackerFilter */
  slice_structure_pipe,   /* STKeepMatingGuardDefenderFilter */
  slice_structure_pipe,   /* STKeepMatingGuardHelpFilter */
  slice_structure_pipe,   /* STKeepMatingGuardSeriesFilter */
  slice_structure_pipe,   /* STMaxFlightsquares */
  slice_structure_pipe,   /* STDegenerateTree */
  slice_structure_branch, /* STMaxNrNonTrivial */
  slice_structure_branch, /* STMaxNrNonTrivialCounter */
  slice_structure_pipe,   /* STMaxThreatLength */
  slice_structure_pipe,   /* STMaxTimeRootDefenderFilter */
  slice_structure_pipe,   /* STMaxTimeDefenderFilter */
  slice_structure_pipe,   /* STMaxTimeHelpFilter */
  slice_structure_pipe,   /* STMaxTimeSeriesFilter */
  slice_structure_pipe,   /* STMaxSolutionsRootSolvableFilter */
  slice_structure_pipe,   /* STMaxSolutionsSolvableFilter */
  slice_structure_pipe,   /* STMaxSolutionsRootDefenderFilter */
  slice_structure_pipe,   /* STMaxSolutionsHelpFilter */
  slice_structure_pipe,   /* STMaxSolutionsSeriesFilter */
  slice_structure_pipe,   /* STStopOnShortSolutionsRootSolvableFilter */
  slice_structure_branch, /* STStopOnShortSolutionsHelpFilter */
  slice_structure_branch, /* STStopOnShortSolutionsSeriesFilter */
  slice_structure_branch, /* STEndOfPhaseWriter */
  slice_structure_branch, /* STEndOfSolutionWriter */
  slice_structure_branch, /* STRefutationWriter */
  slice_structure_branch, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  slice_structure_branch, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  slice_structure_pipe,   /* STOutputPlaintextLineLineWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeGoalWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeMoveInversionCounter */
  slice_structure_pipe,   /* STOutputPlaintextLineMoveInversionCounter */
  slice_structure_pipe    /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Determine whether a slice is of some structural type
 * @param si identifies slice
 * @param type identifies type
 * @return true iff slice si has (at least) structural type type
 */
boolean slice_has_structure(slice_index si, slice_structural_type type)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_structural_type,type,"");
  TraceFunctionParamListEnd();

  switch (highest_structural_type[slices[si].type])
  {
    case slice_structure_leaf:
      result = type==slice_structure_leaf;
      break;

    case slice_structure_binary:
      result = type==slice_structure_binary;
      break;

    case slice_structure_pipe:
      result = type==slice_structure_pipe;
      break;

    case slice_structure_branch:
      result = type==slice_structure_pipe || type==slice_structure_branch;
      break;

    case slice_structure_fork:
      result = (type==slice_structure_pipe
                || type==slice_structure_branch
                || type==slice_structure_fork);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_slice_index_free[max_nr_slices];

static void mark_reachable_slice(slice_index si, stip_structure_traversal *st)
{
  boolean (* const leaked)[max_nr_slices] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(!is_slice_index_free[si]);
  (*leaked)[si] = false;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const reachable_slices_markers[] =
{
  0,                     /* STProxy */
  &mark_reachable_slice, /* STAttackMove */
  &mark_reachable_slice, /* STDefenseMove */
  &mark_reachable_slice, /* STHelpMove */
  &mark_reachable_slice, /* STHelpFork */
  &mark_reachable_slice, /* STSeriesMove */
  &mark_reachable_slice, /* STSeriesMoveToGoal */
  &mark_reachable_slice, /* STSeriesNotLastMove */
  &mark_reachable_slice, /* STSeriesOnlyLastMove */
  &mark_reachable_slice, /* STSeriesFork */
  &mark_reachable_slice, /* STSeriesOR */
  &mark_reachable_slice, /* STGoalReachedTester */
  &mark_reachable_slice, /* STLeaf */
  &mark_reachable_slice, /* STReciprocal */
  &mark_reachable_slice, /* STQuodlibet */
  &mark_reachable_slice, /* STNot */
  &mark_reachable_slice, /* STMoveInverterRootSolvableFilter */
  &mark_reachable_slice, /* STMoveInverterSolvableFilter */
  &mark_reachable_slice, /* STMoveInverterSeriesFilter */
  &mark_reachable_slice, /* STAttackRoot */
  &mark_reachable_slice, /* STDefenseRoot */
  &mark_reachable_slice, /* STPostKeyPlaySuppressor */
  &mark_reachable_slice, /* STContinuationSolver */
  &mark_reachable_slice, /* STContinuationWriter */
  &mark_reachable_slice, /* STBattlePlaySolver */
  &mark_reachable_slice, /* STBattlePlaySolutionWriter */
  &mark_reachable_slice, /* STThreatSolver */
  &mark_reachable_slice, /* STZugzwangWriter */
  &mark_reachable_slice, /* STThreatEnforcer */
  &mark_reachable_slice, /* STThreatCollector */
  &mark_reachable_slice, /* STRefutationsCollector */
  &mark_reachable_slice, /* STVariationWriter */
  &mark_reachable_slice, /* STRefutingVariationWriter */
  &mark_reachable_slice, /* STNoShortVariations */
  &mark_reachable_slice, /* STAttackHashed */
  &mark_reachable_slice, /* STHelpRoot */
  &mark_reachable_slice, /* STHelpShortcut */
  &mark_reachable_slice, /* STHelpHashed */
  &mark_reachable_slice, /* STSeriesRoot */
  &mark_reachable_slice, /* STSeriesShortcut */
  &mark_reachable_slice, /* STParryFork */
  &mark_reachable_slice, /* STSeriesHashed */
  &mark_reachable_slice, /* STSelfCheckGuardRootSolvableFilter */
  &mark_reachable_slice, /* STSelfCheckGuardSolvableFilter */
  &mark_reachable_slice, /* STSelfCheckGuardAttackerFilter */
  &mark_reachable_slice, /* STSelfCheckGuardDefenderFilter */
  &mark_reachable_slice, /* STSelfCheckGuardHelpFilter */
  &mark_reachable_slice, /* STSelfCheckGuardSeriesFilter */
  &mark_reachable_slice, /* STDirectDefenderFilter */
  &mark_reachable_slice, /* STReflexRootFilter */
  &mark_reachable_slice, /* STReflexHelpFilter */
  &mark_reachable_slice, /* STReflexSeriesFilter */
  &mark_reachable_slice, /* STReflexAttackerFilter */
  &mark_reachable_slice, /* STReflexDefenderFilter */
  &mark_reachable_slice, /* STSelfDefense */
  &mark_reachable_slice, /* STRestartGuardRootDefenderFilter */
  &mark_reachable_slice, /* STRestartGuardHelpFilter */
  &mark_reachable_slice, /* STRestartGuardSeriesFilter */
  &mark_reachable_slice, /* STIntelligentHelpFilter */
  &mark_reachable_slice, /* STIntelligentSeriesFilter */
  &mark_reachable_slice, /* STGoalReachableGuardHelpFilter */
  &mark_reachable_slice, /* STGoalReachableGuardSeriesFilter */
  &mark_reachable_slice, /* STIntelligentDuplicateAvoider */
  &mark_reachable_slice, /* STKeepMatingGuardAttackerFilter */
  &mark_reachable_slice, /* STKeepMatingGuardDefenderFilter */
  &mark_reachable_slice, /* STKeepMatingGuardHelpFilter */
  &mark_reachable_slice, /* STKeepMatingGuardSeriesFilter */
  &mark_reachable_slice, /* STMaxFlightsquares */
  &mark_reachable_slice, /* STDegenerateTree */
  &mark_reachable_slice, /* STMaxNrNonTrivial */
  &mark_reachable_slice, /* STMaxNrNonTrivialCounter */
  &mark_reachable_slice, /* STMaxThreatLength */
  &mark_reachable_slice, /* STMaxTimeRootDefenderFilter */
  &mark_reachable_slice, /* STMaxTimeDefenderFilter */
  &mark_reachable_slice, /* STMaxTimeHelpFilter */
  &mark_reachable_slice, /* STMaxTimeSeriesFilter */
  &mark_reachable_slice, /* STMaxSolutionsRootSolvableFilter */
  &mark_reachable_slice, /* STMaxSolutionsSolvableFilter */
  &mark_reachable_slice, /* STMaxSolutionsRootDefenderFilter */
  &mark_reachable_slice, /* STMaxSolutionsHelpFilter */
  &mark_reachable_slice, /* STMaxSolutionsSeriesFilter */
  &mark_reachable_slice, /* STStopOnShortSolutionsRootSolvableFilter */
  &mark_reachable_slice, /* STStopOnShortSolutionsHelpFilter */
  &mark_reachable_slice, /* STStopOnShortSolutionsSeriesFilter */
  &mark_reachable_slice, /* STEndOfPhaseWriter */
  &mark_reachable_slice, /* STEndOfSolutionWriter */
  &mark_reachable_slice, /* STRefutationWriter */
  &mark_reachable_slice, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &mark_reachable_slice, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &mark_reachable_slice, /* STOutputPlaintextLineLineWriter */
  &mark_reachable_slice, /* STOutputPlaintextTreeGoalWriter */
  &mark_reachable_slice, /* STOutputPlaintextTreeMoveInversionCounter */
  &mark_reachable_slice, /* STOutputPlaintextLineMoveInversionCounter */
  &mark_reachable_slice  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void)
{
  boolean leaked[max_nr_slices];
  slice_index i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);
  if (root_slice!=no_slice)
  {
    for (i = 0; i!=max_nr_slices; ++i)
      leaked[i] = !is_slice_index_free[i];

    stip_structure_traversal_init(&st,&reachable_slices_markers,&leaked);
    stip_traverse_structure(root_slice,&st);

    for (i = 0; i!=max_nr_slices; ++i)
    {
      if (leaked[i])
      { /* TraceValue expands to nothing unless DOTRACE is #defined */
        TraceValue("leaked:%u\n",i);
      }
      assert(!leaked[i]);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
void init_slice_allocator(void)
{
  slice_index si;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (si = 0; si!=max_nr_slices; ++si)
    is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a slice index
 * @param type which type
 * @return a so far unused slice index
 */
slice_index alloc_slice(SliceType type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (result = 0; result!=max_nr_slices; ++result)
    if (is_slice_index_free[result])
      break;
  
  assert(result<max_nr_slices);

  is_slice_index_free[result] = false;

  slices[result].type = type; 
  slices[result].starter = no_side; 

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(!is_slice_index_free[si]);
  is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",original);
  TraceFunctionParamListEnd();

  if (slices[original].type==STProxy)
    result = alloc_proxy_slice();
  else
    result = alloc_slice(slices[original].type);

  slices[result] = slices[original];
  slice_set_predecessor(result,no_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",slice);
  TraceFunctionParam("%u",pred);
  TraceFunctionParamListEnd();

  slices[slice].prev = pred;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void traverse_and_deallocate(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void traverse_and_deallocate_proxy(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_proxy_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const deallocators[] =
{
  &traverse_and_deallocate_proxy, /* STProxy */
  &traverse_and_deallocate,       /* STAttackMove */
  &traverse_and_deallocate,       /* STDefenseMove */
  &traverse_and_deallocate,       /* STHelpMove */
  &traverse_and_deallocate,       /* STHelpFork */
  &traverse_and_deallocate,       /* STSeriesMove */
  &traverse_and_deallocate,       /* STSeriesMoveToGoal */
  &traverse_and_deallocate,       /* STSeriesNotLastMove */
  &traverse_and_deallocate,       /* STSeriesOnlyLastMove */
  &traverse_and_deallocate,       /* STSeriesFork */
  &traverse_and_deallocate,       /* STSeriesOR */
  &traverse_and_deallocate,       /* STGoalReachedTester */
  &traverse_and_deallocate,       /* STLeaf */
  &traverse_and_deallocate,       /* STReciprocal */
  &traverse_and_deallocate,       /* STQuodlibet */
  &traverse_and_deallocate,       /* STNot */
  &traverse_and_deallocate,       /* STMoveInverterRootSolvableFilter */
  &traverse_and_deallocate,       /* STMoveInverterSolvableFilter */
  &traverse_and_deallocate,       /* STMoveInverterSeriesFilter */
  &traverse_and_deallocate,       /* STAttackRoot */
  &traverse_and_deallocate,       /* STDefenseRoot */
  &traverse_and_deallocate,       /* STPostKeyPlaySuppressor */
  &traverse_and_deallocate,       /* STContinuationSolver */
  &traverse_and_deallocate,       /* STContinuationWriter */
  &traverse_and_deallocate,       /* STBattlePlaySolver */
  &traverse_and_deallocate,       /* STBattlePlaySolutionWriter */
  &traverse_and_deallocate,       /* STThreatSolver */
  &traverse_and_deallocate,       /* STZugzwangWriter */
  &traverse_and_deallocate,       /* STThreatEnforcer */
  &traverse_and_deallocate,       /* STThreatCollector */
  &traverse_and_deallocate,       /* STRefutationsCollector */
  &traverse_and_deallocate,       /* STVariationWriter */
  &traverse_and_deallocate,       /* STRefutingVariationWriter */
  &traverse_and_deallocate,       /* STNoShortVariations */
  &traverse_and_deallocate,       /* STAttackHashed */
  &traverse_and_deallocate,       /* STHelpRoot */
  &traverse_and_deallocate,       /* STHelpShortcut */
  &traverse_and_deallocate,       /* STHelpHashed */
  &traverse_and_deallocate,       /* STSeriesRoot */
  &traverse_and_deallocate,       /* STSeriesShortcut */
  &traverse_and_deallocate,       /* STParryFork */
  &traverse_and_deallocate,       /* STSeriesHashed */
  &traverse_and_deallocate,       /* STSelfCheckGuardRootSolvableFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardSolvableFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardAttackerFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardDefenderFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardHelpFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardSeriesFilter */
  &traverse_and_deallocate,       /* STDirectDefenderFilter */
  &traverse_and_deallocate,       /* STReflexRootFilter */
  &traverse_and_deallocate,       /* STReflexHelpFilter */
  &traverse_and_deallocate,       /* STReflexSeriesFilter */
  &traverse_and_deallocate,       /* STReflexAttackerFilter */
  &traverse_and_deallocate,       /* STReflexDefenderFilter */
  &traverse_and_deallocate,       /* STSelfDefense */
  &traverse_and_deallocate,       /* STRestartGuardRootDefenderFilter */
  &traverse_and_deallocate,       /* STRestartGuardHelpFilter */
  &traverse_and_deallocate,       /* STRestartGuardSeriesFilter */
  &traverse_and_deallocate,       /* STIntelligentHelpFilter */
  &traverse_and_deallocate,       /* STIntelligentSeriesFilter */
  &traverse_and_deallocate,       /* STGoalReachableGuardHelpFilter */
  &traverse_and_deallocate,       /* STGoalReachableGuardSeriesFilter */
  &traverse_and_deallocate,       /* STIntelligentDuplicateAvoider */
  &traverse_and_deallocate,       /* STKeepMatingGuardAttackerFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardDefenderFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardHelpFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardSeriesFilter */
  &traverse_and_deallocate,       /* STMaxFlightsquares */
  &traverse_and_deallocate,       /* STDegenerateTree */
  &traverse_and_deallocate,       /* STMaxNrNonTrivial */
  &traverse_and_deallocate,       /* STMaxNrNonTrivialCounter */
  &traverse_and_deallocate,       /* STMaxThreatLength */
  &traverse_and_deallocate,       /* STMaxTimeRootDefenderFilter */
  &traverse_and_deallocate,       /* STMaxTimeDefenderFilter */
  &traverse_and_deallocate,       /* STMaxTimeHelpFilter */
  &traverse_and_deallocate,       /* STMaxTimeSeriesFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsRootSolvableFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsSolvableFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsRootDefenderFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsHelpFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsSeriesFilter */
  &traverse_and_deallocate,       /* STStopOnShortSolutionsRootSolvableFilter */
  &traverse_and_deallocate,       /* STStopOnShortSolutionsHelpFilter */
  &traverse_and_deallocate,       /* STStopOnShortSolutionsSeriesFilter */
  &traverse_and_deallocate,       /* STEndOfPhaseWriter */
  &traverse_and_deallocate,       /* STEndOfSolutionWriter */
  &traverse_and_deallocate,       /* STRefutationWriter */
  &traverse_and_deallocate,       /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &traverse_and_deallocate,       /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &traverse_and_deallocate,       /* STOutputPlaintextLineLineWriter */
  &traverse_and_deallocate,       /* STOutputPlaintextTreeGoalWriter */
  &traverse_and_deallocate,       /* STOutputPlaintextTreeMoveInversionCounter */
  &traverse_and_deallocate,       /* STOutputPlaintextLineMoveInversionCounter */
  &traverse_and_deallocate        /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&deallocators,0);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Release all slices
 */
void release_slices(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (root_slice!=no_slice)
  {
    dealloc_slices(root_slice);
    root_slice = no_slice;
  }

  assert_no_leaked_slices();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten the branches by the move that is represented by the root
 * slices
 */
void battle_branch_post_root_shorten(slice_index si,
                                     stip_structure_traversal *st);

static stip_structure_visitor const post_root_shorteners[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &battle_branch_post_root_shorten,  /* STAttackMove */
  &battle_branch_post_root_shorten,  /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesMoveToGoal */
  &stip_traverse_structure_children, /* STSeriesNotLastMove */
  &stip_traverse_structure_children, /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_traverse_structure_children, /* STSeriesOR */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_traverse_structure_children, /* STLeaf */
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
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &battle_branch_post_root_shorten,  /* STSelfCheckGuardAttackerFilter */
  &battle_branch_post_root_shorten,  /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &battle_branch_post_root_shorten,  /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexRootFilter */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &battle_branch_post_root_shorten,  /* STReflexAttackerFilter */
  &battle_branch_post_root_shorten,  /* STReflexDefenderFilter */
  &battle_branch_post_root_shorten,  /* STSelfDefense */
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
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
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
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Shorten the branches by the move that is represented by the root
 * slices
 */
void battle_branch_post_root_shorten(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  battle_branch_shorten_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const root_slice_makers[] =
{
  &proxy_make_root,                           /* STProxy */
  &attack_move_make_root,                     /* STAttackMove */
  &defense_move_make_root,                    /* STDefenseMove */
  &stip_traverse_structure_children,          /* STHelpMove */
  &stip_traverse_structure_children,          /* STHelpFork */
  &stip_traverse_structure_children,          /* STSeriesMove */
  &stip_traverse_structure_children,          /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,          /* STSeriesNotLastMove */
  &stip_traverse_structure_children,          /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children,          /* STSeriesFork */
  &stip_traverse_structure_children,          /* STSeriesOR */
  &goal_reached_tester_make_root,             /* STGoalReachedTester */
  &leaf_make_root,                            /* STLeaf */
  &stip_traverse_structure_children,          /* STReciprocal */
  &quodlibet_make_root,                       /* STQuodlibet */
  &not_make_root,                             /* STNot */
  &stip_traverse_structure_children,          /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,          /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,          /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,          /* STAttackRoot */
  &stip_traverse_structure_children,          /* STDefenseRoot */
  &stip_traverse_structure_children,          /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,          /* STContinuationSolver */
  &stip_traverse_structure_children,          /* STContinuationWriter */
  &stip_traverse_structure_children,          /* STBattlePlaySolver */
  &stip_traverse_structure_children,          /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,          /* STThreatSolver */
  &stip_traverse_structure_children,          /* STZugzwangWriter */
  &stip_traverse_structure_children,          /* STThreatEnforcer */
  &stip_traverse_structure_children,          /* STThreatCollector */
  &stip_traverse_structure_children,          /* STRefutationsCollector */
  &stip_traverse_structure_children,          /* STVariationWriter */
  &stip_traverse_structure_children,          /* STRefutingVariationWriter */
  &stip_traverse_structure_children,          /* STNoShortVariations */
  &stip_traverse_structure_children,          /* STAttackHashed */
  &stip_traverse_structure_children,          /* STHelpRoot */
  &stip_traverse_structure_children,          /* STHelpShortcut */
  &stip_traverse_structure_children,          /* STHelpHashed */
  &stip_traverse_structure_children,          /* STSeriesRoot */
  &stip_traverse_structure_children,          /* STSeriesShortcut */
  &stip_traverse_structure_children,          /* STParryFork */
  &stip_traverse_structure_children,          /* STSeriesHashed */
  &stip_traverse_structure_children,          /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,          /* STSelfCheckGuardSolvableFilter */
  &selfcheck_guard_attacker_filter_make_root, /* STSelfCheckGuardAttackerFilter */
  &selfcheck_guard_defender_filter_make_root, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,          /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,          /* STSelfCheckGuardSeriesFilter */
  &direct_defender_filter_make_root,          /* STDirectDefenderFilter */
  &stip_traverse_structure_children,          /* STReflexRootFilter */
  &stip_traverse_structure_children,          /* STReflexHelpFilter */
  &stip_traverse_structure_children,          /* STReflexSeriesFilter */
  &reflex_attacker_filter_make_root,          /* STReflexAttackerFilter */
  &reflex_defender_filter_make_root,          /* STReflexDefenderFilter */
  &self_defense_make_root,                    /* STSelfDefense */
  &stip_traverse_structure_children,          /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,          /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,          /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,          /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,          /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,          /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,          /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,          /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,          /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,          /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,          /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,          /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,          /* STMaxFlightsquares */
  &stip_traverse_structure_children,          /* STDegenerateTree */
  &stip_traverse_structure_children,          /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,          /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,          /* STMaxThreatLength */
  &stip_traverse_structure_children,          /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,          /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,          /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,          /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,          /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,          /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,          /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,          /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,          /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,          /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,          /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,          /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,          /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,          /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,          /* STRefutationWriter */
  &stip_traverse_structure_children,          /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,          /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,          /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,          /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,          /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,          /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children           /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Recursively make a sequence of root slices
 * @param si identifies the initial non-root slice
 */
slice_index stip_make_root_slices(slice_index si)
{
  stip_structure_traversal st;
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&root_slice_makers,&result);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Create the root slices sequence for a battle play branch; shorten
 * the non-root slices by the moves represented by the root slices 
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void battle_branch_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  *root = stip_make_root_slices(si);

  stip_structure_traversal_init(&st_nested,&post_root_shorteners,0);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const root_slice_inserters[] =
{
  &proxy_make_root,                    /* STProxy */
  &battle_branch_make_root,            /* STAttackMove */
  &battle_branch_make_root,            /* STDefenseMove */
  &help_move_make_root,                /* STHelpMove */
  &help_fork_make_root,                /* STHelpFork */
  &series_move_make_root,              /* STSeriesMove */
  &series_move_to_goal_make_root,      /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,   /* STSeriesNotLastMove */
  &stip_traverse_structure_children,   /* STSeriesOnlyLastMove */
  &stip_traverse_structure_pipe,       /* STSeriesFork */
  &stip_traverse_structure_pipe,       /* STSeriesOR */
  &goal_reached_tester_make_root,      /* STGoalReachedTester */
  &leaf_make_root,                     /* STLeaf */
  &reci_make_root,                     /* STReciprocal */
  &quodlibet_make_root,                /* STQuodlibet */
  &not_make_root,                      /* STNot */
  &stip_traverse_structure_children,   /* STMoveInverterRootSolvableFilter */
  &move_inverter_make_root,            /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,   /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,   /* STAttackRoot */
  &stip_traverse_structure_children,   /* STDefenseRoot */
  &stip_traverse_structure_children,   /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,   /* STContinuationSolver */
  &stip_traverse_structure_children,   /* STContinuationWriter */
  &stip_traverse_structure_children,   /* STBattlePlaySolver */
  &stip_traverse_structure_children,   /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,   /* STThreatSolver */
  &stip_traverse_structure_children,   /* STZugzwangWriter */
  &stip_traverse_structure_children,   /* STThreatEnforcer */
  &stip_traverse_structure_children,   /* STThreatCollector */
  &stip_traverse_structure_children,   /* STRefutationsCollector */
  &stip_traverse_structure_children,   /* STVariationWriter */
  &stip_traverse_structure_children,   /* STRefutingVariationWriter */
  &stip_traverse_structure_children,   /* STNoShortVariations */
  &stip_traverse_structure_children,   /* STAttackHashed */
  &stip_traverse_structure_children,   /* STHelpRoot */
  &stip_traverse_structure_children,   /* STHelpShortcut */
  &stip_traverse_structure_children,   /* STHelpHashed */
  &stip_traverse_structure_children,   /* STSeriesRoot */
  &stip_traverse_structure_children,   /* STSeriesShortcut */
  &stip_traverse_structure_children,   /* STParryFork */
  &stip_traverse_structure_children,   /* STSeriesHashed */
  &stip_traverse_structure_children,   /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardSolvableFilter */
  &battle_branch_make_root,            /* STSelfCheckGuardAttackerFilter */
  &battle_branch_make_root,            /* STSelfCheckGuardDefenderFilter */
  &selfcheck_guard_help_make_root,     /* STSelfCheckGuardHelpFilter */
  &selfcheck_guard_series_make_root,   /* STSelfCheckGuardSeriesFilter */
  &battle_branch_make_root,            /* STDirectDefenderFilter */
  &stip_traverse_structure_children,   /* STReflexRootFilter */
  &reflex_help_filter_make_root,       /* STReflexHelpFilter */
  &reflex_series_filter_make_root,     /* STReflexSeriesFilter */
  &battle_branch_make_root,            /* STReflexAttackerFilter */
  &battle_branch_make_root,            /* STReflexDefenderFilter */
  &battle_branch_make_root,            /* STSelfDefense */
  &stip_traverse_structure_children,   /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,   /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,   /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,   /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,   /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,   /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,   /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxFlightsquares */
  &stip_traverse_structure_children,   /* STDegenerateTree */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,   /* STMaxThreatLength */
  &stip_traverse_structure_children,   /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,   /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,   /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,   /* STRefutationWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,   /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,   /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children    /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 */
void stip_insert_root_slices(void)
{
  stip_structure_traversal st;
  slice_index result = no_slice;
  slice_index const next = slices[root_slice].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);
  assert(slices[root_slice].type==STProxy);

  stip_structure_traversal_init(&st,&root_slice_inserters,&result);
  stip_traverse_structure(next,&st);

  if (slices[next].prev==root_slice)
  {
    TraceStipulation(next);
    TraceStipulation(result);
    dealloc_slices(next);
  }

  pipe_link(root_slice,result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const proxy_resolvers[] =
{
  &pipe_resolve_proxies,             /* STProxy */
  &pipe_resolve_proxies,             /* STAttackMove */
  &pipe_resolve_proxies,             /* STDefenseMove */
  &pipe_resolve_proxies,             /* STHelpMove */
  &branch_fork_resolve_proxies,      /* STHelpFork */
  &pipe_resolve_proxies,             /* STSeriesMove */
  &pipe_resolve_proxies,             /* STSeriesMoveToGoal */
  &pipe_resolve_proxies,             /* STSeriesNotLastMove */
  &pipe_resolve_proxies,             /* STSeriesOnlyLastMove */
  &branch_fork_resolve_proxies,      /* STSeriesFork */
  &branch_fork_resolve_proxies,      /* STSeriesOR */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_traverse_structure_children, /* STLeaf */
  &binary_resolve_proxies,           /* STReciprocal */
  &binary_resolve_proxies,           /* STQuodlibet */
  &pipe_resolve_proxies,             /* STNot */
  &pipe_resolve_proxies,             /* STMoveInverterRootSolvableFilter */
  &pipe_resolve_proxies,             /* STMoveInverterSolvableFilter */
  &pipe_resolve_proxies,             /* STMoveInverterSeriesFilter */
  &pipe_resolve_proxies,             /* STAttackRoot */
  &pipe_resolve_proxies,             /* STDefenseRoot */
  &pipe_resolve_proxies,             /* STPostKeyPlaySuppressor */
  &pipe_resolve_proxies,             /* STContinuationSolver */
  &pipe_resolve_proxies,             /* STContinuationWriter */
  &pipe_resolve_proxies,             /* STBattlePlaySolver */
  &pipe_resolve_proxies,             /* STBattlePlaySolutionWriter */
  &pipe_resolve_proxies,             /* STThreatSolver */
  &pipe_resolve_proxies,             /* STZugzwangWriter */
  &pipe_resolve_proxies,             /* STThreatEnforcer */
  &pipe_resolve_proxies,             /* STThreatCollector */
  &pipe_resolve_proxies,             /* STRefutationsCollector */
  &pipe_resolve_proxies,             /* STVariationWriter */
  &pipe_resolve_proxies,             /* STRefutingVariationWriter */
  &pipe_resolve_proxies,             /* STNoShortVariations */
  &pipe_resolve_proxies,             /* STAttackHashed */
  &pipe_resolve_proxies,             /* STHelpRoot */
  &help_shortcut_resolve_proxies,    /* STHelpShortcut */
  &pipe_resolve_proxies,             /* STHelpHashed */
  &pipe_resolve_proxies,             /* STSeriesRoot */
  &series_shortcut_resolve_proxies,  /* STSeriesShortcut */
  &parry_fork_resolve_proxies,       /* STParryFork */
  &pipe_resolve_proxies,             /* STSeriesHashed */
  &pipe_resolve_proxies,             /* STSelfCheckGuardRootSolvableFilter */
  &pipe_resolve_proxies,             /* STSelfCheckGuardSolvableFilter */
  &pipe_resolve_proxies,             /* STSelfCheckGuardAttackerFilter */
  &pipe_resolve_proxies,             /* STSelfCheckGuardDefenderFilter */
  &pipe_resolve_proxies,             /* STSelfCheckGuardHelpFilter */
  &pipe_resolve_proxies,             /* STSelfCheckGuardSeriesFilter */
  &branch_fork_resolve_proxies,      /* STDirectDefenderFilter */
  &reflex_filter_resolve_proxies,    /* STReflexRootFilter */
  &reflex_filter_resolve_proxies,    /* STReflexHelpFilter */
  &reflex_filter_resolve_proxies,    /* STReflexSeriesFilter */
  &reflex_filter_resolve_proxies,    /* STReflexAttackerFilter */
  &reflex_filter_resolve_proxies,    /* STReflexDefenderFilter */
  &branch_fork_resolve_proxies,      /* STSelfDefense */
  &pipe_resolve_proxies,             /* STRestartGuardRootDefenderFilter */
  &pipe_resolve_proxies,             /* STRestartGuardHelpFilter */
  &pipe_resolve_proxies,             /* STRestartGuardSeriesFilter */
  &pipe_resolve_proxies,             /* STIntelligentHelpFilter */
  &pipe_resolve_proxies,             /* STIntelligentSeriesFilter */
  &pipe_resolve_proxies,             /* STGoalReachableGuardHelpFilter */
  &pipe_resolve_proxies,             /* STGoalReachableGuardSeriesFilter */
  &pipe_resolve_proxies,             /* STIntelligentDuplicateAvoider */
  &pipe_resolve_proxies,             /* STKeepMatingGuardAttackerFilter */
  &pipe_resolve_proxies,             /* STKeepMatingGuardDefenderFilter */
  &pipe_resolve_proxies,             /* STKeepMatingGuardHelpFilter */
  &pipe_resolve_proxies,             /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &pipe_resolve_proxies,             /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &pipe_resolve_proxies,             /* STMaxTimeRootDefenderFilter */
  &pipe_resolve_proxies,             /* STMaxTimeDefenderFilter */
  &pipe_resolve_proxies,             /* STMaxTimeHelpFilter */
  &pipe_resolve_proxies,             /* STMaxTimeSeriesFilter */
  &pipe_resolve_proxies,             /* STMaxSolutionsRootSolvableFilter */
  &pipe_resolve_proxies,             /* STMaxSolutionsSolvableFilter */
  &pipe_resolve_proxies,             /* STMaxSolutionsRootDefenderFilter */
  &pipe_resolve_proxies,             /* STMaxSolutionsHelpFilter */
  &pipe_resolve_proxies,             /* STMaxSolutionsSeriesFilter */
  &pipe_resolve_proxies,             /* STStopOnShortSolutionsRootSolvableFilter */
  &pipe_resolve_proxies,             /* STStopOnShortSolutionsHelpFilter */
  &pipe_resolve_proxies,             /* STStopOnShortSolutionsSeriesFilter */
  &pipe_resolve_proxies,             /* STEndOfPhaseWriter */
  &pipe_resolve_proxies,             /* STEndOfSolutionWriter */
  &pipe_resolve_proxies,             /* STRefutationWriter */
  &pipe_resolve_proxies,             /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &pipe_resolve_proxies,             /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &pipe_resolve_proxies,             /* STOutputPlaintextLineLineWriter */
  &pipe_resolve_proxies,             /* STOutputPlaintextTreeGoalWriter */
  &pipe_resolve_proxies,             /* STOutputPlaintextTreeMoveInversionCounter */
  &pipe_resolve_proxies,             /* STOutputPlaintextLineMoveInversionCounter */
  &pipe_resolve_proxies              /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Substitute links to proxy slices by the proxy's target
 */
void resolve_proxies(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  assert(slices[root_slice].type==STProxy);
  proxy_slice_resolve(&root_slice);

  stip_structure_traversal_init(&st,&proxy_resolvers,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the min_length field of a slice.
 * @param si index of composite slice
 * @param min_length value to be set
 * @return previous value of min_length field
 */
stip_length_type set_min_length(slice_index si, stip_length_type min_length)
{
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpRoot:
      result = slices[si].u.branch.min_length;
      min_length *= 2;
      if (result%2==1)
        --min_length;
      if (min_length<=slices[si].u.branch.length)
        slices[si].u.branch.min_length = min_length;
      break;

    case STSeriesRoot:
      result = slices[si].u.branch.min_length;
      if (min_length+1<=slices[si].u.branch.length)
        slices[si].u.branch.min_length = min_length+1;
      break;

    case STAttackMove:
      result = slices[si].u.branch.min_length;
      break;

    case STSelfCheckGuardRootSolvableFilter:
    case STSelfCheckGuardHelpFilter:
    case STSelfCheckGuardSeriesFilter:
      set_min_length(slices[si].u.pipe.next,min_length);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_move(slice_index si, stip_move_traversal *st)
{
  stip_length_type * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*result;
  TraceValue("%u\n",*result);

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_move_visitor const get_max_nr_moves_functions[] =
{
  &stip_traverse_moves_children, /* STProxy */
  &get_max_nr_moves_move,        /* STAttackMove */
  &get_max_nr_moves_move,        /* STDefenseMove */
  &get_max_nr_moves_move,        /* STHelpMove */
  &stip_traverse_moves_children, /* STHelpFork */
  &get_max_nr_moves_move,        /* STSeriesMove */
  &get_max_nr_moves_move,        /* STSeriesMoveToGoal */
  &stip_traverse_moves_children, /* STSeriesNotLastMove */
  &stip_traverse_moves_children, /* STSeriesOnlyLastMove */
  &stip_traverse_moves_children, /* STSeriesFork */
  &stip_traverse_moves_children, /* STSeriesOR */
  &stip_traverse_moves_noop,     /* STGoalReachedTester */
  &stip_traverse_moves_noop,     /* STLeaf */
  &get_max_nr_moves_binary,      /* STReciprocal */
  &get_max_nr_moves_binary,      /* STQuodlibet */
  &stip_traverse_moves_children, /* STNot */
  &stip_traverse_moves_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_moves_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_moves_children, /* STMoveInverterSeriesFilter */
  &get_max_nr_moves_move,        /* STAttackRoot */
  &stip_traverse_moves_children, /* STDefenseRoot */
  &stip_traverse_moves_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_moves_children, /* STContinuationSolver */
  &stip_traverse_moves_children, /* STContinuationWriter */
  &stip_traverse_moves_children, /* STBattlePlaySolver */
  &stip_traverse_moves_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_moves_children, /* STThreatSolver */
  &stip_traverse_moves_children, /* STZugzwangWriter */
  &stip_traverse_moves_children, /* STThreatEnforcer */
  &stip_traverse_moves_children, /* STThreatCollector */
  &stip_traverse_moves_children, /* STRefutationsCollector */
  &stip_traverse_moves_children, /* STVariationWriter */
  &stip_traverse_moves_children, /* STRefutingVariationWriter */
  &stip_traverse_moves_children, /* STNoShortVariations */
  &stip_traverse_moves_children, /* STAttackHashed */
  &stip_traverse_moves_children, /* STHelpRoot */
  &stip_traverse_moves_children, /* STHelpShortcut */
  &stip_traverse_moves_children, /* STHelpHashed */
  &stip_traverse_moves_children, /* STSeriesRoot */
  &stip_traverse_moves_children, /* STSeriesShortcut */
  &stip_traverse_moves_children, /* STParryFork */
  &stip_traverse_moves_children, /* STSeriesHashed */
  &stip_traverse_moves_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_moves_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_moves_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_moves_children, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_moves_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_moves_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_moves_children, /* STDirectDefenderFilter */
  &stip_traverse_moves_children, /* STReflexRootFilter */
  &stip_traverse_moves_children, /* STReflexHelpFilter */
  &stip_traverse_moves_children, /* STReflexSeriesFilter */
  &stip_traverse_moves_children, /* STReflexAttackerFilter */
  &stip_traverse_moves_children, /* STReflexDefenderFilter */
  &stip_traverse_moves_children, /* STSelfDefense */
  &stip_traverse_moves_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_moves_children, /* STRestartGuardHelpFilter */
  &stip_traverse_moves_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_moves_children, /* STIntelligentHelpFilter */
  &stip_traverse_moves_children, /* STIntelligentSeriesFilter */
  &stip_traverse_moves_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_moves_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_moves_children, /* STIntelligentDuplicateAvoider */
  &stip_traverse_moves_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_moves_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_moves_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_moves_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_moves_children, /* STMaxFlightsquares */
  &stip_traverse_moves_children, /* STDegenerateTree */
  &stip_traverse_moves_children, /* STMaxNrNonTrivial */
  &stip_traverse_moves_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_moves_children, /* STMaxThreatLength */
  &stip_traverse_moves_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_moves_children, /* STMaxTimeDefenderFilter */
  &stip_traverse_moves_children, /* STMaxTimeHelpFilter */
  &stip_traverse_moves_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_moves_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_moves_children, /* STMaxSolutionsSolvableFilter */
  &stip_traverse_moves_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_moves_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_moves_children, /* STMaxSolutionsSeriesFilter */
  &stip_traverse_moves_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_moves_children, /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_moves_children, /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_moves_children, /* STEndOfPhaseWriter */
  &stip_traverse_moves_children, /* STEndOfSolutionWriter */
  &stip_traverse_moves_children, /* STRefutationWriter */
  &stip_traverse_moves_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_moves_children, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_moves_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_moves_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_moves_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_moves_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_moves_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Initialise a move traversal structure
 * @param st to be initialised
 * @param ops operations to be invoked on slices
 * @param param parameter to be passed t operations
 */
void stip_move_traversal_init(stip_move_traversal *st,
                              stip_move_visitors ops,
                              void *param)
{
  st->ops = ops;
  st->level = 0;
  st->remaining = 0;
  st->param = param;
}

/* (Approximately) depth-first traversl of the stipulation
 * @param root start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->remaining);

  TraceEnumerator(SliceType,slices[root].type,"\n");
  assert(slices[root].type<=nr_slice_types);

  {
    stip_move_visitor const operation = (*st->ops)[slices[root].type];
    assert(operation!=0);
    (*operation)(root,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si)
{
  stip_move_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_move_traversal_init(&st,&get_max_nr_moves_functions,&result);
  stip_traverse_moves(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_goals_equal(Goal goal1, Goal goal2)
{
  return (goal1.type==goal2.type
          && (goal1.type!=goal_target || goal1.target==goal2.target));
}

enum
{
  no_unique_goal = nr_goals+1
};

static void find_unique_goal_goal_tester(slice_index si,
                                         stip_structure_traversal *st)
{
  Goal * const found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (found->type==no_goal)
    *found = slices[si].u.goal_reached_tester.goal;
  else if (found->type!=no_unique_goal
           && !are_goals_equal(*found,slices[si].u.goal_reached_tester.goal))
    found->type = no_unique_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const unique_goal_finders[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesMoveToGoal */
  &stip_traverse_structure_children, /* STSeriesNotLastMove */
  &stip_traverse_structure_children, /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_traverse_structure_children, /* STSeriesOR */
  &find_unique_goal_goal_tester,     /* STGoalReachedTester */
  &stip_structure_visitor_noop,      /* STLeaf */
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
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
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
  &stip_traverse_structure_children, /* STIntelligentDuplicateAvoider */
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
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @param si root of subtree where to look for unique goal
 * @return no_slice if goal is not unique; index of a slice with the
 * unique goal otherwise
 */
Goal find_unique_goal(slice_index si)
{
  stip_structure_traversal st;
  Goal result = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&unique_goal_finders,&result);
  stip_traverse_structure(root_slice,&st);

  if (result.type==no_unique_goal)
    result.type = no_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result.type);
  TraceFunctionResultEnd();
  return result;
}

/* Auxiliary data structor for deep_copy: remembers slice copies
 * already made
 */
typedef slice_index copies_type[max_nr_slices];

/* Recursive implementation of in-place deep copying a stipulation
 * sub-tree
 * @param si root of sub-tree
 * @param copies address of array remembering what copies have already
 *               been made
 */
static slice_index deep_copy_recursive(slice_index si, copies_type *copies)
{
  slice_index result = (*copies)[si];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (result==no_slice)
  {
    result = copy_slice(si);
    slices[result].starter = no_side;
    (*copies)[si] = result;

    TraceEnumerator(SliceType,slices[si].type,"\n");
    switch (slices[si].type)
    {
      case STLeaf:
        /* nothing */
        break;

      case STAttackMove:
      case STDefenseMove:
      case STHelpRoot:
      case STHelpMove:
      case STSeriesMove:
      case STAttackRoot:
      case STSeriesRoot:
      case STNot:
      case STMoveInverterRootSolvableFilter:
      case STMoveInverterSolvableFilter:
      case STMoveInverterSeriesFilter:
      case STGoalReachedTester:
      case STAttackHashed:
      case STHelpHashed:
      case STSeriesHashed:
      case STSelfCheckGuardRootSolvableFilter:
      case STSelfCheckGuardSolvableFilter:
      case STSelfCheckGuardAttackerFilter:
      case STSelfCheckGuardDefenderFilter:
      case STSelfCheckGuardHelpFilter:
      case STSelfCheckGuardSeriesFilter:
      case STRestartGuardRootDefenderFilter:
      case STRestartGuardHelpFilter:
      case STRestartGuardSeriesFilter:
      case STGoalReachableGuardHelpFilter:
      case STGoalReachableGuardSeriesFilter:
      case STKeepMatingGuardAttackerFilter:
      case STKeepMatingGuardDefenderFilter:
      case STKeepMatingGuardHelpFilter:
      case STKeepMatingGuardSeriesFilter:
      case STMaxFlightsquares:
      case STMaxNrNonTrivial:
      case STMaxThreatLength:
      case STProxy:
      {
        slice_index const next = slices[si].u.pipe.next;
        slice_index const next_copy = deep_copy_recursive(next,copies);
        pipe_link(result,next_copy);
        break;
      }

      case STHelpFork:
      case STSeriesFork:
      case STSelfDefense:
      {
        slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
        slice_index const to_goal_copy = deep_copy_recursive(to_goal,copies);
        slice_index const next = slices[si].u.pipe.next;
        slice_index const next_copy = deep_copy_recursive(next,copies);
        slices[result].u.branch_fork.towards_goal = to_goal_copy;
        pipe_link(result,next_copy);
        break;
      }

      case STHelpShortcut:
      case STSeriesShortcut:
      {
        slice_index const next = slices[si].u.pipe.next;
        slice_index const next_copy = deep_copy_recursive(next,copies);
        slice_index const short_sols = slices[si].u.shortcut.short_sols;
        slice_index const short_sols_copy = deep_copy_recursive(short_sols,
                                                                copies);
        slices[result].u.shortcut.short_sols = short_sols_copy;
        pipe_link(result,next_copy);
        break;
      }

      case STQuodlibet:
      case STReciprocal:
      {
        slice_index const op1 = slices[si].u.binary.op1;
        slice_index const op1_copy = deep_copy_recursive(op1,copies);
        slice_index const op2 = slices[si].u.binary.op2;
        slice_index const op2_copy = deep_copy_recursive(op2,copies);
        slices[result].u.binary.op1 = op1_copy;
        slices[result].u.binary.op2 = op2_copy;
        break;
      }

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
slice_index stip_deep_copy(slice_index si)
{
  copies_type copies;
  slice_index i;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    copies[i] = no_slice;

  result = deep_copy_recursive(si,&copies);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void transform_to_quodlibet_self_defense(slice_index si,
                                                stip_structure_traversal *st)
{
  slice_index * const proxy_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *proxy_to_goal = stip_deep_copy(slices[si].u.branch_fork.towards_goal);

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_semi_reflex(slice_index si,
                                               stip_structure_traversal *st)
{
  slice_index * const new_proxy_to_goal = st->param;
  slice_index const proxy_to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index const not = slices[proxy_to_goal].u.pipe.next;
  slice_index const branch = slices[not].u.pipe.next;
  slice_index const tester = slices[branch].u.pipe.next;
  Goal const goal = slices[tester].u.goal_reached_tester.goal;
  slice_index new_tester;
  slice_index new_leaf;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);
  assert(slices[tester].type==STGoalReachedTester);
  assert(slices[slices[tester].u.pipe.next].type==STLeaf);

  new_leaf = alloc_leaf_slice();
  new_tester = alloc_goal_reached_tester_slice(goal);
  pipe_link(new_tester,new_leaf);

  *new_proxy_to_goal = alloc_proxy_slice();
  pipe_link(*new_proxy_to_goal,new_tester);

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_direct_defender_filter(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index const * const proxy_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    pipe_append(si,alloc_direct_defender_filter_slice(length,min_length,
                                                      *proxy_to_goal));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_branch_fork(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't recurse towards goal */
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const to_quodlibet_transformers[] =
{
  &stip_traverse_structure_children,   /* STProxy */
  &append_direct_defender_filter,      /* STAttackMove */
  &stip_traverse_structure_children,   /* STDefenseMove */
  &stip_traverse_structure_children,   /* STHelpMove */
  &transform_to_quodlibet_branch_fork, /* STHelpFork */
  &stip_traverse_structure_children,   /* STSeriesMove */
  &stip_traverse_structure_children,   /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,   /* STSeriesNotLastMove */
  &stip_traverse_structure_children,   /* STSeriesOnlyLastMove */
  &transform_to_quodlibet_branch_fork, /* STSeriesFork */
  &stip_traverse_structure_children,   /* STSeriesOR */
  &stip_structure_visitor_noop,        /* STGoalReachedTester */
  &stip_structure_visitor_noop,        /* STLeaf */
  &stip_traverse_structure_children,   /* STReciprocal */
  &stip_traverse_structure_children,   /* STQuodlibet */
  &stip_structure_visitor_noop,        /* STNot */
  &stip_traverse_structure_children,   /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,   /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,   /* STAttackRoot */
  &stip_traverse_structure_children,   /* STDefenseRoot */
  &stip_traverse_structure_children,   /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,   /* STContinuationSolver */
  &stip_traverse_structure_children,   /* STContinuationWriter */
  &stip_traverse_structure_children,   /* STBattlePlaySolver */
  &stip_traverse_structure_children,   /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,   /* STThreatSolver */
  &stip_traverse_structure_children,   /* STZugzwangWriter */
  &stip_traverse_structure_children,   /* STThreatEnforcer */
  &stip_traverse_structure_children,   /* STThreatCollector */
  &stip_traverse_structure_children,   /* STRefutationsCollector */
  &stip_traverse_structure_children,   /* STVariationWriter */
  &stip_traverse_structure_children,   /* STRefutingVariationWriter */
  &stip_traverse_structure_children,   /* STNoShortVariations */
  &stip_traverse_structure_children,   /* STAttackHashed */
  &stip_traverse_structure_children,   /* STHelpRoot */
  &stip_traverse_structure_children,   /* STHelpShortcut */
  &stip_traverse_structure_children,   /* STHelpHashed */
  &stip_traverse_structure_children,   /* STSeriesRoot */
  &stip_traverse_structure_children,   /* STSeriesShortcut */
  &stip_traverse_structure_children,   /* STParryFork */
  &stip_traverse_structure_children,   /* STSeriesHashed */
  &stip_traverse_structure_children,   /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STDirectDefenderFilter */
  &stip_traverse_structure_children,   /* STReflexRootFilter */
  &stip_traverse_structure_children,   /* STReflexHelpFilter */
  &stip_traverse_structure_children,   /* STReflexSeriesFilter */
  &stip_traverse_structure_children,   /* STReflexAttackerFilter */
  &transform_to_quodlibet_semi_reflex, /* STReflexDefenderFilter */
  &transform_to_quodlibet_self_defense,/* STSelfDefense */
  &stip_traverse_structure_children,   /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,   /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,   /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,   /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,   /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,   /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,   /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxFlightsquares */
  &stip_traverse_structure_children,   /* STDegenerateTree */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,   /* STMaxThreatLength */
  &stip_traverse_structure_children,   /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,   /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,   /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,   /* STRefutationWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,   /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,   /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children    /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal.
 * @return true iff quodlibet could be applied
 */
boolean transform_to_quodlibet(void)
{
  stip_structure_traversal st;
  slice_index proxy_to_goal = no_slice;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&to_quodlibet_transformers,&proxy_to_goal);
  stip_traverse_structure(root_slice,&st);

  result = proxy_to_goal!=no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static stip_structure_visitor const to_postkey_play_reducers[] =
{
  &stip_traverse_structure_children,              /* STProxy */
  &stip_traverse_structure_children,              /* STAttackMove */
  &defense_move_reduce_to_postkey_play,           /* STDefenseMove */
  &stip_traverse_structure_children,              /* STHelpMove */
  &stip_traverse_structure_children,              /* STHelpFork */
  &stip_traverse_structure_children,              /* STSeriesMove */
  &stip_traverse_structure_children,              /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,              /* STSeriesNotLastMove */
  &stip_traverse_structure_children,              /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children,              /* STSeriesFork */
  &stip_traverse_structure_children,              /* STSeriesOR */
  &stip_traverse_structure_children,              /* STGoalReachedTester */
  &stip_traverse_structure_children,              /* STLeaf */
  &stip_traverse_structure_children,              /* STReciprocal */
  &stip_traverse_structure_children,              /* STQuodlibet */
  &stip_traverse_structure_children,              /* STNot */
  &stip_traverse_structure_children,              /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,              /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,              /* STMoveInverterSeriesFilter */
  &attack_root_reduce_to_postkey_play,            /* STAttackRoot */
  &stip_traverse_structure_children,              /* STDefenseRoot */
  &stip_traverse_structure_children,              /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,              /* STContinuationSolver */
  &stip_traverse_structure_children,              /* STContinuationWriter */
  &stip_traverse_structure_children,              /* STBattlePlaySolver */
  &stip_traverse_structure_children,              /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,              /* STThreatSolver */
  &stip_traverse_structure_children,              /* STZugzwangWriter */
  &stip_traverse_structure_children,              /* STThreatEnforcer */
  &stip_traverse_structure_children,              /* STThreatCollector */
  &stip_traverse_structure_children,              /* STRefutationsCollector */
  &stip_traverse_structure_children,              /* STVariationWriter */
  &stip_traverse_structure_children,              /* STRefutingVariationWriter */
  &stip_traverse_structure_children,              /* STNoShortVariations */
  &stip_traverse_structure_children,              /* STAttackHashed */
  &stip_traverse_structure_children,              /* STHelpRoot */
  &stip_traverse_structure_children,              /* STHelpShortcut */
  &stip_traverse_structure_children,              /* STHelpHashed */
  &stip_traverse_structure_children,              /* STSeriesRoot */
  &stip_traverse_structure_children,              /* STSeriesShortcut */
  &stip_traverse_structure_children,              /* STParryFork */
  &stip_traverse_structure_children,              /* STSeriesHashed */
  &selfcheckguard_root_solvable_filter_reduce_to_postkey_play,              /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,              /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,              /* STSelfCheckGuardAttackerFilter */
  &selfcheckguard_defender_filter_reduce_to_postkey_play, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,              /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,              /* STSelfCheckGuardSeriesFilter */
  &direct_defender_filter_reduce_to_postkey_play, /* STDirectDefenderFilter */
  &reflex_root_filter_reduce_to_postkey_play,     /* STReflexRootFilter */
  &stip_traverse_structure_children,              /* STReflexHelpFilter */
  &stip_traverse_structure_children,              /* STReflexSeriesFilter */
  &stip_traverse_structure_children,              /* STReflexAttackerFilter */
  &reflex_defender_filter_reduce_to_postkey_play, /* STReflexDefenderFilter */
  &stip_traverse_structure_children,              /* STSelfDefense */
  &stip_traverse_structure_children,              /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,              /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,              /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,              /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,              /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,              /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,              /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,              /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,              /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,              /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,              /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,              /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,              /* STMaxFlightsquares */
  &stip_traverse_structure_children,              /* STDegenerateTree */
  &stip_traverse_structure_children,              /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,              /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,              /* STMaxThreatLength */
  &stip_traverse_structure_children,              /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,              /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,              /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,              /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,              /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,              /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,              /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,              /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,              /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,              /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,              /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,              /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,              /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,              /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,              /* STRefutationWriter */
  &stip_traverse_structure_children,              /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,              /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,              /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,              /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,              /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,              /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children               /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Install the slice representing the postkey slice at the stipulation
 * root
 * @param postkey_slice identifies slice to be installed
 */
static void install_postkey_slice(slice_index postkey_slice)
{
  slice_index const inverter = alloc_move_inverter_root_solvable_filter();
  assert(slices[root_slice].type==STProxy);
  pipe_link(inverter,postkey_slice);
  pipe_link(root_slice,inverter);
}

/* Attempt to apply the postkey play option to the current stipulation
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean stip_apply_postkeyplay(void)
{
  boolean result;
  slice_index postkey_slice = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&to_postkey_play_reducers,&postkey_slice);
  stip_traverse_structure(root_slice,&st);

  TraceValue("%u\n",postkey_slice);
  if (postkey_slice==no_slice)
    result = false;
  else
  {
    if (slices[postkey_slice].type==STProxy)
    {
      install_postkey_slice(slices[postkey_slice].u.pipe.next);
      dealloc_proxy_slice(postkey_slice);
    }
    else
      install_postkey_slice(postkey_slice);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static stip_structure_visitor const setplay_makers[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &defense_move_make_setplay_slice,  /* STDefenseMove */
  &help_move_make_setplay_slice,     /* STHelpMove */
  &stip_traverse_structure_pipe,     /* STHelpFork */
  &series_move_make_setplay_slice,   /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesMoveToGoal */
  &series_move_make_setplay_slice,   /* STSeriesNotLastMove */
  &series_move_make_setplay_slice,   /* STSeriesOnlyLastMove */
  &series_fork_make_setplay_slice,   /* STSeriesFork */
  &stip_traverse_structure_children, /* STSeriesOR */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_traverse_structure_children, /* STLeaf */
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
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_pipe,     /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_pipe,     /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &selfcheck_guard_defender_filter_make_setplay_slice, /* STSelfCheckGuardDefenderFilter */
  &selfcheck_guard_help_make_setplay_slice, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexRootFilter */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &reflex_guard_defender_filter_make_setplay_slice, /* STReflexDefenderFilter */
  &stip_traverse_structure_children, /* STSelfDefense */
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
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
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
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index stip_make_setplay(slice_index si)
{
  slice_index result = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&setplay_makers,&result);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}


static stip_structure_visitor const setplay_appliers[] =
{
  &stip_traverse_structure_children,     /* STProxy */
  &attack_move_apply_setplay,            /* STAttackMove */
  &stip_structure_visitor_noop,          /* STDefenseMove */
  &help_move_apply_setplay,              /* STHelpMove */
  &stip_traverse_structure_pipe,         /* STHelpFork */
  &series_move_apply_setplay,            /* STSeriesMove */
  &series_move_apply_setplay,            /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,     /* STSeriesNotLastMove */
  &stip_traverse_structure_children,     /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children,     /* STSeriesFork */
  &stip_traverse_structure_children,     /* STSeriesOR */
  &stip_traverse_structure_children,     /* STGoalReachedTester */
  &stip_traverse_structure_children,     /* STLeaf */
  &stip_traverse_structure_children,     /* STReciprocal */
  &stip_traverse_structure_children,     /* STQuodlibet */
  &stip_traverse_structure_children,     /* STNot */
  &stip_traverse_structure_children,     /* STMoveInverterRootSolvableFilter */
  &move_inverter_apply_setplay,          /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,     /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,     /* STAttackRoot */
  &stip_traverse_structure_children,     /* STDefenseRoot */
  &stip_traverse_structure_children,     /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,     /* STContinuationSolver */
  &stip_traverse_structure_children,     /* STContinuationWriter */
  &stip_traverse_structure_children,     /* STBattlePlaySolver */
  &stip_traverse_structure_children,     /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,     /* STThreatSolver */
  &stip_traverse_structure_children,     /* STZugzwangWriter */
  &stip_traverse_structure_children,     /* STThreatEnforcer */
  &stip_traverse_structure_children,     /* STThreatCollector */
  &stip_traverse_structure_children,     /* STRefutationsCollector */
  &stip_traverse_structure_children,     /* STVariationWriter */
  &stip_traverse_structure_children,     /* STRefutingVariationWriter */
  &stip_traverse_structure_children,     /* STNoShortVariations */
  &stip_traverse_structure_children,     /* STAttackHashed */
  &stip_traverse_structure_children,     /* STHelpRoot */
  &stip_traverse_structure_pipe,         /* STHelpShortcut */
  &stip_traverse_structure_children,     /* STHelpHashed */
  &stip_traverse_structure_children,     /* STSeriesRoot */
  &stip_traverse_structure_pipe,         /* STSeriesShortcut */
  &stip_traverse_structure_children,     /* STParryFork */
  &stip_traverse_structure_children,     /* STSeriesHashed */
  &stip_traverse_structure_children,     /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,     /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STDirectDefenderFilter */
  &stip_traverse_structure_children,     /* STReflexRootFilter */
  &stip_traverse_structure_children,     /* STReflexHelpFilter */
  &stip_traverse_structure_children,     /* STReflexSeriesFilter */
  &stip_traverse_structure_pipe,         /* STReflexAttackerFilter */
  &reflex_defender_filter_apply_setplay, /* STReflexDefenderFilter */
  &stip_traverse_structure_children,     /* STSelfDefense */
  &stip_traverse_structure_children,     /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,     /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,     /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,     /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,     /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,     /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,     /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,     /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,     /* STMaxFlightsquares */
  &stip_traverse_structure_children,     /* STDegenerateTree */
  &stip_traverse_structure_children,     /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,     /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,     /* STMaxThreatLength */
  &stip_traverse_structure_children,     /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,     /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,     /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,     /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,     /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,     /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,     /* STRefutationWriter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,     /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,     /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,     /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children      /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Combine the set play slices into the current stipulation
 * @param setplay slice index of set play
 */
static void insert_set_play(slice_index setplay_slice)
{
  slice_index mi;
  slice_index op1;
  slice_index op2;
  slice_index const next = slices[root_slice].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",setplay_slice);
  TraceFunctionParamListEnd();

  mi = alloc_move_inverter_solvable_filter();

  if (slices[setplay_slice].prev==no_slice)
    pipe_link(mi,setplay_slice);
  else
    pipe_set_successor(mi,setplay_slice);

  op1 = alloc_proxy_slice();
  pipe_link(op1,mi);

  op2 = alloc_proxy_slice();
  if (slices[next].prev==root_slice)
    pipe_link(op2,next);
  else
    pipe_set_successor(op2,next);

  pipe_unlink(root_slice);
  pipe_link(root_slice,alloc_quodlibet_slice(op1,op2));

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Attempt to add set play to the stipulation
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(void)
{
  boolean result;
  slice_index setplay_slice = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&setplay_appliers,&setplay_slice);
  stip_traverse_structure_pipe(root_slice,&st);

  if (setplay_slice==no_slice)
    result = false;
  else
  {
    insert_set_play(setplay_slice);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

typedef struct
{
    goal_type const * const goals;
    size_t const nrGoals;
    boolean * const doGoalsMatch;
} goal_set;

/* Does a leaf have one of a set of goals?
 * @param goals set of goals
 * @param nrgoal_types number of elements of goals
 * @param si leaf slice identifier
 * @return true iff the leaf has as goal one of the elements of goals.
 */
static boolean leaf_ends_in_one_of(goal_type const goals[],
                                   size_t nrgoal_types,
                                   slice_index si)
{
  goal_type const goal = slices[si].u.goal_reached_tester.goal.type;

  size_t i;
  for (i = 0; i<nrgoal_types; ++i)
    if (goal==goals[i])
      return true;

  return false;
}

static void ends_only_in(slice_index si, stip_structure_traversal *st)
{
  goal_set const * const goals = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goals->doGoalsMatch = (*goals->doGoalsMatch
                          && leaf_ends_in_one_of(goals->goals,goals->nrGoals,
                                                 si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const slice_ends_only_in_checkers[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesMoveToGoal */
  &stip_traverse_structure_children, /* STSeriesNotLastMove */
  &stip_traverse_structure_children, /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_traverse_structure_children, /* STSeriesOR */
  &ends_only_in,                     /* STGoalReachedTester */
  &stip_structure_visitor_noop,      /* STLeaf */
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
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
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
  &stip_traverse_structure_children, /* STIntelligentDuplicateAvoider */
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
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Do all leaves of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrgoal_types number of elements of goals
 * @return true iff all leaves have as goal one of the elements of goals.
 */
boolean stip_ends_only_in(goal_type const goals[], size_t nrGoals)
{
  boolean result = true; /* until traversal proves otherwise */
  goal_set set = { goals, nrGoals, &result };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nrGoals);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&slice_ends_only_in_checkers,&set);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void ends_in_one_of(slice_index si, stip_structure_traversal *st)
{
  goal_set const * const goals = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goals->doGoalsMatch = (*goals->doGoalsMatch
                          || leaf_ends_in_one_of(goals->goals,goals->nrGoals,
                                                 si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const slice_ends_in_one_of_checkers[] =
{
  &stip_traverse_structure_children,   /* STProxy */
  &stip_traverse_structure_children,   /* STAttackMove */
  &stip_traverse_structure_children,   /* STDefenseMove */
  &stip_traverse_structure_children,   /* STHelpMove */
  &stip_traverse_structure_children,   /* STHelpFork */
  &stip_traverse_structure_children,   /* STSeriesMove */
  &stip_traverse_structure_children,   /* STSeriesMoveToGoal */
  &stip_traverse_structure_children,   /* STSeriesNotLastMove */
  &stip_traverse_structure_children,   /* STSeriesOnlyLastMove */
  &stip_traverse_structure_children,   /* STSeriesFork */
  &stip_traverse_structure_children,   /* STSeriesOR */
  &ends_in_one_of,                     /* STGoalReachedTester */
  &stip_traverse_structure_children,   /* STLeaf */
  &stip_traverse_structure_children,   /* STReciprocal */
  &stip_traverse_structure_children,   /* STQuodlibet */
  &stip_traverse_structure_children,   /* STNot */
  &stip_traverse_structure_children,   /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,   /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,   /* STAttackRoot */
  &stip_traverse_structure_children,   /* STDefenseRoot */
  &stip_traverse_structure_children,   /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,   /* STContinuationSolver */
  &stip_traverse_structure_children,   /* STContinuationWriter */
  &stip_traverse_structure_children,   /* STBattlePlaySolver */
  &stip_traverse_structure_children,   /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,   /* STThreatSolver */
  &stip_traverse_structure_children,   /* STZugzwangWriter */
  &stip_traverse_structure_children,   /* STThreatEnforcer */
  &stip_traverse_structure_children,   /* STThreatCollector */
  &stip_traverse_structure_children,   /* STRefutationsCollector */
  &stip_traverse_structure_children,   /* STVariationWriter */
  &stip_traverse_structure_children,   /* STRefutingVariationWriter */
  &stip_traverse_structure_children,   /* STNoShortVariations */
  &stip_traverse_structure_children,   /* STAttackHashed */
  &stip_traverse_structure_children,   /* STHelpRoot */
  &stip_traverse_structure_children,   /* STHelpShortcut */
  &stip_traverse_structure_children,   /* STHelpHashed */
  &stip_traverse_structure_children,   /* STSeriesRoot */
  &stip_traverse_structure_children,   /* STSeriesShortcut */
  &stip_traverse_structure_children,   /* STParryFork */
  &stip_traverse_structure_children,   /* STSeriesHashed */
  &stip_traverse_structure_children,   /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STDirectDefenderFilter */
  &stip_traverse_structure_children,   /* STReflexRootFilter */
  &stip_traverse_structure_children,   /* STReflexHelpFilter */
  &stip_traverse_structure_children,   /* STReflexSeriesFilter */
  &stip_traverse_structure_children,   /* STReflexAttackerFilter */
  &stip_traverse_structure_children,   /* STReflexDefenderFilter */
  &stip_traverse_structure_children,   /* STSelfDefense */
  &stip_traverse_structure_children,   /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,   /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,   /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,   /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,   /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,   /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_children,   /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxFlightsquares */
  &stip_traverse_structure_children,   /* STDegenerateTree */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,   /* STMaxThreatLength */
  &stip_traverse_structure_children,   /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,   /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STEndOfPhaseWriter */
  &stip_traverse_structure_children,   /* STEndOfSolutionWriter */
  &stip_traverse_structure_children,   /* STRefutationWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children,   /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children,   /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children,   /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children    /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Does >= 1 leaf of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
boolean stip_ends_in_one_of(goal_type const goals[], size_t nrGoals)
{
  boolean result = false;
  goal_set set = { goals, nrGoals, &result };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nrGoals);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&slice_ends_in_one_of_checkers,&set);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param st address of structure defining traversal
 */
static void make_exact_branch(slice_index branch, stip_structure_traversal *st)
{
  slices[branch].u.branch.min_length = slices[branch].u.branch.length;

  stip_traverse_structure_children(branch,st);
}

static stip_structure_visitor const exact_makers[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &make_exact_branch,                /* STAttackMove */
  &make_exact_branch,                /* STDefenseMove */
  &make_exact_branch,                /* STHelpMove */
  &make_exact_branch,                /* STHelpFork */
  &make_exact_branch,                /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesMoveToGoal */
  &make_exact_branch,                /* STSeriesNotLastMove */
  &make_exact_branch,                /* STSeriesOnlyLastMove */
  &make_exact_branch,                /* STSeriesFork */
  &stip_traverse_structure_children, /* STSeriesOR */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_traverse_structure_children, /* STLeaf */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &make_exact_branch,                /* STAttackRoot */
  &make_exact_branch,                /* STDefenseRoot */
  &make_exact_branch,                /* STPostKeyPlaySuppressor */
  &make_exact_branch,                /* STContinuationSolver */
  &make_exact_branch,                /* STContinuationWriter */
  &make_exact_branch,                /* STBattlePlaySolver */
  &make_exact_branch,                /* STBattlePlaySolutionWriter */
  &make_exact_branch,                /* STThreatSolver */
  &make_exact_branch,                /* STZugzwangWriter */
  &make_exact_branch,                /* STThreatEnforcer */
  &make_exact_branch,                /* STThreatCollector */
  &make_exact_branch,                /* STRefutationsCollector */
  &make_exact_branch,                /* STVariationWriter */
  &make_exact_branch,                /* STRefutingVariationWriter */
  &make_exact_branch,                /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &make_exact_branch,                /* STHelpRoot */
  &make_exact_branch,                /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &make_exact_branch,                /* STSeriesRoot */
  &make_exact_branch,                /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &make_exact_branch,                /* STSelfCheckGuardRootSolvableFilter */
  &make_exact_branch,                /* STSelfCheckGuardSolvableFilter */
  &make_exact_branch,                /* STSelfCheckGuardAttackerFilter */
  &make_exact_branch,                /* STSelfCheckGuardDefenderFilter */
  &make_exact_branch,                /* STSelfCheckGuardHelpFilter */
  &make_exact_branch,                /* STSelfCheckGuardSeriesFilter */
  &make_exact_branch,                /* STDirectDefenderFilter */
  &make_exact_branch,                /* STReflexRootFilter */
  &make_exact_branch,                /* STReflexHelpFilter */
  &make_exact_branch,                /* STReflexSeriesFilter */
  &make_exact_branch,                /* STReflexAttackerFilter */
  &make_exact_branch,                /* STReflexDefenderFilter */
  &make_exact_branch,                /* STSelfDefense */
  &make_exact_branch,                /* STRestartGuardRootDefenderFilter */
  &make_exact_branch,                /* STRestartGuardHelpFilter */
  &make_exact_branch,                /* STRestartGuardSeriesFilter */
  &make_exact_branch,                /* STIntelligentHelpFilter */
  &make_exact_branch,                /* STIntelligentSeriesFilter */
  &make_exact_branch,                /* STGoalReachableGuardHelpFilter */
  &make_exact_branch,                /* STGoalReachableGuardSeriesFilter */
  &make_exact_branch,                /* STIntelligentDuplicateAvoider */
  &make_exact_branch,                /* STKeepMatingGuardAttackerFilter */
  &make_exact_branch,                /* STKeepMatingGuardDefenderFilter */
  &make_exact_branch,                /* STKeepMatingGuardHelpFilter */
  &make_exact_branch,                /* STKeepMatingGuardSeriesFilter */
  &make_exact_branch,                /* STMaxFlightsquares */
  &make_exact_branch,                /* STDegenerateTree */
  &make_exact_branch,                /* STMaxNrNonTrivial */
  &make_exact_branch,                /* STMaxNrNonTrivialCounter */
  &make_exact_branch,                /* STMaxThreatLength */
  &make_exact_branch,                /* STMaxTimeRootDefenderFilter */
  &make_exact_branch,                /* STMaxTimeDefenderFilter */
  &make_exact_branch,                /* STMaxTimeHelpFilter */
  &make_exact_branch,                /* STMaxTimeSeriesFilter */
  &make_exact_branch,                /* STMaxSolutionsRootSolvableFilter */
  &make_exact_branch,                /* STMaxSolutionsSolvableFilter */
  &make_exact_branch,                /* STMaxSolutionsRootDefenderFilter */
  &make_exact_branch,                /* STMaxSolutionsHelpFilter */
  &make_exact_branch,                /* STMaxSolutionsSeriesFilter */
  &make_exact_branch,                /* STStopOnShortSolutionsRootSolvableFilter */
  &make_exact_branch,                /* STStopOnShortSolutionsHelpFilter */
  &make_exact_branch,                /* STStopOnShortSolutionsSeriesFilter */
  &make_exact_branch,                /* STEndOfPhaseWriter */
  &make_exact_branch,                /* STEndOfSolutionWriter */
  &make_exact_branch,                /* STRefutationWriter */
  &make_exact_branch,                /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &make_exact_branch,                /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &make_exact_branch,                /* STOutputPlaintextLineLineWriter */
  &make_exact_branch,                /* STOutputPlaintextTreeGoalWriter */
  &make_exact_branch,                /* STOutputPlaintextTreeMoveInversionCounter */
  &make_exact_branch,                /* STOutputPlaintextLineMoveInversionCounter */
  &make_exact_branch                 /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Make the stipulation exact
 */
void stip_make_exact(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&exact_makers,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const starter_detectors[] =
{
  &pipe_detect_starter,              /* STProxy */
  &attack_move_detect_starter,       /* STAttackMove */
  &defense_move_detect_starter,      /* STDefenseMove */
  &help_move_detect_starter,         /* STHelpMove */
  &branch_fork_detect_starter,       /* STHelpFork */
  &series_move_detect_starter,       /* STSeriesMove */
  &series_move_detect_starter,       /* STSeriesMoveToGoal */
  &pipe_detect_starter,              /* STSeriesNotLastMove */
  &pipe_detect_starter,              /* STSeriesOnlyLastMove */
  &branch_fork_detect_starter,       /* STSeriesFork */
  &branch_fork_detect_starter,       /* STSeriesOR */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_structure_visitor_noop,      /* STLeaf */
  &reci_detect_starter,              /* STReciprocal */
  &quodlibet_detect_starter,         /* STQuodlibet */
  &pipe_detect_starter,              /* STNot */
  &move_inverter_detect_starter,     /* STMoveInverterRootSolvableFilter */
  &move_inverter_detect_starter,     /* STMoveInverterSolvableFilter */
  &move_inverter_detect_starter,     /* STMoveInverterSeriesFilter */
  &attack_move_detect_starter,       /* STAttackRoot */
  &pipe_detect_starter,              /* STDefenseRoot */
  &pipe_detect_starter,              /* STPostKeyPlaySuppressor */
  &pipe_detect_starter,              /* STContinuationSolver */
  &pipe_detect_starter,              /* STContinuationWriter */
  &pipe_detect_starter,              /* STBattlePlaySolver */
  &pipe_detect_starter,              /* STBattlePlaySolutionWriter */
  &pipe_detect_starter,              /* STThreatSolver */
  &pipe_detect_starter,              /* STZugzwangWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STThreatCollector */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &stip_traverse_structure_children, /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &pipe_detect_starter,              /* STHelpRoot */
  &pipe_detect_starter,              /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &pipe_detect_starter,              /* STSeriesRoot */
  &pipe_detect_starter,              /* STSeriesShortcut */
  &pipe_detect_starter,              /* STParryFork */
  &pipe_detect_starter,              /* STSeriesHashed */
  &pipe_detect_starter,              /* STSelfCheckGuardRootSolvableFilter */
  &pipe_detect_starter,              /* STSelfCheckGuardSolvableFilter */
  &pipe_detect_starter,              /* STSelfCheckGuardAttackerFilter */
  &pipe_detect_starter,              /* STSelfCheckGuardDefenderFilter */
  &pipe_detect_starter,              /* STSelfCheckGuardHelpFilter */
  &pipe_detect_starter,              /* STSelfCheckGuardSeriesFilter */
  &branch_fork_detect_starter,       /* STDirectDefenderFilter */
  &branch_fork_detect_starter,       /* STReflexRootFilter */
  &branch_fork_detect_starter,       /* STReflexHelpFilter */
  &branch_fork_detect_starter,       /* STReflexSeriesFilter */
  &branch_fork_detect_starter,       /* STReflexAttackerFilter */
  &branch_fork_detect_starter,       /* STReflexDefenderFilter */
  &branch_fork_detect_starter,       /* STSelfDefense */
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
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
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
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_children, /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_children, /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_children  /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Detect the starting side from the stipulation
 */
void stip_detect_starter(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&starter_detectors,NULL);
  stip_traverse_structure(root_slice,&st);

  TraceStipulation(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Impose the starting side on a stipulation
 * @param pipe identifies pipe
 * @param st address of structure that holds the state of the traversal
 */
static void default_impose_starter(slice_index si,
                                   stip_structure_traversal *st)
{
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const starter_imposers[] =
{
  &default_impose_starter,        /* STProxy */
  &pipe_impose_inverted_starter,  /* STAttackMove */
  &pipe_impose_inverted_starter,  /* STDefenseMove */
  &pipe_impose_inverted_starter,  /* STHelpMove */
  &default_impose_starter,        /* STHelpFork */
  &pipe_impose_inverted_starter,  /* STSeriesMove */
  &pipe_impose_inverted_starter,  /* STSeriesMoveToGoal */
  &default_impose_starter,        /* STSeriesNotLastMove */
  &default_impose_starter,        /* STSeriesOnlyLastMove */
  &default_impose_starter,        /* STSeriesFork */
  &default_impose_starter,        /* STSeriesOR */
  &default_impose_starter,        /* STGoalReachedTester */
  &default_impose_starter,        /* STLeaf */
  &default_impose_starter,        /* STReciprocal */
  &default_impose_starter,        /* STQuodlibet */
  &default_impose_starter,        /* STNot */
  &pipe_impose_inverted_starter,  /* STMoveInverterRootSolvableFilter */
  &pipe_impose_inverted_starter,  /* STMoveInverterSolvableFilter */
  &pipe_impose_inverted_starter,  /* STMoveInverterSeriesFilter */
  &pipe_impose_inverted_starter,  /* STAttackRoot */
  &default_impose_starter,        /* STDefenseRoot */
  &default_impose_starter,        /* STPostKeyPlaySuppressor */
  &default_impose_starter,        /* STContinuationSolver */
  &default_impose_starter,        /* STContinuationWriter */
  &default_impose_starter,        /* STBattlePlaySolver */
  &default_impose_starter,        /* STBattlePlaySolutionWriter */
  &default_impose_starter,        /* STThreatSolver */
  &default_impose_starter,        /* STZugzwangWriter */
  &default_impose_starter,        /* STThreatEnforcer */
  &default_impose_starter,        /* STThreatCollector */
  &default_impose_starter,        /* STRefutationsCollector */
  &default_impose_starter,        /* STVariationWriter */
  &default_impose_starter,        /* STRefutingVariationWriter */
  &default_impose_starter,        /* STNoShortVariations */
  &default_impose_starter,        /* STAttackHashed */
  &default_impose_starter,        /* STHelpRoot */
  &default_impose_starter,        /* STHelpShortcut */
  &default_impose_starter,        /* STHelpHashed */
  &default_impose_starter,        /* STSeriesRoot */
  &default_impose_starter,        /* STSeriesShortcut */
  &default_impose_starter,        /* STParryFork */
  &default_impose_starter,        /* STSeriesHashed */
  &default_impose_starter,        /* STSelfCheckGuardRootSolvableFilter */
  &default_impose_starter,        /* STSelfCheckGuardSolvableFilter */
  &default_impose_starter,        /* STSelfCheckGuardAttackerFilter */
  &default_impose_starter,        /* STSelfCheckGuardDefenderFilter */
  &default_impose_starter,        /* STSelfCheckGuardHelpFilter */
  &default_impose_starter,        /* STSelfCheckGuardSeriesFilter */
  &direct_defense_impose_starter, /* STDirectDefenderFilter */
  &default_impose_starter,        /* STReflexRootFilter */
  &default_impose_starter,        /* STReflexHelpFilter */
  &default_impose_starter,        /* STReflexSeriesFilter */
  &default_impose_starter,        /* STReflexAttackerFilter */
  &default_impose_starter,        /* STReflexDefenderFilter */
  &default_impose_starter,        /* STSelfDefense */
  &default_impose_starter,        /* STRestartGuardRootDefenderFilter */
  &default_impose_starter,        /* STRestartGuardHelpFilter */
  &default_impose_starter,        /* STRestartGuardSeriesFilter */
  &default_impose_starter,        /* STIntelligentHelpFilter */
  &default_impose_starter,        /* STIntelligentSeriesFilter */
  &default_impose_starter,        /* STGoalReachableGuardHelpFilter */
  &default_impose_starter,        /* STGoalReachableGuardSeriesFilter */
  &default_impose_starter,        /* STIntelligentDuplicateAvoider */
  &default_impose_starter,        /* STKeepMatingGuardAttackerFilter */
  &default_impose_starter,        /* STKeepMatingGuardDefenderFilter */
  &default_impose_starter,        /* STKeepMatingGuardHelpFilter */
  &default_impose_starter,        /* STKeepMatingGuardSeriesFilter */
  &default_impose_starter,        /* STMaxFlightsquares */
  &default_impose_starter,        /* STDegenerateTree */
  &default_impose_starter,        /* STMaxNrNonTrivial */
  &default_impose_starter,        /* STMaxNrNonTrivialCounter */
  &default_impose_starter,        /* STMaxThreatLength */
  &default_impose_starter,        /* STMaxTimeRootDefenderFilter */
  &default_impose_starter,        /* STMaxTimeDefenderFilter */
  &default_impose_starter,        /* STMaxTimeHelpFilter */
  &default_impose_starter,        /* STMaxTimeSeriesFilter */
  &default_impose_starter,        /* STMaxSolutionsRootSolvableFilter */
  &default_impose_starter,        /* STMaxSolutionsSolvableFilter */
  &default_impose_starter,        /* STMaxSolutionsRootDefenderFilter */
  &default_impose_starter,        /* STMaxSolutionsHelpFilter */
  &default_impose_starter,        /* STMaxSolutionsSeriesFilter */
  &default_impose_starter,        /* STStopOnShortSolutionsRootSolvableFilter */
  &default_impose_starter,        /* STStopOnShortSolutionsHelpFilter */
  &default_impose_starter,        /* STStopOnShortSolutionsSeriesFilter */
  &default_impose_starter,        /* STEndOfPhaseWriter */
  &default_impose_starter,        /* STEndOfSolutionWriter */
  &default_impose_starter,        /* STRefutationWriter */
  &default_impose_starter,        /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &default_impose_starter,        /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &default_impose_starter,        /* STOutputPlaintextLineLineWriter */
  &default_impose_starter,        /* STOutputPlaintextTreeGoalWriter */
  &default_impose_starter,        /* STOutputPlaintextTreeMoveInversionCounter */
  &default_impose_starter,        /* STOutputPlaintextLineMoveInversionCounter */
  &default_impose_starter         /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Set the starting side of the stipulation
 */
void stip_impose_starter(Side starter)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&starter_imposers,&starter);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
void stip_structure_visitor_noop(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Dispatch a slice structure operation to a slice based on its type
 * @param si identifies slice
 * @param ops contains addresses of visitors per slice type
 * @param st address of structure defining traversal
 */
static void stip_structure_visit_slice(slice_index si,
                                       stip_structure_visitors ops,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    stip_structure_visitor const operation = (*ops)[slices[si].type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a structure traversal structure
 * @param st to be initialised
 * @param ops operations to be invoked on slices
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init(stip_structure_traversal *st,
                                   stip_structure_visitors ops,
                                   void *param)
{
  unsigned int i;
  for (i = 0; i!=max_nr_slices; ++i)
    st->traversed[i] = slice_not_traversed;

  st->ops = ops;

  st->param = param;
}
  
/* Query the traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
stip_structure_traversal_state
get_stip_structure_traversal_state(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",st->traversed[si]);
  TraceFunctionResultEnd();
  return st->traversed[si];
}

/* (Approximately) depth-first traversal of the stipulation
 * @param ops mapping from slice types to operations
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_structure(slice_index root, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (st->traversed[root]==slice_not_traversed)
  {
    /* avoid infinite recursion */
    st->traversed[root] = slice_being_traversed;
    stip_structure_visit_slice(root,st->ops,st);
    st->traversed[root] = slice_traversed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const structure_children_traversers[] =
{
  &stip_traverse_structure_pipe,            /* STProxy */
  &stip_traverse_structure_pipe,            /* STAttackMove */
  &stip_traverse_structure_pipe,            /* STDefenseMove */
  &stip_traverse_structure_pipe,            /* STHelpMove */
  &stip_traverse_structure_help_fork,       /* STHelpFork */
  &stip_traverse_structure_pipe,            /* STSeriesMove */
  &stip_traverse_structure_pipe,            /* STSeriesMoveToGoal */
  &stip_traverse_structure_pipe,            /* STSeriesNotLastMove */
  &stip_traverse_structure_pipe,            /* STSeriesOnlyLastMove */
  &stip_traverse_structure_series_fork,     /* STSeriesFork */
  &stip_traverse_structure_series_OR,       /* STSeriesOR */
  &stip_traverse_structure_pipe,            /* STGoalReachedTester */
  &stip_structure_visitor_noop,             /* STLeaf */
  &stip_traverse_structure_binary,          /* STReciprocal */
  &stip_traverse_structure_binary,          /* STQuodlibet */
  &stip_traverse_structure_pipe,            /* STNot */
  &stip_traverse_structure_pipe,            /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_pipe,            /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_pipe,            /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_pipe,            /* STAttackRoot */
  &stip_traverse_structure_pipe,            /* STDefenseRoot */
  &stip_traverse_structure_pipe,            /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_pipe,            /* STContinuationSolver */
  &stip_traverse_structure_pipe,            /* STContinuationWriter */
  &stip_traverse_structure_pipe,            /* STBattlePlaySolver */
  &stip_traverse_structure_pipe,            /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_pipe,            /* STThreatSolver */
  &stip_traverse_structure_pipe,            /* STZugzwangWriter */
  &stip_traverse_structure_pipe,            /* STThreatEnforcer */
  &stip_traverse_structure_pipe,            /* STThreatCollector */
  &stip_traverse_structure_pipe,            /* STRefutationsCollector */
  &stip_traverse_structure_pipe,            /* STVariationWriter */
  &stip_traverse_structure_pipe,            /* STRefutingVariationWriter */
  &stip_traverse_structure_pipe,            /* STNoShortVariations */
  &stip_traverse_structure_pipe,            /* STAttackHashed */
  &stip_traverse_structure_pipe,            /* STHelpRoot */
  &stip_traverse_structure_help_shortcut,   /* STHelpShortcut */
  &stip_traverse_structure_pipe,            /* STHelpHashed */
  &stip_traverse_structure_pipe,            /* STSeriesRoot */
  &stip_traverse_structure_series_shortcut, /* STSeriesShortcut */
  &stip_traverse_structure_parry_fork,      /* STParryFork */
  &stip_traverse_structure_pipe,            /* STSeriesHashed */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_pipe,            /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_battle_fork,     /* STDirectDefenderFilter */
  &stip_traverse_structure_reflex_filter,   /* STReflexRootFilter */
  &stip_traverse_structure_reflex_filter,   /* STReflexHelpFilter */
  &stip_traverse_structure_reflex_filter,   /* STReflexSeriesFilter */
  &stip_traverse_structure_reflex_filter,   /* STReflexAttackerFilter */
  &stip_traverse_structure_reflex_filter,   /* STReflexDefenderFilter */
  &stip_traverse_structure_battle_fork,     /* STSelfDefense */
  &stip_traverse_structure_pipe,            /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_pipe,            /* STRestartGuardHelpFilter */
  &stip_traverse_structure_pipe,            /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_pipe,            /* STIntelligentHelpFilter */
  &stip_traverse_structure_pipe,            /* STIntelligentSeriesFilter */
  &stip_traverse_structure_pipe,            /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_pipe,            /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_pipe,            /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_pipe,            /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_pipe,            /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_pipe,            /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_pipe,            /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_pipe,            /* STMaxFlightsquares */
  &stip_traverse_structure_pipe,            /* STDegenerateTree */
  &stip_traverse_structure_pipe,            /* STMaxNrNonTrivial */
  &stip_traverse_structure_pipe,            /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_pipe,            /* STMaxThreatLength */
  &stip_traverse_structure_pipe,            /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_pipe,            /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_pipe,            /* STMaxTimeHelpFilter */
  &stip_traverse_structure_pipe,            /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_pipe,            /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_pipe,            /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_pipe,            /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_pipe,            /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_structure_pipe,            /* STEndOfPhaseWriter */
  &stip_traverse_structure_pipe,            /* STEndOfSolutionWriter */
  &stip_traverse_structure_pipe,            /* STRefutationWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_structure_pipe,            /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_structure_pipe             /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_visit_slice(si,&structure_children_traversers,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* No-op callback for move traversals
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_noop(slice_index si, stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Type of callback for stipulation traversals
 */
typedef void (*stip_moves_visitor)(slice_index si,
                                   struct stip_move_traversal *st);

static stip_moves_visitor const moves_children_traversers[] =
{
  &stip_traverse_moves_pipe,                  /* STProxy */
  &stip_traverse_moves_branch,                /* STAttackMove */
  &stip_traverse_moves_branch,                /* STDefenseMove */
  &stip_traverse_moves_branch,                /* STHelpMove */
  &stip_traverse_moves_help_fork,             /* STHelpFork */
  &stip_traverse_moves_branch,                /* STSeriesMove */
  &stip_traverse_moves_branch,                /* STSeriesMoveToGoal */
  &stip_traverse_moves_series_not_last_move,  /* STSeriesNotLastMove */
  &stip_traverse_moves_series_only_last_move, /* STSeriesOnlyLastMove */
  &stip_traverse_moves_series_fork,           /* STSeriesFork */
  &stip_traverse_moves_series_OR,             /* STSeriesOR */
  &stip_traverse_moves_pipe,                  /* STGoalReachedTester */
  &stip_traverse_moves_noop,                  /* STLeaf */
  &stip_traverse_moves_binary,                /* STReciprocal */
  &stip_traverse_moves_binary,                /* STQuodlibet */
  &stip_traverse_moves_pipe,                  /* STNot */
  &stip_traverse_moves_pipe,                  /* STMoveInverterRootSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STMoveInverterSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STMoveInverterSeriesFilter */
  &stip_traverse_moves_branch,                /* STAttackRoot */
  &stip_traverse_moves_branch,                /* STDefenseRoot */
  &stip_traverse_moves_pipe,                  /* STPostKeyPlaySuppressor */
  &stip_traverse_moves_pipe,                  /* STContinuationSolver */
  &stip_traverse_moves_pipe,                  /* STContinuationWriter */
  &stip_traverse_moves_pipe,                  /* STBattlePlaySolver */
  &stip_traverse_moves_pipe,                  /* STBattlePlaySolutionWriter */
  &stip_traverse_moves_pipe,                  /* STThreatSolver */
  &stip_traverse_moves_pipe,                  /* STZugzwangWriter */
  &stip_traverse_moves_pipe,                  /* STThreatEnforcer */
  &stip_traverse_moves_pipe,                  /* STThreatCollector */
  &stip_traverse_moves_pipe,                  /* STRefutationsCollector */
  &stip_traverse_moves_pipe,                  /* STVariationWriter */
  &stip_traverse_moves_pipe,                  /* STRefutingVariationWriter */
  &stip_traverse_moves_pipe,                  /* STNoShortVariations */
  &stip_traverse_moves_pipe,                  /* STAttackHashed */
  &stip_traverse_moves_help_root,             /* STHelpRoot */
  &stip_traverse_moves_help_shortcut,         /* STHelpShortcut */
  &stip_traverse_moves_pipe,                  /* STHelpHashed */
  &stip_traverse_moves_series_root,           /* STSeriesRoot */
  &stip_traverse_moves_series_shortcut,       /* STSeriesShortcut */
  &stip_traverse_moves_pipe,                  /* STParryFork */
  &stip_traverse_moves_pipe,                  /* STSeriesHashed */
  &stip_traverse_moves_pipe,                  /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_moves_pipe,                  /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STSelfCheckGuardHelpFilter */
  &stip_traverse_moves_pipe,                  /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_moves_battle_fork,           /* STDirectDefenderFilter */
  &stip_traverse_moves_reflex_root_filter,    /* STReflexRootFilter */
  &stip_traverse_moves_help_fork,             /* STReflexHelpFilter */
  &stip_traverse_moves_series_fork,           /* STReflexSeriesFilter */
  &stip_traverse_moves_reflex_attack_filter,  /* STReflexAttackerFilter */
  &stip_traverse_moves_battle_fork,           /* STReflexDefenderFilter */
  &stip_traverse_moves_battle_fork,           /* STSelfDefense */
  &stip_traverse_moves_pipe,                  /* STRestartGuardRootDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STRestartGuardHelpFilter */
  &stip_traverse_moves_pipe,                  /* STRestartGuardSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STIntelligentHelpFilter */
  &stip_traverse_moves_pipe,                  /* STIntelligentSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STGoalReachableGuardHelpFilter */
  &stip_traverse_moves_pipe,                  /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STIntelligentDuplicateAvoider */
  &stip_traverse_moves_pipe,                  /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_moves_pipe,                  /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STKeepMatingGuardHelpFilter */
  &stip_traverse_moves_pipe,                  /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STMaxFlightsquares */
  &stip_traverse_moves_pipe,                  /* STDegenerateTree */
  &stip_traverse_moves_pipe,                  /* STMaxNrNonTrivial */
  &stip_traverse_moves_pipe,                  /* STMaxNrNonTrivialCounter */
  &stip_traverse_moves_pipe,                  /* STMaxThreatLength */
  &stip_traverse_moves_pipe,                  /* STMaxTimeRootDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STMaxTimeDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STMaxTimeHelpFilter */
  &stip_traverse_moves_pipe,                  /* STMaxTimeSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STMaxSolutionsSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STMaxSolutionsHelpFilter */
  &stip_traverse_moves_pipe,                  /* STMaxSolutionsSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_moves_pipe,                  /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_moves_pipe,                  /* STStopOnShortSolutionsSeriesFilter */
  &stip_traverse_moves_pipe,                  /* STEndOfPhaseWriter */
  &stip_traverse_moves_pipe,                  /* STEndOfSolutionWriter */
  &stip_traverse_moves_pipe,                  /* STRefutationWriter */
  &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &stip_traverse_moves_pipe,                  /* STOutputPlaintextLineLineWriter */
  &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_moves_pipe,                  /* STOutputPlaintextTreeMoveInversionCounter */
  &stip_traverse_moves_pipe,                  /* STOutputPlaintextLineMoveInversionCounter */
  &stip_traverse_moves_pipe                   /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};


/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    SliceType const type = slices[si].type;
    stip_moves_visitor const operation = moves_children_traversers[type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
