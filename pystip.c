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
#include "stipulation/battle_play/defense_fork.h"
#include "stipulation/battle_play/defense_end.h"
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/attack_end.h"
#include "stipulation/battle_play/attack_fork.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/move_to_goal.h"
#include "stipulation/help_play/shortcut.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/move_to_goal.h"
#include "stipulation/series_play/shortcut.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/parry_fork.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME SliceType
#define ENUMERATORS \
  ENUMERATOR(STProxy),                                                  \
    ENUMERATOR(STAttackMove),                                           \
    ENUMERATOR(STAttackMoveToGoal),                                     \
    ENUMERATOR(STDefenseMove),                                          \
    ENUMERATOR(STDefenseMoveAgainstGoal),                               \
    ENUMERATOR(STHelpMove),      /* M-N moves of help play */           \
    ENUMERATOR(STHelpMoveToGoal),  /* last help move reaching goal */   \
    ENUMERATOR(STHelpFork),        /* decides when play in branch is over */ \
    ENUMERATOR(STSeriesMove),    /* M-N moves of series play */         \
    ENUMERATOR(STSeriesMoveToGoal),    /* last series move reaching goal */ \
    ENUMERATOR(STSeriesFork),      /* decides when play in branch is over */ \
    ENUMERATOR(STDoubleMateAttackerFilter),  /* enforces precondition for doublemate */ \
    ENUMERATOR(STEnPassantAttackerFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STCastlingAttackerFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STCounterMateAttackerFilter),  /* enforces precondition for counter-mate */ \
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
    ENUMERATOR(STAttackEnd),      /* battle play, half-moves used up */ \
    ENUMERATOR(STAttackFork),      /* battle play, continue with subsequent branch */ \
    ENUMERATOR(STDefenseEnd),     /* battle play, half-moves used up */ \
    ENUMERATOR(STDefenseFork),     /* battle play, continue with subsequent branch */ \
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
    ENUMERATOR(STMaxNrNonChecks), /* deals with option NonTrivial */   \
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


/* Keep track of allocated slice indices
 */
static boolean is_slice_index_free[max_nr_slices];


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
  slice_structure_branch, /* STAttackMoveToGoal */
  slice_structure_branch, /* STDefenseMove */
  slice_structure_branch, /* STDefenseMoveAgainstGoal */
  slice_structure_branch, /* STHelpMove */
  slice_structure_branch, /* STHelpMoveToGoal */
  slice_structure_fork,   /* STHelpFork */
  slice_structure_branch, /* STSeriesMove */
  slice_structure_branch, /* STSeriesMoveToGoal */
  slice_structure_fork,   /* STSeriesFork */
  slice_structure_pipe,   /* STDoubleMateAttackerFilter */
  slice_structure_pipe,   /* STEnPassantAttackerFilter */
  slice_structure_branch, /* STCastlingAttackerFilter */
  slice_structure_pipe,   /* STCounterMateAttackerFilter */
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
  slice_structure_fork,   /* STHelpShortcut */
  slice_structure_branch, /* STHelpHashed */
  slice_structure_branch, /* STSeriesRoot */
  slice_structure_fork,   /* STSeriesShortcut */
  slice_structure_fork,   /* STParryFork */
  slice_structure_branch, /* STSeriesHashed */
  slice_structure_pipe,   /* STSelfCheckGuardRootSolvableFilter */
  slice_structure_pipe,   /* STSelfCheckGuardSolvableFilter */
  slice_structure_branch, /* STSelfCheckGuardAttackerFilter */
  slice_structure_branch, /* STSelfCheckGuardDefenderFilter */
  slice_structure_branch, /* STSelfCheckGuardHelpFilter */
  slice_structure_branch, /* STSelfCheckGuardSeriesFilter */
  slice_structure_fork,   /* STDirectDefenderFilter */
  slice_structure_fork,   /* STReflexRootFilter */
  slice_structure_fork,   /* STReflexHelpFilter */
  slice_structure_fork,   /* STReflexSeriesFilter */
  slice_structure_fork,   /* STReflexAttackerFilter */
  slice_structure_fork,   /* STReflexDefenderFilter */
  slice_structure_fork,   /* STSelfDefense */
  slice_structure_pipe,   /* STAttackEnd */
  slice_structure_fork,   /* STAttackFork */
  slice_structure_pipe,   /* STDefenseEnd */
  slice_structure_fork,   /* STDefenseFork */
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
  slice_structure_branch, /* STMaxNrNonChecks */
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

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  if (root_slice!=no_slice)
  {
    slice_index i;
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0,0,0);
    stip_traverse_structure(root_slice,&st);

    for (i = 0; i!=max_nr_slices; ++i)
      if (!is_slice_index_free[i])
      {
        if (st.traversed[i]!=slice_traversed)
        { /* Trace* expand to nothing unless DOTRACE is #defined */
          TraceValue("leaked:%u",i);
          TraceEnumerator(SliceType,slices[i].type,"\n");
        }
        assert(st.traversed[i]==slice_traversed);
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

  TraceEnumerator(Side,slices[original].starter,"");
  TraceEnumerator(Side,slices[result].starter,"\n");

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

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si)
{
  slice_index i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  
  stip_structure_traversal_init(&st,0,0,0);
  stip_traverse_structure(si,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    if (st.traversed[i]==slice_traversed)
    {
      if (slices[i].type==STProxy)
        dealloc_proxy_slice(i);
      else
        dealloc_slice(i);
    }

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

static structure_traversers_visitors const root_slice_makers[] =
{
  { STProxy,                       &proxy_make_root                          },
  { STAttackMove,                  &attack_move_make_root                    },
  { STDefenseMove,                 &defense_move_make_root                   },
  { STGoalReachedTester,           &goal_reached_tester_make_root            },
  { STLeaf,                        &leaf_make_root                           },
  { STQuodlibet,                   &quodlibet_make_root                      },
  { STNot,                         &not_make_root                            },
  { STSelfCheckGuardAttackerFilter,&selfcheck_guard_attacker_filter_make_root},
  { STSelfCheckGuardDefenderFilter,&selfcheck_guard_defender_filter_make_root},
  { STDirectDefenderFilter,        &direct_defender_filter_make_root         },
  { STReflexAttackerFilter,        &reflex_attacker_filter_make_root         },
  { STReflexDefenderFilter,        &reflex_defender_filter_make_root         },
  { STSelfDefense,                 &self_defense_make_root                   },
  { STDefenseEnd,                  &defense_end_make_root                    }
};

enum
{
  nr_root_slice_makers = (sizeof root_slice_makers
                          / sizeof root_slice_makers[0])
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

  stip_structure_traversal_init(&st,
                                root_slice_makers,nr_root_slice_makers,
                                &result);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten the branches by the move that is represented by the root
 * slices
 */
void battle_branch_post_root_shorten_end(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  battle_branch_shorten_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

static structure_traversers_visitors post_root_shorteners[] =
{
  { STAttackMove,                   &battle_branch_post_root_shorten,     },
  { STDefenseMove,                  &battle_branch_post_root_shorten_end, },
  { STSelfCheckGuardAttackerFilter, &battle_branch_post_root_shorten,     },
  { STSelfCheckGuardDefenderFilter, &battle_branch_post_root_shorten,     },
  { STDirectDefenderFilter,         &battle_branch_post_root_shorten,     },
  { STReflexAttackerFilter,         &battle_branch_post_root_shorten,     },
  { STReflexDefenderFilter,         &battle_branch_post_root_shorten,     },
  { STAttackEnd,                    &battle_branch_post_root_shorten,     },
  { STAttackFork,                   &battle_branch_post_root_shorten,     },
  { STDefenseEnd,                   &battle_branch_post_root_shorten,     },
  { STDefenseFork,                  &battle_branch_post_root_shorten,     }
};

enum
{
  nr_post_root_shorteners = (sizeof post_root_shorteners
                             / sizeof post_root_shorteners[0])
};

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

  stip_structure_traversal_init(&st_nested,
                                post_root_shorteners,nr_post_root_shorteners,
                                0);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors root_slice_inserters[] =
{
  { STProxy,                        &proxy_make_root                  },
  { STAttackMove,                   &battle_branch_make_root          },
  { STDefenseMove,                  &battle_branch_make_root          },
  { STHelpMove,                     &help_move_make_root              },
  { STHelpMoveToGoal,               &help_move_to_goal_make_root      },
  { STHelpFork,                     &help_fork_make_root              },
  { STSeriesMove,                   &series_move_make_root            },
  { STSeriesMoveToGoal,             &series_move_to_goal_make_root    },
  { STSeriesFork,                   &series_fork_make_root            },
  { STGoalReachedTester,            &goal_reached_tester_make_root    },
  { STLeaf,                         &leaf_make_root                   },
  { STReciprocal,                   &reci_make_root                   },
  { STQuodlibet,                    &quodlibet_make_root              },
  { STNot,                          &not_make_root                    },
  { STMoveInverterSolvableFilter,   &move_inverter_make_root          },
  { STSelfCheckGuardAttackerFilter, &battle_branch_make_root          },
  { STSelfCheckGuardDefenderFilter, &battle_branch_make_root          },
  { STSelfCheckGuardHelpFilter,     &selfcheck_guard_help_make_root   },
  { STSelfCheckGuardSeriesFilter,   &selfcheck_guard_series_make_root },
  { STDirectDefenderFilter,         &battle_branch_make_root          },
  { STReflexHelpFilter,             &reflex_help_filter_make_root     },
  { STReflexSeriesFilter,           &reflex_series_filter_make_root   },
  { STReflexAttackerFilter,         &battle_branch_make_root          },
  { STReflexDefenderFilter,         &battle_branch_make_root          },
  { STSelfDefense,                  &battle_branch_make_root          }
};

enum
{
  nr_root_slice_inserters = (sizeof root_slice_inserters
                             / sizeof root_slice_inserters[0])
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

  stip_structure_traversal_init(&st,
                                root_slice_inserters,nr_root_slice_inserters,
                                &result);
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

/* Substitute links to proxy slices by the proxy's target
 */
void resolve_proxies(void)
{
  slice_index si;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  assert(slices[root_slice].type==STProxy);
  proxy_slice_resolve(&root_slice);

  for (si = 0; si!=max_nr_slices; ++si)
    if (!is_slice_index_free[si])
      switch (highest_structural_type[slices[si].type])
      {
        case slice_structure_leaf:
          /* intentionally nothing */
          break;

        case slice_structure_pipe:
        case slice_structure_branch:
          pipe_resolve_proxies(si);
          break;

        case slice_structure_fork:
          branch_fork_resolve_proxies(si);
          break;

        case slice_structure_binary:
          binary_resolve_proxies(si);
          break;

        default:
          assert(0);
          break;
      }

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
static void get_max_nr_moves_move(slice_index si, stip_moves_traversal *st)
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

static moves_traversers_visitors const get_max_nr_moves_functions[] =
{
  { STAttackRoot,             &get_max_nr_moves_move   },
  { STAttackMoveToGoal,       &get_max_nr_moves_move   },
  { STAttackMove,             &get_max_nr_moves_move   },
  { STDefenseMove,            &get_max_nr_moves_move   },
  { STDefenseMoveAgainstGoal, &get_max_nr_moves_move   },
  { STHelpMove,               &get_max_nr_moves_move   },
  { STHelpMoveToGoal,         &get_max_nr_moves_move   },
  { STSeriesMove,             &get_max_nr_moves_move   },
  { STSeriesMoveToGoal,       &get_max_nr_moves_move   },
  { STQuodlibet,              &get_max_nr_moves_binary },
  { STReciprocal,             &get_max_nr_moves_binary }
};

enum
{
  nr_get_max_nr_moves_functions = (sizeof get_max_nr_moves_functions
                                   / sizeof get_max_nr_moves_functions[0])
};

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si)
{
  stip_moves_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_moves_traversal_init(&st,
                           get_max_nr_moves_functions,
                           nr_get_max_nr_moves_functions,
                           &result);
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

static structure_traversers_visitors unique_goal_finders[] =
{
  { STGoalReachedTester, &find_unique_goal_goal_tester }
};

enum
{
  nr_unique_goal_finders = (sizeof unique_goal_finders
                          / sizeof unique_goal_finders[0])
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

  stip_structure_traversal_init(&st,
                                unique_goal_finders,nr_unique_goal_finders,
                                &result);
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
      case STDefenseEnd:
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
      case STVariationWriter:
      case STOutputPlaintextTreeGoalWriter:
      {
        slice_index const next = slices[si].u.pipe.next;
        slice_index const next_copy = deep_copy_recursive(next,copies);
        pipe_link(result,next_copy);
        break;
      }

      case STHelpFork:
      case STSeriesFork:
      case STSelfDefense:
      case STDefenseFork:
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
    pipe_append(si,alloc_direct_defender_filter_slice(length-1,min_length-1,
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

static structure_traversers_visitors to_quodlibet_transformers[] =
{
  { STAttackMove,             &append_direct_defender_filter       },
  { STHelpFork,               &transform_to_quodlibet_branch_fork  },
  { STSeriesFork,             &transform_to_quodlibet_branch_fork  },
  { STNot,                    &stip_structure_visitor_noop         },
  { STReflexDefenderFilter,   &transform_to_quodlibet_semi_reflex  },
  { STSelfDefense,            &transform_to_quodlibet_self_defense }
};

enum
{
  nr_to_quodlibet_transformers = (sizeof to_quodlibet_transformers
                                  / sizeof to_quodlibet_transformers[0])
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

  stip_structure_traversal_init(&st,
                                to_quodlibet_transformers,
                                nr_to_quodlibet_transformers,
                                &proxy_to_goal);
  stip_traverse_structure(root_slice,&st);

  result = proxy_to_goal!=no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static structure_traversers_visitors to_postkey_play_reducers[] =
{
  { STDefenseMove,                      &defense_move_reduce_to_postkey_play                        },
  { STAttackRoot,                       &attack_root_reduce_to_postkey_play                         },
  { STSelfCheckGuardRootSolvableFilter, &selfcheckguard_root_solvable_filter_reduce_to_postkey_play },
  { STSelfCheckGuardDefenderFilter,     &selfcheckguard_defender_filter_reduce_to_postkey_play      },
  { STDirectDefenderFilter,             &direct_defender_filter_reduce_to_postkey_play              },
  { STReflexRootFilter,                 &reflex_root_filter_reduce_to_postkey_play                  },
  { STReflexDefenderFilter,             &reflex_defender_filter_reduce_to_postkey_play              },
  { STAttackEnd,                        &attack_end_reduce_to_postkey_play                          },
  { STDefenseEnd,                       &defense_end_reduce_to_postkey_play                         }
};

enum
{
  nr_to_postkey_play_reducers = (sizeof to_postkey_play_reducers
                                 / sizeof to_postkey_play_reducers[0])
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

  stip_structure_traversal_init(&st,
                                to_postkey_play_reducers,
                                nr_to_postkey_play_reducers,
                                &postkey_slice);
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

static structure_traversers_visitors setplay_makers[] =
{
  { STDefenseMove,                  &defense_move_make_setplay_slice                    },
  { STHelpMove,                     &help_move_make_setplay_slice                       },
  { STHelpMoveToGoal,               &help_move_make_setplay_slice                       },
  { STHelpFork,                     &help_fork_make_setplay_slice                       },
  { STSeriesMove,                   &series_move_make_setplay_slice                     },
  { STSeriesFork,                   &series_fork_make_setplay_slice                     },
  { STHelpShortcut,                 &stip_traverse_structure_pipe                       },
  { STSeriesShortcut,               &stip_traverse_structure_pipe                       },
  { STSelfCheckGuardDefenderFilter, &selfcheck_guard_defender_filter_make_setplay_slice },
  { STSelfCheckGuardHelpFilter,     &selfcheck_guard_help_make_setplay_slice            },
  { STReflexDefenderFilter,         &reflex_guard_defender_filter_make_setplay_slice    }
};

enum
{
  nr_setplay_makers = (sizeof setplay_makers
                          / sizeof setplay_makers[0])
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

  stip_structure_traversal_init(&st,setplay_makers,nr_setplay_makers,&result);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static structure_traversers_visitors setplay_appliers[] =
{
  { STAttackMove,                 &attack_move_apply_setplay            },
  { STDefenseMove,                &stip_structure_visitor_noop          },
  { STHelpMove,                   &help_move_apply_setplay              },
  { STHelpFork,                   &stip_traverse_structure_pipe         },
  { STSeriesMove,                 &series_move_apply_setplay            },
  { STSeriesMoveToGoal,           &series_move_apply_setplay            },
  { STMoveInverterSolvableFilter, &move_inverter_apply_setplay          },
  { STHelpShortcut,               &stip_traverse_structure_pipe         },
  { STSeriesShortcut,             &stip_traverse_structure_pipe         },
  { STReflexAttackerFilter,       &stip_traverse_structure_pipe         },
  { STReflexDefenderFilter,       &reflex_defender_filter_apply_setplay }
};

enum
{
  nr_setplay_appliers = (sizeof setplay_appliers / sizeof setplay_appliers[0])
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

  stip_structure_traversal_init(&st,
                                setplay_appliers,nr_setplay_appliers,
                                &setplay_slice);
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

static structure_traversers_visitors slice_ends_only_in_checkers[] =
{
  { STGoalReachedTester, &ends_only_in }
};

enum
{
  nr_slice_ends_only_in_checkers = (sizeof slice_ends_only_in_checkers
                                    / sizeof slice_ends_only_in_checkers[0])
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

  stip_structure_traversal_init(&st,
                                slice_ends_only_in_checkers,
                                nr_slice_ends_only_in_checkers,
                                &set);
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

static structure_traversers_visitors slice_ends_in_one_of_checkers[] =
{
  { STGoalReachedTester, &ends_in_one_of }
};

enum
{
  nr_slice_ends_in_one_of_checkers
  = (sizeof slice_ends_in_one_of_checkers
     / sizeof slice_ends_in_one_of_checkers[0])
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

  stip_structure_traversal_init(&st,
                                slice_ends_in_one_of_checkers,
                                nr_slice_ends_in_one_of_checkers,
                                &set);
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
static void make_exact_battle_branch(slice_index branch,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  slices[branch].u.branch.min_length = slices[branch].u.branch.length-1;

  stip_traverse_structure_children(branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param st address of structure defining traversal
 */
static void make_exact_help_branch(slice_index branch,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  slices[branch].u.branch.min_length = slices[branch].u.branch.length-1;

  stip_traverse_structure_children(branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param st address of structure defining traversal
 */
static void make_exact_series_branch(slice_index branch,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  slices[branch].u.branch.min_length = slices[branch].u.branch.length;

  stip_traverse_structure_children(branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors exact_makers[] =
{
  { STAttackMove,                       &make_exact_battle_branch },
  { STDefenseMove,                      &make_exact_battle_branch },
  { STHelpMove,                         &make_exact_help_branch   },
  { STHelpFork,                         &make_exact_help_branch   },
  { STSeriesMove,                       &make_exact_series_branch },
  { STSeriesFork,                       &make_exact_series_branch },
  { STSelfCheckGuardAttackerFilter,     &make_exact_battle_branch },
  { STSelfCheckGuardDefenderFilter,     &make_exact_battle_branch },
  { STSelfCheckGuardHelpFilter,         &make_exact_help_branch   },
  { STSelfCheckGuardSeriesFilter,       &make_exact_series_branch },
  { STDirectDefenderFilter,             &make_exact_battle_branch },
  { STReflexHelpFilter,                 &make_exact_help_branch   },
  { STReflexSeriesFilter,               &make_exact_series_branch },
  { STReflexAttackerFilter,             &make_exact_battle_branch },
  { STReflexDefenderFilter,             &make_exact_battle_branch },
  { STSelfDefense,                      &make_exact_battle_branch },
  { STAttackEnd,                        &make_exact_battle_branch },
  { STDefenseEnd,                       &make_exact_battle_branch }
};

enum
{
  nr_exact_makers = sizeof exact_makers / sizeof exact_makers[0]
};

/* Make the stipulation exact
 */
void stip_make_exact(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,exact_makers,nr_exact_makers,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors starter_detectors[] =
{
  { STProxy,                            &pipe_detect_starter          },
  { STAttackMove,                       &attack_move_detect_starter   },
  { STDefenseMove,                      &defense_move_detect_starter  },
  { STHelpMove,                         &help_move_detect_starter     },
  { STHelpMoveToGoal,                   &help_move_detect_starter     },
  { STHelpFork,                         &branch_fork_detect_starter   },
  { STSeriesMove,                       &series_move_detect_starter   },
  { STSeriesMoveToGoal,                 &series_move_detect_starter   },
  { STSeriesFork,                       &branch_fork_detect_starter   },
  { STReciprocal,                       &reci_detect_starter          },
  { STQuodlibet,                        &quodlibet_detect_starter     },
  { STNot,                              &pipe_detect_starter          },
  { STMoveInverterRootSolvableFilter,   &move_inverter_detect_starter },
  { STMoveInverterSolvableFilter,       &move_inverter_detect_starter },
  { STMoveInverterSeriesFilter,         &move_inverter_detect_starter },
  { STAttackRoot,                       &attack_move_detect_starter   },
  { STDefenseRoot,                      &pipe_detect_starter          },
  { STHelpRoot,                         &pipe_detect_starter          },
  { STHelpShortcut,                     &pipe_detect_starter          },
  { STSeriesRoot,                       &pipe_detect_starter          },
  { STSeriesShortcut,                   &pipe_detect_starter          },
  { STParryFork,                        &pipe_detect_starter          },
  { STSeriesHashed,                     &pipe_detect_starter          },
  { STSelfCheckGuardRootSolvableFilter, &pipe_detect_starter          },
  { STSelfCheckGuardSolvableFilter,     &pipe_detect_starter          },
  { STSelfCheckGuardAttackerFilter,     &pipe_detect_starter          },
  { STSelfCheckGuardDefenderFilter,     &pipe_detect_starter          },
  { STSelfCheckGuardHelpFilter,         &pipe_detect_starter          },
  { STSelfCheckGuardSeriesFilter,       &pipe_detect_starter          },
  { STDirectDefenderFilter,             &branch_fork_detect_starter   },
  { STReflexRootFilter,                 &branch_fork_detect_starter   },
  { STReflexHelpFilter,                 &branch_fork_detect_starter   },
  { STReflexSeriesFilter,               &branch_fork_detect_starter   },
  { STReflexAttackerFilter,             &branch_fork_detect_starter   },
  { STReflexDefenderFilter,             &branch_fork_detect_starter   },
  { STSelfDefense,                      &branch_fork_detect_starter   },
  { STAttackEnd,                        &pipe_detect_starter          },
  { STDefenseEnd,                       &pipe_detect_starter          }
};

enum
{
  nr_starter_detectors = (sizeof starter_detectors
                          / sizeof starter_detectors[0])
};

/* Detect the starting side from the stipulation
 */
void stip_detect_starter(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,
                                starter_detectors,nr_starter_detectors,
                                NULL);
  stip_traverse_structure(root_slice,&st);

  TraceStipulation(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Impose the starting side on a stipulation
 * @param si identifies slice
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

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
static void impose_inverted_starter(slice_index si,
                                    stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;

  *starter = advers(*starter);
  stip_traverse_structure_children(si,st);
  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice types that change the starting side
 */
static SliceType starter_inverters[] =
{
  STAttackRoot,
  STAttackMove,
  STAttackMoveToGoal,
  STDefenseMove,
  STDefenseMoveAgainstGoal,
  STHelpMove,
  STHelpMoveToGoal,
  STSeriesMove,
  STSeriesMoveToGoal,
  STMoveInverterRootSolvableFilter,
  STMoveInverterSolvableFilter,
  STMoveInverterSeriesFilter
};

enum
{
  nr_starter_inverters = (sizeof starter_inverters
                          / sizeof starter_inverters[0])
};

/* Set the starting side of the stipulation
 * @param starter starting side at the root of the stipulation
 */
void stip_impose_starter(Side starter)
{
  stip_structure_traversal st;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    st.traversed[i] = slice_not_traversed;

  for (i = 0; i!=nr_slice_types; ++i)
    st.map.visitors[i] = &default_impose_starter;

  for (i = 0; i!=nr_starter_inverters; ++i)
    st.map.visitors[starter_inverters[i]] = &impose_inverted_starter;

  st.param = &starter;

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
static
void stip_structure_visit_slice(slice_index si,
                                stip_structure_visitor (*ops)[nr_slice_types],
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

  if (root!=no_slice)
    if (st->traversed[root]==slice_not_traversed)
    {
      /* avoid infinite recursion */
      st->traversed[root] = slice_being_traversed;
      stip_structure_visit_slice(root,&st->map.visitors,st);
      st->traversed[root] = slice_traversed;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor structure_children_traversers[] =
{
  &stip_traverse_structure_pipe,            /* STProxy */
  &stip_traverse_structure_pipe,            /* STAttackMove */
  &stip_traverse_structure_pipe,            /* STAttackMoveToGoal */
  &stip_traverse_structure_pipe,            /* STDefenseMove */
  &stip_traverse_structure_pipe,            /* STDefenseMoveAgainstGoal */
  &stip_traverse_structure_pipe,            /* STHelpMove */
  &stip_traverse_structure_pipe,            /* STHelpMoveToGoal */
  &stip_traverse_structure_help_fork,       /* STHelpFork */
  &stip_traverse_structure_pipe,            /* STSeriesMove */
  &stip_traverse_structure_pipe,            /* STSeriesMoveToGoal */
  &stip_traverse_structure_series_fork,     /* STSeriesFork */
  &stip_traverse_structure_pipe,            /* STDoubleMateAttackerFilter */
  &stip_traverse_structure_pipe,            /* STEnPassantAttackerFilter */
  &stip_traverse_structure_pipe,            /* STCastlingAttackerFilter */
  &stip_traverse_structure_pipe,            /* STCounterMateAttackerFilter */
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
  &stip_traverse_structure_pipe,            /* STAttackEnd */
  &stip_traverse_structure_battle_fork,     /* STAttackFork */
  &stip_traverse_structure_pipe,            /* STDefenseEnd */
  &stip_traverse_structure_battle_fork,     /* STDefenseFork */
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
  &stip_traverse_structure_pipe,            /* STMaxNrNonChecks */
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

void
stip_structure_traversal_init(stip_structure_traversal *st,
                              structure_traversers_visitors const visitors[],
                              unsigned int nr_visitors,
                              void *param)
{
  unsigned int i;
  for (i = 0; i!=max_nr_slices; ++i)
    st->traversed[i] = slice_not_traversed;

  for (i = 0; i!=nr_slice_types; ++i)
    st->map.visitors[i] = structure_children_traversers[i];

  for (i = 0; i!=nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;

  st->param = param;
}

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

static moves_visitor_map_type const moves_children_traversers =
{
  {
    &stip_traverse_moves_pipe,                  /* STProxy */
    &stip_traverse_moves_branch_slice,          /* STAttackMove */
    &stip_traverse_moves_branch_slice,          /* STAttackMoveToGoal */
    &stip_traverse_moves_branch_slice,          /* STDefenseMove */
    &stip_traverse_moves_branch_slice,          /* STDefenseMoveAgainstGoal */
    &stip_traverse_moves_branch_slice,          /* STHelpMove */
    &stip_traverse_moves_branch_slice,          /* STHelpMoveToGoal */
    &stip_traverse_moves_help_fork,             /* STHelpFork */
    &stip_traverse_moves_branch_slice,          /* STSeriesMove */
    &stip_traverse_moves_branch_slice,          /* STSeriesMoveToGoal */
    &stip_traverse_moves_series_fork,           /* STSeriesFork */
    &stip_traverse_moves_pipe,                  /* STDoubleMateAttackerFilter */
    &stip_traverse_moves_pipe,                  /* STEnPassantAttackerFilter */
    &stip_traverse_moves_pipe,                  /* STCastlingAttackerFilter */
    &stip_traverse_moves_pipe,                  /* STCounterMateAttackerFilter */
    &stip_traverse_moves_pipe,                  /* STGoalReachedTester */
    &stip_traverse_moves_noop,                  /* STLeaf */
    &stip_traverse_moves_binary,                /* STReciprocal */
    &stip_traverse_moves_binary,                /* STQuodlibet */
    &stip_traverse_moves_pipe,                  /* STNot */
    &stip_traverse_moves_pipe,                  /* STMoveInverterRootSolvableFilter */
    &stip_traverse_moves_pipe,                  /* STMoveInverterSolvableFilter */
    &stip_traverse_moves_pipe,                  /* STMoveInverterSeriesFilter */
    &stip_traverse_moves_branch_slice,          /* STAttackRoot */
    &stip_traverse_moves_pipe,                  /* STDefenseRoot */
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
    &stip_traverse_moves_direct_defender_filter,/* STDirectDefenderFilter */
    &stip_traverse_moves_reflex_root_filter,    /* STReflexRootFilter */
    &stip_traverse_moves_help_fork,             /* STReflexHelpFilter */
    &stip_traverse_moves_reflex_series_filter,  /* STReflexSeriesFilter */
    &stip_traverse_moves_reflex_attack_filter,  /* STReflexAttackerFilter */
    &stip_traverse_moves_battle_fork,           /* STReflexDefenderFilter */
    &stip_traverse_moves_battle_fork,           /* STSelfDefense */
    &stip_traverse_moves_attack_end,            /* STAttackEnd */
    &stip_traverse_moves_attack_fork,           /* STAttackFork */
    &stip_traverse_moves_defense_end,           /* STDefenseEnd */
    &stip_traverse_moves_defense_fork,          /* STDefenseFork */
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
    &stip_traverse_moves_pipe,                  /* STMaxNrNonChecks */
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
  }
};

/* Initialise a move traversal structure
 * @param st to be initialised
 * @param moves_traversers_visitors array of alternative visitors; for
 *                                  slices with types not mentioned in
 *                                  moves_traversers_visitors, the default
 *                                  visitor will be used
 * @param nr_visitors length of moves_traversers_visitors
 * @param param parameter to be passed t operations
 */
void stip_moves_traversal_init(stip_moves_traversal *st,
                               moves_traversers_visitors const visitors[],
                               unsigned int nr_visitors,
                               void *param)
{
  unsigned int i;
  
  st->map = moves_children_traversers;

  for (i = 0; i<nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;

  st->level = 0;
  st->remaining = 0;
  st->param = param;
}

/* (Approximately) depth-first traversl of the stipulation
 * @param root start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->remaining);

  TraceEnumerator(SliceType,slices[root].type,"\n");
  assert(slices[root].type<=nr_slice_types);

  {
    stip_moves_visitor const operation = st->map.visitors[slices[root].type];
    assert(operation!=0);
    (*operation)(root,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* No-op callback for move traversals
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_noop(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    SliceType const type = slices[si].type;
    stip_moves_visitor const operation = moves_children_traversers.visitors[type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
