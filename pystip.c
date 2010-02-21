#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/help_play/branch.h"
#include "pyleaf.h"
#include "pyleafd.h"
#include "pyleafh.h"
#include "pyleaff.h"
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
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/defense_root.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/shortcut.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/shortcut.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME SliceType
#define ENUMERATORS \
  ENUMERATOR(STProxy),                                                  \
                                                                        \
    ENUMERATOR(STAttackMove),      /* M-N moves of direct play */     \
    ENUMERATOR(STDefenseMove),                                 \
    ENUMERATOR(STHelpMove),      /* M-N moves of help play */         \
    ENUMERATOR(STHelpFork),        /* decides when play in branch is over */ \
    ENUMERATOR(STSeriesMove),    /* M-N moves of series play */       \
    ENUMERATOR(STSeriesFork),      /* decides when play in branch is over */ \
                                                                        \
    ENUMERATOR(STLeafDirect),      /* goal in 1 */                      \
    ENUMERATOR(STLeafHelp),        /* help-goal in 1 */                 \
    ENUMERATOR(STLeafForced),      /* forced goal in 1 half move */     \
                                                                        \
    ENUMERATOR(STReciprocal),      /* logical AND */                    \
    ENUMERATOR(STQuodlibet),       /* logical OR */                     \
    ENUMERATOR(STNot),             /* logical NOT */                    \
                                                                        \
    ENUMERATOR(STMoveInverterRootSolvableFilter),    /* inverts side to move */ \
    ENUMERATOR(STMoveInverterSolvableFilter),    /* inverts side to move */ \
    ENUMERATOR(STMoveInverterSeriesFilter),    /* inverts side to move */ \
                                                                        \
    ENUMERATOR(STAttackRoot),      /* root level of direct play */      \
    ENUMERATOR(STDefenseRoot), /* root level of postkey direct play */ \
    ENUMERATOR(STAttackHashed),    /* direct play with hash table */    \
                                                                        \
    ENUMERATOR(STHelpRoot),        /* root level of help play */        \
    ENUMERATOR(STHelpShortcut),    /* selects branch for solving short solutions */        \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
                                                                        \
    ENUMERATOR(STSeriesRoot),      /* root level of series play */      \
    ENUMERATOR(STSeriesShortcut),  /* selects branch for solving short solutions */ \
                                                                        \
    ENUMERATOR(STParryFork),       /* parry move in series */           \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */    \
                                                    \
    ENUMERATOR(STSelfCheckGuardRootSolvableFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardSolvableFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardRootDefenderFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardAttackerFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardDefenderFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardHelpFilter),  /* stop when a side exposes its king */ \
    ENUMERATOR(STSelfCheckGuardSeriesFilter),  /* stop when a side exposes its king */ \
                                                                        \
    ENUMERATOR(STDirectDefense),   /* direct play, just played defense */ \
    ENUMERATOR(STReflexHelpFilter),/* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexSeriesFilter),    /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexAttackerFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STReflexDefenderFilter),  /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSelfAttack),      /* self play, just played attack */  \
    ENUMERATOR(STSelfDefense),     /* self play, just played defense */ \
                                                                        \
    ENUMERATOR(STRestartGuardRootDefenderFilter),    /* write move numbers */ \
    ENUMERATOR(STRestartGuardHelpFilter),    /* write move numbers */ \
    ENUMERATOR(STRestartGuardSeriesFilter),    /* write move numbers */ \
                                                                        \
    ENUMERATOR(STIntelligentHelpFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STIntelligentSeriesFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardHelpFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardSeriesFilter), /* goal still reachable in intelligent mode? */ \
                                                                        \
    ENUMERATOR(STKeepMatingGuardRootDefenderFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardAttackerFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardDefenderFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardHelpFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STKeepMatingGuardSeriesFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
                                                                        \
    ENUMERATOR(STMaxTimeRootDefenderFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeDefenderFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeHelpFilter), /* deals with option maxtime */  \
    ENUMERATOR(STMaxTimeSeriesFilter), /* deals with option maxtime */  \
                                                                        \
    ENUMERATOR(STMaxSolutionsRootSolvableFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsRootDefenderFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsHelpFilter), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsSeriesFilter), /* deals with option maxsolutions */ \
    ENUMERATOR(STStopOnShortSolutionsRootSolvableFilter), /* deals with option stoponshortsolutions */  \
    ENUMERATOR(STStopOnShortSolutionsHelpFilter), /* deals with option stoponshortsolutions */  \
     ENUMERATOR(STStopOnShortSolutionsSeriesFilter), /* deals with option stoponshortsolutions */  \
                                                                       \
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
  slice_structure_fork,   /* STSeriesFork */
  slice_structure_leaf,   /* STLeafDirect */
  slice_structure_leaf,   /* STLeafHelp */
  slice_structure_leaf,   /* STLeafForced */
  slice_structure_binary, /* STReciprocal */
  slice_structure_binary, /* STQuodlibet */
  slice_structure_pipe,   /* STNot */
  slice_structure_pipe,   /* STMoveInverterRootSolvableFilter */
  slice_structure_pipe,   /* STMoveInverterSolvableFilter */
  slice_structure_pipe,   /* STMoveInverterSeriesFilter */
  slice_structure_branch, /* STAttackRoot */
  slice_structure_branch, /* STDefenseRoot */
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
  slice_structure_pipe,   /* STSelfCheckGuardRootDefenderFilter */
  slice_structure_pipe,   /* STSelfCheckGuardAttackerFilter */
  slice_structure_pipe,   /* STSelfCheckGuardDefenderFilter */
  slice_structure_pipe,   /* STSelfCheckGuardHelpFilter */
  slice_structure_pipe,   /* STSelfCheckGuardSeriesFilter */
  slice_structure_fork,   /* STDirectDefense */
  slice_structure_fork,   /* STReflexHelpFilter */
  slice_structure_fork,   /* STReflexSeriesFilter */
  slice_structure_fork,   /* STReflexAttackerFilter */
  slice_structure_fork,   /* STReflexDefenderFilter */
  slice_structure_fork,   /* STSelfAttack */
  slice_structure_fork,   /* STSelfDefense */
  slice_structure_pipe,   /* STRestartGuardRootDefenderFilter */
  slice_structure_pipe,   /* STRestartGuardHelpFilter */
  slice_structure_pipe,   /* STRestartGuardSeriesFilter */
  slice_structure_branch, /* STIntelligentHelpFilter */
  slice_structure_branch, /* STIntelligentSeriesFilter */
  slice_structure_branch, /* STGoalReachableGuardHelpFilter */
  slice_structure_branch, /* STGoalReachableGuardSeriesFilter */
  slice_structure_pipe,   /* STKeepMatingGuardRootDefenderFilter */
  slice_structure_pipe,   /* STKeepMatingGuardAttackerFilter */
  slice_structure_pipe,   /* STKeepMatingGuardDefenderFilter */
  slice_structure_pipe,   /* STKeepMatingGuardHelpFilter */
  slice_structure_pipe,   /* STKeepMatingGuardSeriesFilter */
  slice_structure_pipe,   /* STMaxFlightsquares */
  slice_structure_pipe,   /* STDegenerateTree */
  slice_structure_branch, /* STMaxNrNonTrivial */
  slice_structure_pipe,   /* STMaxThreatLength */
  slice_structure_pipe,   /* STMaxTimeRootDefenderFilter */
  slice_structure_pipe,   /* STMaxTimeDefenderFilter */
  slice_structure_pipe,   /* STMaxTimeHelpFilter */
  slice_structure_pipe,   /* STMaxTimeSeriesFilter */
  slice_structure_pipe,   /* STMaxSolutionsRootSolvableFilter */
  slice_structure_pipe,   /* STMaxSolutionsRootDefenderFilter */
  slice_structure_pipe,   /* STMaxSolutionsHelpFilter */
  slice_structure_pipe,   /* STMaxSolutionsSeriesFilter */
  slice_structure_pipe,   /* STStopOnShortSolutionsRootSolvableFilter */
  slice_structure_branch, /* STStopOnShortSolutionsHelpFilter */
  slice_structure_branch  /* STStopOnShortSolutionsSeriesFilter */
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

static boolean mark_reachable_slice(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  boolean (* const leaked)[max_nr_slices] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(!is_slice_index_free[si]);
  (*leaked)[si] = false;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const reachable_slices_markers[] =
{
  0,                     /* STProxy */
  &mark_reachable_slice, /* STAttackMove */
  &mark_reachable_slice, /* STDefenseMove */
  &mark_reachable_slice, /* STHelpMove */
  &mark_reachable_slice, /* STHelpFork */
  &mark_reachable_slice, /* STSeriesMove */
  &mark_reachable_slice, /* STSeriesFork */
  &mark_reachable_slice, /* STLeafDirect */
  &mark_reachable_slice, /* STLeafHelp */
  &mark_reachable_slice, /* STLeafForced */
  &mark_reachable_slice, /* STReciprocal */
  &mark_reachable_slice, /* STQuodlibet */
  &mark_reachable_slice, /* STNot */
  &mark_reachable_slice, /* STMoveInverterRootSolvableFilter */
  &mark_reachable_slice, /* STMoveInverterSolvableFilter */
  &mark_reachable_slice, /* STMoveInverterSeriesFilter */
  &mark_reachable_slice, /* STAttackRoot */
  &mark_reachable_slice, /* STDefenseRoot */
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
  &mark_reachable_slice, /* STSelfCheckGuardRootDefenderFilter */
  &mark_reachable_slice, /* STSelfCheckGuardAttackerFilter */
  &mark_reachable_slice, /* STSelfCheckGuardDefenderFilter */
  &mark_reachable_slice, /* STSelfCheckGuardHelpFilter */
  &mark_reachable_slice, /* STSelfCheckGuardSeriesFilter */
  &mark_reachable_slice, /* STDirectDefense */
  &mark_reachable_slice, /* STReflexHelpFilter */
  &mark_reachable_slice, /* STReflexSeriesFilter */
  &mark_reachable_slice, /* STReflexAttackerFilter */
  &mark_reachable_slice, /* STReflexDefenderFilter */
  &mark_reachable_slice, /* STSelfAttack */
  &mark_reachable_slice, /* STSelfDefense */
  &mark_reachable_slice, /* STRestartGuardRootDefenderFilter */
  &mark_reachable_slice, /* STRestartGuardHelpFilter */
  &mark_reachable_slice, /* STRestartGuardSeriesFilter */
  &mark_reachable_slice, /* STIntelligentHelpFilter */
  &mark_reachable_slice, /* STIntelligentSeriesFilter */
  &mark_reachable_slice, /* STGoalReachableGuardHelpFilter */
  &mark_reachable_slice, /* STGoalReachableGuardSeriesFilter */
  &mark_reachable_slice, /* STKeepMatingGuardRootDefenderFilter */
  &mark_reachable_slice, /* STKeepMatingGuardAttackerFilter */
  &mark_reachable_slice, /* STKeepMatingGuardDefenderFilter */
  &mark_reachable_slice, /* STKeepMatingGuardHelpFilter */
  &mark_reachable_slice, /* STKeepMatingGuardSeriesFilter */
  &mark_reachable_slice, /* STMaxFlightsquares */
  &mark_reachable_slice, /* STDegenerateTree */
  &mark_reachable_slice, /* STMaxNrNonTrivial */
  &mark_reachable_slice, /* STMaxThreatLength */
  &mark_reachable_slice, /* STMaxTimeRootDefenderFilter */
  &mark_reachable_slice, /* STMaxTimeDefenderFilter */
  &mark_reachable_slice, /* STMaxTimeHelpFilter */
  &mark_reachable_slice, /* STMaxTimeSeriesFilter */
  &mark_reachable_slice, /* STMaxSolutionsRootSolvableFilter */
  &mark_reachable_slice, /* STMaxSolutionsRootDefenderFilter */
  &mark_reachable_slice, /* STMaxSolutionsHelpFilter */
  &mark_reachable_slice, /* STMaxSolutionsSeriesFilter */
  &mark_reachable_slice, /* STStopOnShortSolutionsRootSolvableFilter */
  &mark_reachable_slice, /* STStopOnShortSolutionsHelpFilter */
  &mark_reachable_slice  /* STStopOnShortSolutionsSeriesFilter */
};

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void)
{
  boolean leaked[max_nr_slices];
  slice_index i;
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);
  if (root_slice!=no_slice)
  {
    for (i = 0; i!=max_nr_slices; ++i)
      leaked[i] = !is_slice_index_free[i];

    slice_traversal_init(&st,&reachable_slices_markers,&leaked);
    traverse_slices(root_slice,&st);

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

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(SliceType type, square s)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(type==STLeafDirect
         || type==STLeafHelp
         || type==STLeafForced);

  result = alloc_slice(type);
  slices[result].u.leaf.goal = goal_target;
  slices[result].u.leaf.target = s;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a (non-target) leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_leaf_slice(SliceType type, Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",type);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  assert(type==STLeafDirect
         || type==STLeafHelp
         || type==STLeafForced);

  result = alloc_slice(type);
  slices[result].u.leaf.goal = goal;
  slices[result].u.leaf.target = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

static boolean traverse_and_deallocate(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean traverse_and_deallocate_proxy(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  dealloc_proxy_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const deallocators[] =
{
  &traverse_and_deallocate_proxy, /* STProxy */
  &traverse_and_deallocate,       /* STAttackMove */
  &traverse_and_deallocate,       /* STDefenseMove */
  &traverse_and_deallocate,       /* STHelpMove */
  &traverse_and_deallocate,       /* STHelpFork */
  &traverse_and_deallocate,       /* STSeriesMove */
  &traverse_and_deallocate,       /* STSeriesFork */
  &traverse_and_deallocate,       /* STLeafDirect */
  &traverse_and_deallocate,       /* STLeafHelp */
  &traverse_and_deallocate,       /* STLeafForced */
  &traverse_and_deallocate,       /* STReciprocal */
  &traverse_and_deallocate,       /* STQuodlibet */
  &traverse_and_deallocate,       /* STNot */
  &traverse_and_deallocate,       /* STMoveInverterRootSolvableFilter */
  &traverse_and_deallocate,       /* STMoveInverterSolvableFilter */
  &traverse_and_deallocate,       /* STMoveInverterSeriesFilter */
  &traverse_and_deallocate,       /* STAttackRoot */
  &traverse_and_deallocate,       /* STDefenseRoot */
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
  &traverse_and_deallocate,       /* STSelfCheckGuardRootDefenderFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardAttackerFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardDefenderFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardHelpFilter */
  &traverse_and_deallocate,       /* STSelfCheckGuardSeriesFilter */
  &traverse_and_deallocate,       /* STDirectDefense */
  &traverse_and_deallocate,       /* STReflexHelpFilter */
  &traverse_and_deallocate,       /* STReflexSeriesFilter */
  &traverse_and_deallocate,       /* STReflexAttackerFilter */
  &traverse_and_deallocate,       /* STReflexDefenderFilter */
  &traverse_and_deallocate,       /* STSelfAttack */
  &traverse_and_deallocate,       /* STSelfDefense */
  &traverse_and_deallocate,       /* STRestartGuardRootDefenderFilter */
  &traverse_and_deallocate,       /* STRestartGuardHelpFilter */
  &traverse_and_deallocate,       /* STRestartGuardSeriesFilter */
  &traverse_and_deallocate,       /* STIntelligentHelpFilter */
  &traverse_and_deallocate,       /* STIntelligentSeriesFilter */
  &traverse_and_deallocate,       /* STGoalReachableGuardHelpFilter */
  &traverse_and_deallocate,       /* STGoalReachableGuardSeriesFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardRootDefenderFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardAttackerFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardDefenderFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardHelpFilter */
  &traverse_and_deallocate,       /* STKeepMatingGuardSeriesFilter */
  &traverse_and_deallocate,       /* STMaxFlightsquares */
  &traverse_and_deallocate,       /* STDegenerateTree */
  &traverse_and_deallocate,       /* STMaxNrNonTrivial */
  &traverse_and_deallocate,       /* STMaxThreatLength */
  &traverse_and_deallocate,       /* STMaxTimeRootDefenderFilter */
  &traverse_and_deallocate,       /* STMaxTimeDefenderFilter */
  &traverse_and_deallocate,       /* STMaxTimeHelpFilter */
  &traverse_and_deallocate,       /* STMaxTimeSeriesFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsRootSolvableFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsRootDefenderFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsHelpFilter */
  &traverse_and_deallocate,       /* STMaxSolutionsSeriesFilter */
  &traverse_and_deallocate,       /* STStopOnShortSolutionsRootSolvableFilter */
  &traverse_and_deallocate,       /* STStopOnShortSolutionsHelpFilter */
  &traverse_and_deallocate        /* STStopOnShortSolutionsSeriesFilter */
};

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&deallocators,0);
  traverse_slices(si,&st);

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

static boolean leaf_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *root = si;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const root_slice_inserters[] =
{
  &slice_traverse_children,            /* STProxy */
  &attack_move_insert_root,            /* STAttackMove */
  &defense_move_insert_root,      /* STDefenseMove */
  &help_move_insert_root,              /* STHelpMove */
  &help_fork_insert_root,              /* STHelpFork */
  &series_move_insert_root,            /* STSeriesMove */
  &series_fork_insert_root,            /* STSeriesFork */
  &leaf_insert_root,                   /* STLeafDirect */
  &leaf_insert_root,                   /* STLeafHelp */
  &leaf_insert_root,                   /* STLeafForced */
  &reci_insert_root,                   /* STReciprocal */
  &quodlibet_insert_root,              /* STQuodlibet */
  &not_insert_root,                    /* STNot */
  &slice_traverse_children,            /* STMoveInverterRootSolvableFilter */
  &move_inverter_insert_root,          /* STMoveInverterSolvableFilter */
  &slice_traverse_children,            /* STMoveInverterSeriesFilter */
  &slice_traverse_children,            /* STAttackRoot */
  &slice_traverse_children,            /* STDefenseRoot */
  &slice_traverse_children,            /* STAttackHashed */
  &slice_traverse_children,            /* STHelpRoot */
  &slice_traverse_children,            /* STHelpShortcut */
  &slice_traverse_children,            /* STHelpHashed */
  &slice_traverse_children,            /* STSeriesRoot */
  &slice_traverse_children,            /* STSeriesShortcut */
  &slice_traverse_children,            /* STParryFork */
  &slice_traverse_children,            /* STSeriesHashed */
  &slice_traverse_children,            /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,            /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,            /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,            /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,            /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,            /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,            /* STSelfCheckGuardSeriesFilter */
  &direct_defense_insert_root,         /* STDirectDefense */
  &reflex_help_filter_insert_root,     /* STReflexHelpFilter */
  &reflex_series_filter_insert_root,   /* STReflexSeriesFilter */
  &reflex_attacker_filter_insert_root, /* STReflexAttackerFilter */
  &reflex_defender_filter_insert_root, /* STReflexDefenderFilter */
  &self_attack_insert_root,            /* STSelfAttack */
  &self_defense_insert_root,           /* STSelfDefense */
  &slice_traverse_children,            /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,            /* STRestartGuardHelpFilter */
  &slice_traverse_children,            /* STRestartGuardSeriesFilter */
  &slice_traverse_children,            /* STIntelligentHelpFilter */
  &slice_traverse_children,            /* STIntelligentSeriesFilter */
  &slice_traverse_children,            /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,            /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,            /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,            /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,            /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,            /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,            /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,            /* STMaxFlightsquares */
  &slice_traverse_children,            /* STDegenerateTree */
  &slice_traverse_children,            /* STMaxNrNonTrivial */
  &slice_traverse_children,            /* STMaxThreatLength */
  &slice_traverse_children,            /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,            /* STMaxTimeDefenderFilter */
  &slice_traverse_children,            /* STMaxTimeHelpFilter */
  &slice_traverse_children,            /* STMaxTimeSeriesFilter */
  &slice_traverse_children,            /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,            /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,            /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,            /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,            /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,            /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children             /* STStopOnShortSolutionsSeriesFilter */
};

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 */
void stip_insert_root_slices(void)
{
  slice_traversal st;
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&root_slice_inserters,&result);
  traverse_slices(root_slice,&st);

  pipe_link(root_slice,result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const proxy_resolvers[] =
{
  &pipe_resolve_proxies,            /* STProxy */
  &pipe_resolve_proxies,            /* STAttackMove */
  &pipe_resolve_proxies,            /* STDefenseMove */
  &pipe_resolve_proxies,            /* STHelpMove */
  &branch_fork_resolve_proxies,     /* STHelpFork */
  &pipe_resolve_proxies,            /* STSeriesMove */
  &branch_fork_resolve_proxies,     /* STSeriesFork */
  &slice_traverse_children,         /* STLeafDirect */
  &slice_traverse_children,         /* STLeafHelp */
  &slice_traverse_children,         /* STLeafForced */
  &binary_resolve_proxies,          /* STReciprocal */
  &binary_resolve_proxies,          /* STQuodlibet */
  &pipe_resolve_proxies,            /* STNot */
  &pipe_resolve_proxies,            /* STMoveInverterRootSolvableFilter */
  &pipe_resolve_proxies,            /* STMoveInverterSolvableFilter */
  &pipe_resolve_proxies,            /* STMoveInverterSeriesFilter */
  &pipe_resolve_proxies,            /* STAttackRoot */
  &pipe_resolve_proxies,            /* STDefenseRoot */
  &pipe_resolve_proxies,            /* STAttackHashed */
  &pipe_resolve_proxies,            /* STHelpRoot */
  &help_shortcut_resolve_proxies,   /* STHelpShortcut */
  &pipe_resolve_proxies,            /* STHelpHashed */
  &pipe_resolve_proxies,            /* STSeriesRoot */
  &series_shortcut_resolve_proxies, /* STSeriesShortcut */
  &slice_traverse_children,         /* STParryFork */
  &pipe_resolve_proxies,            /* STSeriesHashed */
  &pipe_resolve_proxies,            /* STSelfCheckGuardRootSolvableFilter */
  &pipe_resolve_proxies,            /* STSelfCheckGuardSolvableFilter */
  &pipe_resolve_proxies,            /* STSelfCheckGuardRootDefenderFilter */
  &pipe_resolve_proxies,            /* STSelfCheckGuardAttackerFilter */
  &pipe_resolve_proxies,            /* STSelfCheckGuardDefenderFilter */
  &pipe_resolve_proxies,            /* STSelfCheckGuardHelpFilter */
  &pipe_resolve_proxies,            /* STSelfCheckGuardSeriesFilter */
  &branch_fork_resolve_proxies,     /* STDirectDefense */
  &reflex_filter_resolve_proxies,   /* STReflexHelpFilter */
  &reflex_filter_resolve_proxies,   /* STReflexSeriesFilter */
  &reflex_filter_resolve_proxies,   /* STReflexAttackerFilter */
  &reflex_filter_resolve_proxies,   /* STReflexDefenderFilter */
  &branch_fork_resolve_proxies,     /* STSelfAttack */
  &branch_fork_resolve_proxies,     /* STSelfDefense */
  &slice_traverse_children,         /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,         /* STRestartGuardHelpFilter */
  &slice_traverse_children,         /* STRestartGuardSeriesFilter */
  &pipe_resolve_proxies,            /* STIntelligentHelpFilter */
  &pipe_resolve_proxies,            /* STIntelligentSeriesFilter */
  &pipe_resolve_proxies,            /* STGoalReachableGuardHelpFilter */
  &pipe_resolve_proxies,            /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,         /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,         /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,         /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,         /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,         /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,         /* STMaxFlightsquares */
  &slice_traverse_children,         /* STDegenerateTree */
  &slice_traverse_children,         /* STMaxNrNonTrivial */
  &slice_traverse_children,         /* STMaxThreatLength */
  &pipe_resolve_proxies,            /* STMaxTimeRootDefenderFilter */
  &pipe_resolve_proxies,            /* STMaxTimeDefenderFilter */
  &pipe_resolve_proxies,            /* STMaxTimeHelpFilter */
  &pipe_resolve_proxies,            /* STMaxTimeSeriesFilter */
  &pipe_resolve_proxies,            /* STMaxSolutionsRootSolvableFilter */
  &pipe_resolve_proxies,            /* STMaxSolutionsRootDefenderFilter */
  &pipe_resolve_proxies,            /* STMaxSolutionsHelpFilter */
  &pipe_resolve_proxies,            /* STMaxSolutionsSeriesFilter */
  &pipe_resolve_proxies,            /* STStopOnShortSolutionsRootSolvableFilter */
  &pipe_resolve_proxies,            /* STStopOnShortSolutionsHelpFilter */
  &pipe_resolve_proxies             /* STStopOnShortSolutionsSeriesFilter */
};

/* Substitute links to proxy slices by the proxy's target
 */
void resolve_proxies(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  assert(slices[root_slice].type==STProxy);
  proxy_slice_resolve(&root_slice);

  slice_traversal_init(&st,&proxy_resolvers,0);
  traverse_slices(root_slice,&st);

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
 * @param address of structure representing traversal
 * @return true iff traversal of si was successful
 */
static boolean get_max_nr_moves_direct_defender(slice_index si,
                                                slice_traversal *st)
{
  boolean result;
  stip_length_type * const max_nr = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(slices[si].u.pipe.next,st);
  *max_nr += slices[si].u.branch.length+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param address of structure representing traversal
 * @return true iff traversal of si was successful
 */
static boolean get_max_nr_moves_branch(slice_index si, slice_traversal *st)
{
  boolean result;
  stip_length_type * const max_nr = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_traverse_children(si,st);
  *max_nr += slices[si].u.branch.length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param address of structure representing traversal
 * @return true iff traversal of si was successful
 */
static boolean get_max_nr_moves_branch_fork(slice_index si,
                                            slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(slices[si].u.branch_fork.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param address of structure representing traversal
 * @return true iff traversal of si was successful
 */
static boolean get_max_nr_moves_leaf(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  stip_length_type * const max_nr = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *max_nr = 1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const get_max_nr_moves_functions[] =
{
  &slice_traverse_children,          /* STProxy */
  &slice_traverse_children,          /* STAttackMove */
  &get_max_nr_moves_direct_defender, /* STDefenseMove */
  &get_max_nr_moves_branch,          /* STHelpMove */
  &get_max_nr_moves_branch_fork,     /* STHelpFork */
  &get_max_nr_moves_branch,          /* STSeriesMove */
  &get_max_nr_moves_branch_fork,     /* STSeriesFork */
  &get_max_nr_moves_leaf,            /* STLeafDirect */
  &get_max_nr_moves_leaf,            /* STLeafHelp */
  &get_max_nr_moves_leaf,            /* STLeafForced */
  &slice_traverse_children,          /* STReciprocal */
  &slice_traverse_children,          /* STQuodlibet */
  &slice_traverse_children,          /* STNot */
  &slice_traverse_children,          /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,          /* STMoveInverterSolvableFilter */
  &slice_traverse_children,          /* STMoveInverterSeriesFilter */
  &slice_traverse_children,          /* STAttackRoot */
  &slice_traverse_children,          /* STDefenseRoot */
  &slice_traverse_children,          /* STAttackHashed */
  &slice_traverse_children,          /* STHelpRoot */
  &slice_traverse_children,          /* STHelpShortcut */
  &slice_traverse_children,          /* STHelpHashed */
  &slice_traverse_children,          /* STSeriesRoot */
  &slice_traverse_children,          /* STSeriesShortcut */
  &slice_traverse_children,          /* STParryFork */
  &slice_traverse_children,          /* STSeriesHashed */
  &slice_traverse_children,          /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,          /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,          /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,          /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,          /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,          /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,          /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,          /* STDirectDefense */
  &slice_traverse_children,          /* STReflexHelpFilter */
  &slice_traverse_children,          /* STReflexSeriesFilter */
  &slice_traverse_children,          /* STReflexAttackerFilter */
  &slice_traverse_children,          /* STReflexDefenderFilter */
  &slice_traverse_children,          /* STSelfAttack */
  &slice_traverse_children,          /* STSelfDefense */
  &slice_traverse_children,          /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,          /* STRestartGuardHelpFilter */
  &slice_traverse_children,          /* STRestartGuardSeriesFilter */
  &slice_traverse_children,          /* STIntelligentHelpFilter */
  &slice_traverse_children,          /* STIntelligentSeriesFilter */
  &slice_traverse_children,          /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,          /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,          /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,          /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,          /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,          /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,          /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,          /* STMaxFlightsquares */
  &slice_traverse_children,          /* STDegenerateTree */
  &slice_traverse_children,          /* STMaxNrNonTrivial */
  &slice_traverse_children,          /* STMaxThreatLength */
  &slice_traverse_children,          /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,          /* STMaxTimeDefenderFilter */
  &slice_traverse_children,          /* STMaxTimeHelpFilter */
  &slice_traverse_children,          /* STMaxTimeSeriesFilter */
  &slice_traverse_children,          /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,          /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,          /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,          /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,          /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,          /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children           /* STStopOnShortSolutionsSeriesFilter */
};

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si)
{
  slice_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&get_max_nr_moves_functions,&result);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_goals_equal(slice_index si1, slice_index si2)
{
  return ((slices[si1].u.leaf.goal ==slices[si2].u.leaf.goal)
          && (slices[si1].u.leaf.goal!=goal_target
              || (slices[si1].u.leaf.target==slices[si2].u.leaf.target)));
}

enum
{
  no_unique_goal = max_nr_slices+1
};

static boolean find_unique_goal_leaf(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*found==no_slice)
    *found = si;
  else if (*found!=no_unique_goal && !are_goals_equal(*found,si))
    *found = no_unique_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const unique_goal_finders[] =
{
  &slice_traverse_children, /* STProxy */
  &slice_traverse_children, /* STAttackMove */
  &slice_traverse_children, /* STDefenseMove */
  &slice_traverse_children, /* STHelpMove */
  &slice_traverse_children, /* STHelpFork */
  &slice_traverse_children, /* STSeriesMove */
  &slice_traverse_children, /* STSeriesFork */
  &find_unique_goal_leaf,   /* STLeafDirect */
  &find_unique_goal_leaf,   /* STLeafHelp */
  &find_unique_goal_leaf,   /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSeriesFilter */
  &slice_traverse_children, /* STAttackRoot */
  &slice_traverse_children, /* STDefenseRoot */
  &slice_traverse_children, /* STAttackHashed */
  &slice_traverse_children, /* STHelpRoot */
  &slice_traverse_children, /* STHelpShortcut */
  &slice_traverse_children, /* STHelpHashed */
  &slice_traverse_children, /* STSeriesRoot */
  &slice_traverse_children, /* STSeriesShortcut */
  &slice_traverse_children, /* STParryFork */
  &slice_traverse_children, /* STSeriesHashed */
  &slice_traverse_children, /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children, /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children, /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children, /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children, /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children, /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children, /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children, /* STDirectDefense */
  &slice_traverse_children, /* STReflexHelpFilter */
  &slice_traverse_children, /* STReflexSeriesFilter */
  &slice_traverse_children, /* STReflexAttackerFilter */
  &slice_traverse_children, /* STReflexDefenderFilter */
  &slice_traverse_children, /* STSelfAttack */
  &slice_traverse_children, /* STSelfDefense */
  &slice_traverse_children, /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children, /* STRestartGuardHelpFilter */
  &slice_traverse_children, /* STRestartGuardSeriesFilter */
  &slice_traverse_children, /* STIntelligentHelpFilter */
  &slice_traverse_children, /* STIntelligentSeriesFilter */
  &slice_traverse_children, /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children, /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children, /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children, /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children, /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children, /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children, /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children, /* STMaxFlightsquares */
  &slice_traverse_children, /* STDegenerateTree */
  &slice_traverse_children, /* STMaxNrNonTrivial */
  &slice_traverse_children, /* STMaxThreatLength */
  &slice_traverse_children, /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children, /* STMaxTimeDefenderFilter */
  &slice_traverse_children, /* STMaxTimeHelpFilter */
  &slice_traverse_children, /* STMaxTimeSeriesFilter */
  &slice_traverse_children, /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children, /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children, /* STMaxSolutionsHelpFilter */
  &slice_traverse_children, /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children, /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @param si root of subtree where to look for unique goal
 * @return no_slice if goal is not unique; index of a slice with the
 * unique goal otherwise
 */
slice_index find_unique_goal(slice_index si)
{
  slice_traversal st;
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&unique_goal_finders,&result);
  traverse_slices(root_slice,&st);

  if (result==no_unique_goal)
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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
    (*copies)[si] = result;

    TraceEnumerator(SliceType,slices[si].type,"\n");
    switch (slices[si].type)
    {
      case STLeafDirect:
      case STLeafHelp:
      case STLeafForced:
        /* nothing */
        break;

      case STAttackMove:
      case STDefenseMove:
      case STHelpRoot:
      case STHelpMove:
      case STSeriesMove:
      case STAttackRoot:
      case STDefenseRoot:
      case STSeriesRoot:
      case STNot:
      case STMoveInverterRootSolvableFilter:
      case STMoveInverterSolvableFilter:
      case STMoveInverterSeriesFilter:
      case STAttackHashed:
      case STHelpHashed:
      case STSeriesHashed:
      case STSelfCheckGuardRootSolvableFilter:
      case STSelfCheckGuardSolvableFilter:
      case STSelfCheckGuardRootDefenderFilter:
      case STSelfCheckGuardAttackerFilter:
      case STSelfCheckGuardDefenderFilter:
      case STSelfCheckGuardHelpFilter:
      case STSelfCheckGuardSeriesFilter:
      case STRestartGuardRootDefenderFilter:
      case STRestartGuardHelpFilter:
      case STRestartGuardSeriesFilter:
      case STGoalReachableGuardHelpFilter:
      case STGoalReachableGuardSeriesFilter:
      case STKeepMatingGuardRootDefenderFilter:
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
      case STDirectDefense:
      case STSelfAttack:
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
static slice_index deep_copy(slice_index si)
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

/* Convert a leaf slice to STLeafDirect as part of a traversal
 * @param si identifies leaf slice to be converted
 * @param st address of structure representing the traversal
 */
static boolean make_leaf_direct(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STLeafDirect;
  if (slices[si].starter!=no_side)
    slices[si].starter = advers(slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const leaves_direct_makers[] =
{
  &slice_traverse_children,   /* STProxy */
  &slice_traverse_children,   /* STAttackMove */
  &slice_traverse_children,   /* STDefenseMove */
  &slice_traverse_children,   /* STHelpMove */
  &slice_traverse_children,   /* STHelpFork */
  &slice_traverse_children,   /* STSeriesMove */
  &slice_traverse_children,   /* STSeriesFork */
  &slice_operation_noop,      /* STLeafDirect */
  &make_leaf_direct,          /* STLeafHelp */
  &make_leaf_direct,          /* STLeafForced */
  &slice_traverse_children,   /* STReciprocal */
  &slice_traverse_children,   /* STQuodlibet */
  &slice_traverse_children,   /* STNot */
  &slice_traverse_children,   /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,   /* STMoveInverterSolvableFilter */
  &slice_traverse_children,   /* STMoveInverterSeriesFilter */
  &slice_traverse_children,   /* STAttackRoot */
  &slice_traverse_children,   /* STDefenseRoot */
  &slice_traverse_children,   /* STAttackHashed */
  &slice_traverse_children,   /* STHelpRoot */
  &slice_traverse_children,   /* STHelpShortcut */
  &slice_traverse_children,   /* STHelpHashed */
  &slice_traverse_children,   /* STSeriesRoot */
  &slice_traverse_children,   /* STSeriesShortcut */
  &slice_traverse_children,   /* STParryFork */
  &slice_traverse_children,   /* STSeriesHashed */
  &slice_traverse_children,   /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,   /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,   /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,   /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,   /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,   /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,   /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,   /* STDirectDefense */
  &slice_traverse_children,   /* STReflexHelpFilter */
  &slice_traverse_children,   /* STReflexSeriesFilter */
  &slice_traverse_children,   /* STReflexAttackerFilter */
  &slice_traverse_children,   /* STReflexDefenderFilter */
  &slice_traverse_children,   /* STSelfAttack */
  &slice_traverse_children,   /* STSelfDefense */
  &slice_traverse_children,   /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,   /* STRestartGuardHelpFilter */
  &slice_traverse_children,   /* STRestartGuardSeriesFilter */
  &slice_traverse_children,   /* STIntelligentHelpFilter */
  &slice_traverse_children,   /* STIntelligentSeriesFilter */
  &slice_traverse_children,   /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,   /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,   /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,   /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,   /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,   /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,   /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,   /* STMaxFlightsquares */
  &slice_traverse_children,   /* STDegenerateTree */
  &slice_traverse_children,   /* STMaxNrNonTrivial */
  &slice_traverse_children,   /* STMaxThreatLength */
  &slice_traverse_children,   /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,   /* STMaxTimeDefenderFilter */
  &slice_traverse_children,   /* STMaxTimeHelpFilter */
  &slice_traverse_children,   /* STMaxTimeSeriesFilter */
  &slice_traverse_children,   /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,   /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,   /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,   /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,   /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,   /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children    /* STStopOnShortSolutionsSeriesFilter */
};

/* Convert all leaves of a stipulation sub-tree to STLeafDirect
 */
static void make_leaves_direct(slice_index si)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&leaves_direct_makers,0);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_direct_defense_after(slice_index pos,
                                        slice_index proxy_to_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pos);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  if (slices[slices[pos].u.pipe.next].type==STDirectDefense)
    /* already done*/ ;
  else
  {
    stip_length_type const length = slices[pos].u.branch.length;
    stip_length_type const min_length = slices[pos].u.branch.min_length;
    slice_index dirdef = alloc_direct_defense(length,min_length,proxy_to_goal);
    pipe_link(dirdef,slices[pos].u.pipe.next);
    pipe_link(pos,dirdef);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean transform_to_quodlibet_self_attack(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  slice_index const proxy_to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index * const new_proxy_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *new_proxy_to_goal = deep_copy(proxy_to_goal);
  make_leaves_direct(*new_proxy_to_goal);

  slice_traverse_children(si,st);

  *new_proxy_to_goal = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean transform_to_quodlibet_branch_direct_defender(slice_index si,
                                                             slice_traversal *st)
{
  boolean const result = true;
  slice_index pos = si;
  slice_index const * const new_proxy_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  /* If we are instrumenting a s#N, append a STDirectDefense slice
   * after each STSelfDefense.
   * Otherwise (i.e. if we are instrumenting a semi-r#n), insert the
   * STDirectDefense slice directly after each STDefenseMove
   * slice.
   */
  if (slices[slices[pos].u.pipe.next].type==STSelfDefense)
    pos = slices[pos].u.pipe.next;
  insert_direct_defense_after(pos,*new_proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean transform_to_quodlibet_branch_fork(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't recurse towards goal */
  traverse_slices(next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const to_quodlibet_transformers[] =
{
  &slice_traverse_children,                       /* STProxy */
  &slice_traverse_children,                       /* STAttackMove */
  &transform_to_quodlibet_branch_direct_defender, /* STDefenseMove */
  &slice_traverse_children,                       /* STHelpMove */
  &transform_to_quodlibet_branch_fork,            /* STHelpFork */
  &slice_traverse_children,                       /* STSeriesMove */
  &transform_to_quodlibet_branch_fork,            /* STSeriesFork */
  &slice_operation_noop,                          /* STLeafDirect */
  &slice_operation_noop,                          /* STLeafHelp */
  &slice_operation_noop,                          /* STLeafForced */
  &slice_traverse_children,                       /* STReciprocal */
  &slice_traverse_children,                       /* STQuodlibet */
  &slice_traverse_children,                       /* STNot */
  &slice_traverse_children,                       /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,                       /* STMoveInverterSolvableFilter */
  &slice_traverse_children,                       /* STMoveInverterSeriesFilter */
  &slice_traverse_children,                       /* STAttackRoot */
  &transform_to_quodlibet_branch_direct_defender, /* STDefenseRoot */
  &slice_traverse_children,                       /* STAttackHashed */
  &slice_traverse_children,                       /* STHelpRoot */
  &slice_traverse_children,                       /* STHelpShortcut */
  &slice_traverse_children,                       /* STHelpHashed */
  &slice_traverse_children,                       /* STSeriesRoot */
  &slice_traverse_children,                       /* STSeriesShortcut */
  &slice_traverse_children,                       /* STParryFork */
  &slice_traverse_children,                       /* STSeriesHashed */
  &slice_traverse_children,                       /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,                       /* STDirectDefense */
  &slice_traverse_children,                       /* STReflexHelpFilter */
  &slice_traverse_children,                       /* STReflexSeriesFilter */
  &slice_traverse_children,                       /* STReflexAttackerFilter */
  &transform_to_quodlibet_self_attack,            /* STReflexDefenderFilter */
  &transform_to_quodlibet_self_attack,            /* STSelfAttack */
  &slice_traverse_children,                       /* STSelfDefense */
  &slice_traverse_children,                       /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,                       /* STRestartGuardHelpFilter */
  &slice_traverse_children,                       /* STRestartGuardSeriesFilter */
  &slice_traverse_children,                       /* STIntelligentHelpFilter */
  &slice_traverse_children,                       /* STIntelligentSeriesFilter */
  &slice_traverse_children,                       /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,                       /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,                       /* STMaxFlightsquares */
  &slice_traverse_children,                       /* STDegenerateTree */
  &slice_traverse_children,                       /* STMaxNrNonTrivial */
  &slice_traverse_children,                       /* STMaxThreatLength */
  &slice_traverse_children,                       /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,                       /* STMaxTimeDefenderFilter */
  &slice_traverse_children,                       /* STMaxTimeHelpFilter */
  &slice_traverse_children,                       /* STMaxTimeSeriesFilter */
  &slice_traverse_children,                       /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,                       /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,                       /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,                       /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,                       /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,                       /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children                        /* STStopOnShortSolutionsSeriesFilter */
};

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal. 
 */
void transform_to_quodlibet(void)
{
  slice_traversal st;
  slice_index towards_goal = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&to_quodlibet_transformers,&towards_goal);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const to_postkey_play_reducers[] =
{
  &slice_traverse_children,                       /* STProxy */
  &slice_traverse_children,                       /* STAttackMove */
  &slice_traverse_children,                       /* STDefenseMove */
  &slice_traverse_children,                       /* STHelpMove */
  &slice_traverse_children,                       /* STHelpFork */
  &slice_traverse_children,                       /* STSeriesMove */
  &slice_traverse_children,                       /* STSeriesFork */
  &slice_traverse_children,                       /* STLeafDirect */
  &slice_traverse_children,                       /* STLeafHelp */
  &slice_traverse_children,                       /* STLeafForced */
  &slice_traverse_children,                       /* STReciprocal */
  &slice_traverse_children,                       /* STQuodlibet */
  &slice_traverse_children,                       /* STNot */
  &slice_traverse_children,                       /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,                       /* STMoveInverterSolvableFilter */
  &slice_traverse_children,                       /* STMoveInverterSeriesFilter */
  &attack_root_reduce_to_postkey_play,            /* STAttackRoot */
  &defense_root_reduce_to_postkey_play, /* STDefenseRoot */
  &slice_traverse_children,                       /* STAttackHashed */
  &slice_traverse_children,                       /* STHelpRoot */
  &slice_traverse_children,                       /* STHelpShortcut */
  &slice_traverse_children,                       /* STHelpHashed */
  &slice_traverse_children,                       /* STSeriesRoot */
  &slice_traverse_children,                       /* STSeriesShortcut */
  &slice_traverse_children,                       /* STParryFork */
  &slice_traverse_children,                       /* STSeriesHashed */
  &slice_traverse_children,                       /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,                       /* STSelfCheckGuardSeriesFilter */
  &direct_defense_root_reduce_to_postkey_play,    /* STDirectDefense */
  &slice_traverse_children,                       /* STReflexHelpFilter */
  &slice_traverse_children,                       /* STReflexSeriesFilter */
  &reflex_attacker_filter_reduce_to_postkey_play, /* STReflexAttackerFilter */
  &reflex_defender_filter_reduce_to_postkey_play, /* STReflexDefenderFilter */
  &self_attack_root_reduce_to_postkey_play,       /* STSelfAttack */
  &slice_traverse_children,                       /* STSelfDefense */
  &slice_traverse_children,                       /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,                       /* STRestartGuardHelpFilter */
  &slice_traverse_children,                       /* STRestartGuardSeriesFilter */
  &slice_traverse_children,                       /* STIntelligentHelpFilter */
  &slice_traverse_children,                       /* STIntelligentSeriesFilter */
  &slice_traverse_children,                       /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,                       /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,                       /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,                       /* STMaxFlightsquares */
  &slice_traverse_children,                       /* STDegenerateTree */
  &slice_traverse_children,                       /* STMaxNrNonTrivial */
  &slice_traverse_children,                       /* STMaxThreatLength */
  &slice_traverse_children,                       /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,                       /* STMaxTimeDefenderFilter */
  &slice_traverse_children,                       /* STMaxTimeHelpFilter */
  &slice_traverse_children,                       /* STMaxTimeSeriesFilter */
  &slice_traverse_children,                       /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,                       /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,                       /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,                       /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,                       /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,                       /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children                        /* STStopOnShortSolutionsSeriesFilter */
};

/* Install the slice representing the postkey slice at the stipulation
 * root
 * @param postkey_slice identifies slice to be installed
 */
static void install_postkey_slice(slice_index postkey_slice)
{
  slice_index const inverter = alloc_move_inverter_root_solvable_filter();
  pipe_link(inverter,postkey_slice);
  assert(slices[root_slice].type==STProxy);
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
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&to_postkey_play_reducers,&postkey_slice);
  traverse_slices(root_slice,&st);

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

static slice_operation const setplay_makers[] =
{
  &slice_traverse_children,                   /* STProxy */
  &slice_traverse_children,                   /* STAttackMove */
  &slice_traverse_children,                   /* STDefenseMove */
  &pipe_traverse_next,                        /* STHelpMove */
  &help_fork_make_setplay_slice,              /* STHelpFork */
  &pipe_traverse_next,                        /* STSeriesMove */
  &series_fork_make_setplay_slice,            /* STSeriesFork */
  &slice_traverse_children,                   /* STLeafDirect */
  &slice_traverse_children,                   /* STLeafHelp */
  &slice_traverse_children,                   /* STLeafForced */
  &slice_traverse_children,                   /* STReciprocal */
  &slice_traverse_children,                   /* STQuodlibet */
  &slice_traverse_children,                   /* STNot */
  &pipe_traverse_next,                        /* STMoveInverterRootSolvableFilter */
  &pipe_traverse_next,                        /* STMoveInverterSolvableFilter */
  &pipe_traverse_next,                        /* STMoveInverterSeriesFilter */
  &attack_root_make_setplay_slice,            /* STAttackRoot */
  &defense_root_make_setplay_slice, /* STDefenseRoot */
  &slice_traverse_children,                   /* STAttackHashed */
  &help_root_make_setplay_slice,              /* STHelpRoot */
  &pipe_traverse_next,                        /* STHelpShortcut */
  &slice_traverse_children,                   /* STHelpHashed */
  &series_root_make_setplay_slice,            /* STSeriesRoot */
  &pipe_traverse_next,                        /* STSeriesShortcut */
  &slice_traverse_children,                   /* STParryFork */
  &slice_traverse_children,                   /* STSeriesHashed */
  &pipe_traverse_next,                        /* STSelfCheckGuardRootSolvableFilter */
  &pipe_traverse_next,                        /* STSelfCheckGuardSolvableFilter */
  &pipe_traverse_next,                        /* STSelfCheckGuardRootDefenderFilter */
  &pipe_traverse_next,                        /* STSelfCheckGuardAttackerFilter */
  &pipe_traverse_next,                        /* STSelfCheckGuardDefenderFilter */
  &pipe_traverse_next,                        /* STSelfCheckGuardHelpFilter */
  &pipe_traverse_next,                        /* STSelfCheckGuardSeriesFilter */
  &direct_defense_root_make_setplay_slice,    /* STDirectDefense */
  &slice_traverse_children,                   /* STReflexHelpFilter */
  &slice_traverse_children,                   /* STReflexSeriesFilter */
  &reflex_attacker_filter_make_setplay_slice, /* STReflexAttackerFilter */
  &reflex_defender_filter_make_setplay_slice, /* STReflexDefenderFilter */
  &self_attack_root_make_setplay_slice,       /* STSelfAttack */
  &slice_traverse_children,                   /* STSelfDefense */
  &pipe_traverse_next,                        /* STRestartGuardRootDefenderFilter */
  &pipe_traverse_next,                        /* STRestartGuardHelpFilter */
  &pipe_traverse_next,                        /* STRestartGuardSeriesFilter */
  &slice_traverse_children,                   /* STIntelligentHelpFilter */
  &slice_traverse_children,                   /* STIntelligentSeriesFilter */
  &slice_traverse_children,                   /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,                   /* STGoalReachableGuardSeriesFilter */
  &pipe_traverse_next,                        /* STKeepMatingGuardRootDefenderFilter */
  &pipe_traverse_next,                        /* STKeepMatingGuardAttackerFilter */
  &pipe_traverse_next,                        /* STKeepMatingGuardDefenderFilter */
  &pipe_traverse_next,                        /* STKeepMatingGuardHelpFilter */
  &pipe_traverse_next,                        /* STKeepMatingGuardSeriesFilter */
  &pipe_traverse_next,                        /* STMaxFlightsquares */
  &slice_traverse_children,                   /* STDegenerateTree */
  &pipe_traverse_next,                        /* STMaxNrNonTrivial */
  &pipe_traverse_next,                        /* STMaxThreatLength */
  &pipe_traverse_next,                        /* STMaxTimeRootDefenderFilter */
  &pipe_traverse_next,                        /* STMaxTimeDefenderFilter */
  &pipe_traverse_next,                        /* STMaxTimeHelpFilter */
  &pipe_traverse_next,                        /* STMaxTimeSeriesFilter */
  &pipe_traverse_next,                        /* STMaxSolutionsRootSolvableFilter */
  &pipe_traverse_next,                        /* STMaxSolutionsRootDefenderFilter */
  &pipe_traverse_next,                        /* STMaxSolutionsHelpFilter */
  &pipe_traverse_next,                        /* STMaxSolutionsSeriesFilter */
  &pipe_traverse_next,                        /* STStopOnShortSolutionsRootSolvableFilter */
  &pipe_traverse_next,                        /* STStopOnShortSolutionsHelpFilter */
  &pipe_traverse_next                         /* STStopOnShortSolutionsSeriesFilter */
};

/* Combine the set play slices into the current stipulation
 * @param setplay slice index of set play
 * @param sibling slice whose sibling slice setplay is to be installed
 *                 as
 */
static void combine_set_play(slice_index setplay_slice, slice_index sibling)
{
  slice_index mi;
  slice_index op1;
  slice_index op2;
  slice_index const hook = slices[sibling].prev;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",setplay_slice);
  TraceFunctionParam("%u",sibling);
  TraceFunctionParamListEnd();

  mi = alloc_move_inverter_root_solvable_filter();
  if (slices[setplay_slice].prev==no_slice)
    pipe_link(mi,setplay_slice);
  else
    pipe_set_successor(mi,setplay_slice);

  op1 = alloc_proxy_slice();
  pipe_link(op1,mi);

  op2 = alloc_proxy_slice();
  pipe_link(op2,slices[hook].u.pipe.next);

  pipe_set_successor(hook,alloc_quodlibet_slice(op1,op2));

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Attempt to add set play to the stipulation
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(void)
{
  boolean result;
  setplay_slice_production prod = { no_slice, no_slice };
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&setplay_makers,&prod);
  traverse_slices(root_slice,&st);

  if (prod.setplay_slice==no_slice)
    result = false;
  else
  {
    assert(prod.sibling!=no_slice);
    combine_set_play(prod.setplay_slice,prod.sibling);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

typedef struct
{
    Goal const * const goals;
    size_t const nrGoals;
    boolean * const doGoalsMatch;
} goal_set;

/* Does a leaf have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si leaf slice identifier
 * @return true iff the leaf has as goal one of the elements of goals.
 */
static boolean leaf_ends_in_one_of(Goal const goals[],
                                   size_t nrGoals,
                                   slice_index si)
{
  Goal const goal = slices[si].u.leaf.goal;

  size_t i;
  for (i = 0; i<nrGoals; ++i)
    if (goal==goals[i])
      return true;

  return false;
}

static boolean leaf_ends_only_in(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  goal_set const * const goals = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goals->doGoalsMatch = (*goals->doGoalsMatch
                          && leaf_ends_in_one_of(goals->goals,goals->nrGoals,
                                                 si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_ends_only_in_checkers[] =
{
  &slice_traverse_children, /* STProxy */
  &slice_traverse_children, /* STAttackMove */
  &slice_traverse_children, /* STDefenseMove */
  &slice_traverse_children, /* STHelpMove */
  &slice_traverse_children, /* STHelpFork */
  &slice_traverse_children, /* STSeriesMove */
  &slice_traverse_children, /* STSeriesFork */
  &leaf_ends_only_in,       /* STLeafDirect */
  &leaf_ends_only_in,       /* STLeafHelp */
  &leaf_ends_only_in,       /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSeriesFilter */
  &slice_traverse_children, /* STAttackRoot */
  &slice_traverse_children, /* STDefenseRoot */
  &slice_traverse_children, /* STAttackHashed */
  &slice_traverse_children, /* STHelpRoot */
  &slice_traverse_children, /* STHelpShortcut */
  &slice_traverse_children, /* STHelpHashed */
  &slice_traverse_children, /* STSeriesRoot */
  &slice_traverse_children, /* STSeriesShortcut */
  &slice_traverse_children, /* STParryFork */
  &slice_traverse_children, /* STSeriesHashed */
  &slice_traverse_children, /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children, /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children, /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children, /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children, /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children, /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children, /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children, /* STDirectDefense */
  &slice_traverse_children, /* STReflexHelpFilter */
  &slice_traverse_children, /* STReflexSeriesFilter */
  &slice_traverse_children, /* STReflexAttackerFilter */
  &slice_traverse_children, /* STReflexDefenderFilter */
  &slice_traverse_children, /* STSelfAttack */
  &slice_traverse_children, /* STSelfDefense */
  &slice_traverse_children, /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children, /* STRestartGuardHelpFilter */
  &slice_traverse_children, /* STRestartGuardSeriesFilter */
  &slice_traverse_children, /* STIntelligentHelpFilter */
  &slice_traverse_children, /* STIntelligentSeriesFilter */
  &slice_traverse_children, /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children, /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children, /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children, /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children, /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children, /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children, /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children, /* STMaxFlightsquares */
  &slice_traverse_children, /* STDegenerateTree */
  &slice_traverse_children, /* STMaxNrNonTrivial */
  &slice_traverse_children, /* STMaxThreatLength */
  &slice_traverse_children, /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children, /* STMaxTimeDefenderFilter */
  &slice_traverse_children, /* STMaxTimeHelpFilter */
  &slice_traverse_children, /* STMaxTimeSeriesFilter */
  &slice_traverse_children, /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children, /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children, /* STMaxSolutionsHelpFilter */
  &slice_traverse_children, /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children, /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Do all leaves of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff all leaves have as goal one of the elements of goals.
 */
boolean stip_ends_only_in(Goal const goals[], size_t nrGoals)
{
  boolean result = true; /* until traversal proves otherwise */
  goal_set set = { goals, nrGoals, &result };
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nrGoals);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&slice_ends_only_in_checkers,&set);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean slice_ends_in_one_of_leaf(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  goal_set const * const goals = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goals->doGoalsMatch = (*goals->doGoalsMatch
                          || leaf_ends_in_one_of(goals->goals,goals->nrGoals,
                                                 si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_ends_in_one_of_checkers[] =
{
  &slice_traverse_children,   /* STProxy */
  &slice_traverse_children,   /* STAttackMove */
  &slice_traverse_children,   /* STDefenseMove */
  &slice_traverse_children,   /* STHelpMove */
  &slice_traverse_children,   /* STHelpFork */
  &slice_traverse_children,   /* STSeriesMove */
  &slice_traverse_children,   /* STSeriesFork */
  &slice_ends_in_one_of_leaf, /* STLeafDirect */
  &slice_ends_in_one_of_leaf, /* STLeafHelp */
  &slice_ends_in_one_of_leaf, /* STLeafForced */
  &slice_traverse_children,   /* STReciprocal */
  &slice_traverse_children,   /* STQuodlibet */
  &slice_traverse_children,   /* STNot */
  &slice_traverse_children,   /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,   /* STMoveInverterSolvableFilter */
  &slice_traverse_children,   /* STMoveInverterSeriesFilter */
  &slice_traverse_children,   /* STAttackRoot */
  &slice_traverse_children,   /* STDefenseRoot */
  &slice_traverse_children,   /* STAttackHashed */
  &slice_traverse_children,   /* STHelpRoot */
  &slice_traverse_children,   /* STHelpShortcut */
  &slice_traverse_children,   /* STHelpHashed */
  &slice_traverse_children,   /* STSeriesRoot */
  &slice_traverse_children,   /* STSeriesShortcut */
  &slice_traverse_children,   /* STParryFork */
  &slice_traverse_children,   /* STSeriesHashed */
  &slice_traverse_children,   /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,   /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,   /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,   /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,   /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,   /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,   /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,   /* STDirectDefense */
  &slice_traverse_children,   /* STReflexHelpFilter */
  &slice_traverse_children,   /* STReflexSeriesFilter */
  &slice_traverse_children,   /* STReflexAttackerFilter */
  &slice_traverse_children,   /* STReflexDefenderFilter */
  &slice_traverse_children,   /* STSelfAttack */
  &slice_traverse_children,   /* STSelfDefense */
  &slice_traverse_children,   /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,   /* STRestartGuardHelpFilter */
  &slice_traverse_children,   /* STRestartGuardSeriesFilter */
  &slice_traverse_children,   /* STIntelligentHelpFilter */
  &slice_traverse_children,   /* STIntelligentSeriesFilter */
  &slice_traverse_children,   /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,   /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,   /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,   /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,   /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,   /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,   /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,   /* STMaxFlightsquares */
  &slice_traverse_children,   /* STDegenerateTree */
  &slice_traverse_children,   /* STMaxNrNonTrivial */
  &slice_traverse_children,   /* STMaxThreatLength */
  &slice_traverse_children,   /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,   /* STMaxTimeDefenderFilter */
  &slice_traverse_children,   /* STMaxTimeHelpFilter */
  &slice_traverse_children,   /* STMaxTimeSeriesFilter */
  &slice_traverse_children,   /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,   /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,   /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,   /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,   /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,   /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children    /* STStopOnShortSolutionsSeriesFilter */
};

/* Does >= 1 leaf of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
boolean stip_ends_in_one_of(Goal const goals[], size_t nrGoals)
{
  boolean result = false;
  goal_set set = { goals, nrGoals, &result };
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nrGoals);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&slice_ends_in_one_of_checkers,&set);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param st address of structure defining traversal
 */
static boolean make_exact_branch(slice_index branch, slice_traversal *st)
{
  slices[branch].u.branch.min_length
      = slices[branch].u.branch.length;

  return slice_traverse_children(branch,st);
}

static slice_operation const exact_makers[] =
{
  &slice_traverse_children, /* STProxy */
  &make_exact_branch,       /* STAttackMove */
  &make_exact_branch,       /* STDefenseMove */
  &make_exact_branch,       /* STHelpMove */
  &make_exact_branch,       /* STHelpFork */
  &make_exact_branch,       /* STSeriesMove */
  &make_exact_branch,       /* STSeriesFork */
  &slice_traverse_children, /* STLeafDirect */
  &slice_traverse_children, /* STLeafHelp */
  &slice_traverse_children, /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSolvableFilter */
  &slice_traverse_children, /* STMoveInverterSeriesFilter */
  &make_exact_branch,       /* STAttackRoot */
  &make_exact_branch,       /* STDefenseRoot */
  &slice_traverse_children, /* STAttackHashed */
  &make_exact_branch,       /* STHelpRoot */
  &make_exact_branch,       /* STHelpShortcut */
  &slice_traverse_children, /* STHelpHashed */
  &make_exact_branch,       /* STSeriesRoot */
  &make_exact_branch,       /* STSeriesShortcut */
  &slice_traverse_children, /* STParryFork */
  &slice_traverse_children, /* STSeriesHashed */
  &make_exact_branch,       /* STSelfCheckGuardRootSolvableFilter */
  &make_exact_branch,       /* STSelfCheckGuardSolvableFilter */
  &make_exact_branch,       /* STSelfCheckGuardRootDefenderFilter */
  &make_exact_branch,       /* STSelfCheckGuardAttackerFilter */
  &make_exact_branch,       /* STSelfCheckGuardDefenderFilter */
  &make_exact_branch,       /* STSelfCheckGuardHelpFilter */
  &make_exact_branch,       /* STSelfCheckGuardSeriesFilter */
  &make_exact_branch,       /* STDirectDefense */
  &make_exact_branch,       /* STReflexHelpFilter */
  &make_exact_branch,       /* STReflexSeriesFilter */
  &make_exact_branch,       /* STReflexAttackerFilter */
  &make_exact_branch,       /* STReflexDefenderFilter */
  &make_exact_branch,       /* STSelfAttack */
  &make_exact_branch,       /* STSelfDefense */
  &make_exact_branch,       /* STRestartGuardRootDefenderFilter */
  &make_exact_branch,       /* STRestartGuardHelpFilter */
  &make_exact_branch,       /* STRestartGuardSeriesFilter */
  &make_exact_branch,       /* STIntelligentHelpFilter */
  &make_exact_branch,       /* STIntelligentSeriesFilter */
  &make_exact_branch,       /* STGoalReachableGuardHelpFilter */
  &make_exact_branch,       /* STGoalReachableGuardSeriesFilter */
  &make_exact_branch,       /* STKeepMatingGuardRootDefenderFilter */
  &make_exact_branch,       /* STKeepMatingGuardAttackerFilter */
  &make_exact_branch,       /* STKeepMatingGuardDefenderFilter */
  &make_exact_branch,       /* STKeepMatingGuardHelpFilter */
  &make_exact_branch,       /* STKeepMatingGuardSeriesFilter */
  &make_exact_branch,       /* STMaxFlightsquares */
  &make_exact_branch,       /* STDegenerateTree */
  &make_exact_branch,       /* STMaxNrNonTrivial */
  &make_exact_branch,       /* STMaxThreatLength */
  &make_exact_branch,       /* STMaxTimeRootDefenderFilter */
  &make_exact_branch,       /* STMaxTimeDefenderFilter */
  &make_exact_branch,       /* STMaxTimeHelpFilter */
  &make_exact_branch,       /* STMaxTimeSeriesFilter */
  &make_exact_branch,       /* STMaxSolutionsRootSolvableFilter */
  &make_exact_branch,       /* STMaxSolutionsRootDefenderFilter */
  &make_exact_branch,       /* STMaxSolutionsHelpFilter */
  &make_exact_branch,       /* STMaxSolutionsSeriesFilter */
  &make_exact_branch,       /* STStopOnShortSolutionsRootSolvableFilter */
  &make_exact_branch,       /* STStopOnShortSolutionsHelpFilter */
  &make_exact_branch        /* STStopOnShortSolutionsSeriesFilter */
};

/* Make the stipulation exact
 */
void stip_make_exact(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&exact_makers,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const starter_detectors[] =
{
  &pipe_detect_starter,                   /* STProxy */
  &attack_move_detect_starter,            /* STAttackMove */
  &slice_traverse_children,               /* STDefenseMove */
  &help_move_detect_starter,              /* STHelpMove */
  &branch_fork_detect_starter,            /* STHelpFork */
  &series_move_detect_starter,            /* STSeriesMove */
  &branch_fork_detect_starter,            /* STSeriesFork */
  &leaf_d_detect_starter,                 /* STLeafDirect */
  &leaf_h_detect_starter,                 /* STLeafHelp */
  &leaf_forced_detect_starter,            /* STLeafForced */
  &reci_detect_starter,                   /* STReciprocal */
  &quodlibet_detect_starter,              /* STQuodlibet */
  &pipe_detect_starter,                   /* STNot */
  &move_inverter_detect_starter,          /* STMoveInverterRootSolvableFilter */
  &move_inverter_detect_starter,          /* STMoveInverterSolvableFilter */
  &move_inverter_detect_starter,          /* STMoveInverterSeriesFilter */
  &attack_move_detect_starter,            /* STAttackRoot */
  &defense_root_detect_starter, /* STDefenseRoot */
  &slice_traverse_children,               /* STAttackHashed */
  &pipe_detect_starter,                   /* STHelpRoot */
  &pipe_detect_starter,                   /* STHelpShortcut */
  &slice_traverse_children,               /* STHelpHashed */
  &pipe_detect_starter,                   /* STSeriesRoot */
  &pipe_detect_starter,                   /* STSeriesShortcut */
  &pipe_detect_starter,                   /* STParryFork */
  &slice_traverse_children,               /* STSeriesHashed */
  &slice_traverse_children,               /* STSelfCheckGuardRootSolvableFilter */
  &slice_traverse_children,               /* STSelfCheckGuardSolvableFilter */
  &slice_traverse_children,               /* STSelfCheckGuardRootDefenderFilter */
  &slice_traverse_children,               /* STSelfCheckGuardAttackerFilter */
  &slice_traverse_children,               /* STSelfCheckGuardDefenderFilter */
  &slice_traverse_children,               /* STSelfCheckGuardHelpFilter */
  &slice_traverse_children,               /* STSelfCheckGuardSeriesFilter */
  &pipe_detect_starter,                   /* STDirectDefense */
  &pipe_detect_starter,                   /* STReflexHelpFilter */
  &pipe_detect_starter,                   /* STReflexSeriesFilter */
  &pipe_detect_starter,                   /* STReflexAttackerFilter */
  &pipe_detect_starter,                   /* STReflexDefenderFilter */
  &pipe_detect_starter,                   /* STSelfAttack */
  &pipe_detect_starter,                   /* STSelfDefense */
  &slice_traverse_children,               /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,               /* STRestartGuardHelpFilter */
  &slice_traverse_children,               /* STRestartGuardSeriesFilter */
  &slice_traverse_children,               /* STIntelligentHelpFilter */
  &slice_traverse_children,               /* STIntelligentSeriesFilter */
  &slice_traverse_children,               /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,               /* STGoalReachableGuardSeriesFilter */
  &slice_traverse_children,               /* STKeepMatingGuardRootDefenderFilter */
  &slice_traverse_children,               /* STKeepMatingGuardAttackerFilter */
  &slice_traverse_children,               /* STKeepMatingGuardDefenderFilter */
  &slice_traverse_children,               /* STKeepMatingGuardHelpFilter */
  &slice_traverse_children,               /* STKeepMatingGuardSeriesFilter */
  &slice_traverse_children,               /* STMaxFlightsquares */
  &slice_traverse_children,               /* STDegenerateTree */
  &slice_traverse_children,               /* STMaxNrNonTrivial */
  &slice_traverse_children,               /* STMaxThreatLength */
  &slice_traverse_children,               /* STMaxTimeRootDefenderFilter */
  &slice_traverse_children,               /* STMaxTimeDefenderFilter */
  &slice_traverse_children,               /* STMaxTimeHelpFilter */
  &slice_traverse_children,               /* STMaxTimeSeriesFilter */
  &slice_traverse_children,               /* STMaxSolutionsRootSolvableFilter */
  &slice_traverse_children,               /* STMaxSolutionsRootDefenderFilter */
  &slice_traverse_children,               /* STMaxSolutionsHelpFilter */
  &slice_traverse_children,               /* STMaxSolutionsSeriesFilter */
  &slice_traverse_children,               /* STStopOnShortSolutionsRootSolvableFilter */
  &slice_traverse_children,               /* STStopOnShortSolutionsHelpFilter */
  &slice_traverse_children                /* STStopOnShortSolutionsSeriesFilter */
};

/* Detect the starting side from the stipulation
 */
void stip_detect_starter(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&starter_detectors,NULL);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const starter_imposers[] =
{
  &pipe_impose_starter,           /* STProxy */
  &pipe_impose_inverted_starter,  /* STAttackMove */
  &pipe_impose_inverted_starter,  /* STDefenseMove */
  &pipe_impose_inverted_starter,  /* STHelpMove */
  &branch_fork_impose_starter,    /* STHelpFork */
  &pipe_impose_inverted_starter,  /* STSeriesMove */
  &branch_fork_impose_starter,    /* STSeriesFork */
  &leaf_impose_starter,           /* STLeafDirect */
  &leaf_impose_starter,           /* STLeafHelp */
  &leaf_impose_starter,           /* STLeafForced */
  &reci_impose_starter,           /* STReciprocal */
  &quodlibet_impose_starter,      /* STQuodlibet */
  &pipe_impose_starter,           /* STNot */
  &pipe_impose_inverted_starter,  /* STMoveInverterRootSolvableFilter */
  &pipe_impose_inverted_starter,  /* STMoveInverterSolvableFilter */
  &pipe_impose_inverted_starter,  /* STMoveInverterSeriesFilter */
  &pipe_impose_inverted_starter,  /* STAttackRoot */
  &pipe_impose_inverted_starter,  /* STDefenseRoot */
  &pipe_impose_starter,           /* STAttackHashed */
  &pipe_impose_starter,           /* STHelpRoot */
  &pipe_impose_starter,           /* STHelpShortcut */
  &pipe_impose_starter,           /* STHelpHashed */
  &pipe_impose_starter,           /* STSeriesRoot */
  &pipe_impose_starter,           /* STSeriesShortcut */
  &branch_fork_impose_starter,    /* STParryFork */
  &pipe_impose_starter,           /* STSeriesHashed */
  &pipe_impose_starter,           /* STSelfCheckGuardRootSolvableFilter */
  &pipe_impose_starter,           /* STSelfCheckGuardSolvableFilter */
  &pipe_impose_starter,           /* STSelfCheckGuardRootDefenderFilter */
  &pipe_impose_starter,           /* STSelfCheckGuardAttackerFilter */
  &pipe_impose_starter,           /* STSelfCheckGuardDefenderFilter */
  &pipe_impose_starter,           /* STSelfCheckGuardHelpFilter */
  &pipe_impose_starter,           /* STSelfCheckGuardSeriesFilter */
  &direct_defense_impose_starter, /* STDirectDefense */
  &reflex_filter_impose_starter,  /* STReflexHelpFilter */
  &reflex_filter_impose_starter,  /* STReflexSeriesFilter */
  &reflex_filter_impose_starter,  /* STReflexAttackerFilter */
  &reflex_filter_impose_starter,  /* STReflexDefenderFilter */
  &self_attack_impose_starter,    /* STSelfAttack */
  &self_defense_impose_starter,   /* STSelfDefense */
  &pipe_impose_starter,           /* STRestartGuardRootDefenderFilter */
  &pipe_impose_starter,           /* STRestartGuardHelpFilter */
  &pipe_impose_starter,           /* STRestartGuardSeriesFilter */
  &pipe_impose_starter,           /* STIntelligentHelpFilter */
  &pipe_impose_starter,           /* STIntelligentSeriesFilter */
  &pipe_impose_starter,           /* STGoalReachableGuardHelpFilter */
  &pipe_impose_starter,           /* STGoalReachableGuardSeriesFilter */
  &pipe_impose_starter,           /* STKeepMatingGuardRootDefenderFilter */
  &pipe_impose_starter,           /* STKeepMatingGuardAttackerFilter */
  &pipe_impose_starter,           /* STKeepMatingGuardDefenderFilter */
  &pipe_impose_starter,           /* STKeepMatingGuardHelpFilter */
  &pipe_impose_starter,           /* STKeepMatingGuardSeriesFilter */
  &pipe_impose_starter,           /* STMaxFlightsquares */
  &pipe_impose_starter,           /* STDegenerateTree */
  &pipe_impose_starter,           /* STMaxNrNonTrivial */
  &pipe_impose_starter,           /* STMaxThreatLength */
  &pipe_impose_starter,           /* STMaxTimeRootDefenderFilter */
  &pipe_impose_starter,           /* STMaxTimeDefenderFilter */
  &pipe_impose_starter,           /* STMaxTimeHelpFilter */
  &pipe_impose_starter,           /* STMaxTimeSeriesFilter */
  &pipe_impose_starter,           /* STMaxSolutionsRootSolvableFilter */
  &pipe_impose_starter,           /* STMaxSolutionsRootDefenderFilter */
  &pipe_impose_starter,           /* STMaxSolutionsHelpFilter */
  &pipe_impose_starter,           /* STMaxSolutionsSeriesFilter */
  &pipe_impose_starter,           /* STStopOnShortSolutionsRootSolvableFilter */
  &pipe_impose_starter,           /* STStopOnShortSolutionsHelpFilter */
  &pipe_impose_starter            /* STStopOnShortSolutionsSeriesFilter */
};

/* Set the starting side of the stipulation
 */
void stip_impose_starter(Side starter)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",starter);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&starter_imposers,&starter);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
boolean slice_operation_noop(slice_index si, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",true);
  TraceFunctionResultEnd();
  return true;
}

/* Dispatch an operation to a slice based on the slice's type
 * @param si identifies slice
 * @param st address of structure defining traversal
 * @return true iff dispatched operation returned true
 */
static boolean dispatch_to_slice(slice_index si,
                                 operation_mapping ops,
                                 slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    slice_operation const operation = (*ops)[slices[si].type];
    assert(operation!=0);
    result = (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise a slice_traversal structure
 * @param st to be initialised
 * @param ops operations to be invoked on slices
 * @param param parameter to be passed t operations
 */
void slice_traversal_init(slice_traversal *st,
                          operation_mapping ops,
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
slice_traversal_slice_state
get_slice_traversal_slice_state(slice_index si, slice_traversal *st)
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

/* (Approximately) depth-first traversl of the stipulation
 * @param ops mapping from slice types to operations
 * @param param address of data structure holding parameters for the operation
 * @return true iff root and its children have been successfully
 *         traversed
 */
boolean traverse_slices(slice_index root, slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (st->traversed[root]==slice_not_traversed)
  {
    /* avoid infinite recursion */
    st->traversed[root] = slice_being_traversed;
    st->traversed[root] = (dispatch_to_slice(root,st->ops,st)
                           ? slice_traversed
                           : slice_not_traversed);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",st->traversed[root]==slice_traversed);
  TraceFunctionResultEnd();
  return st->traversed[root]==slice_traversed;
}

/* Traverse a subtree
 * @param fork root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff fork and its children have been successfully
 *         traversed
 */
static boolean traverse_fork(slice_index fork, slice_traversal *st)
{
  boolean result1;
  boolean result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  result1 = traverse_slices(slices[fork].u.binary.op1,st);
  result2 = traverse_slices(slices[fork].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u", result1 && result2);
  TraceFunctionResultEnd();
  return result1 && result2;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff pipe and its children have been successfully
 *         traversed
 */
static boolean traverse_pipe(slice_index pipe, slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  result = traverse_slices(slices[pipe].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff pipe and its children have been successfully
 *         traversed
 */
static boolean traverse_shortcut(slice_index pipe, slice_traversal *st)
{
  boolean result;
  boolean result1;
  boolean result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  result1 = traverse_slices(slices[pipe].u.pipe.next,st);
  result2 = traverse_slices(slices[pipe].u.shortcut.short_sols,st);
  result = result1 && result2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff branch_fork and its children have been successfully
 *         traversed
 */
static boolean traverse_branch_fork(slice_index branch, slice_traversal *st)
{
  boolean const result_pipe = traverse_pipe(branch,st);
  slice_index const towards_goal = slices[branch].u.branch_fork.towards_goal;
  boolean const result_toward_goal = traverse_slices(towards_goal,st);
  return result_pipe && result_toward_goal;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff branch_fork and its children have been successfully
 *         traversed
 */
static boolean traverse_parry_fork(slice_index branch, slice_traversal *st)
{
  boolean const result_pipe = traverse_pipe(branch,st);
  slice_index const parrying = slices[branch].u.parry_fork.parrying;
  boolean const result_parrying = traverse_slices(parrying,st);
  return result_pipe && result_parrying;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff branch_fork and its children have been successfully
 *         traversed
 */
static boolean traverse_reflex_guard(slice_index branch, slice_traversal *st)
{
  boolean const result_pipe = traverse_pipe(branch,st);
  slice_index const avoided = slices[branch].u.reflex_guard.avoided;
  boolean const result_not_slice = traverse_slices(avoided,st);
  return result_pipe && result_not_slice;
}

static slice_operation const traversers[] =
{
  &traverse_pipe,         /* STProxy */
  &traverse_pipe,         /* STAttackMove */
  &traverse_pipe,         /* STDefenseMove */
  &traverse_pipe,         /* STHelpMove */
  &traverse_branch_fork,  /* STHelpFork */
  &traverse_pipe,         /* STSeriesMove */
  &traverse_branch_fork,  /* STSeriesFork */
  &slice_operation_noop,  /* STLeafDirect */
  &slice_operation_noop,  /* STLeafHelp */
  &slice_operation_noop,  /* STLeafForced */
  &traverse_fork,         /* STReciprocal */
  &traverse_fork,         /* STQuodlibet */
  &traverse_pipe,         /* STNot */
  &traverse_pipe,         /* STMoveInverterRootSolvableFilter */
  &traverse_pipe,         /* STMoveInverterSolvableFilter */
  &traverse_pipe,         /* STMoveInverterSeriesFilter */
  &traverse_pipe,         /* STAttackRoot */
  &traverse_pipe,         /* STDefenseRoot */
  &traverse_pipe,         /* STAttackHashed */
  &traverse_pipe,         /* STHelpRoot */
  &traverse_shortcut,     /* STHelpShortcut */
  &traverse_pipe,         /* STHelpHashed */
  &traverse_pipe,         /* STSeriesRoot */
  &traverse_shortcut,     /* STSeriesShortcut */
  &traverse_parry_fork,   /* STParryFork */
  &traverse_pipe,         /* STSeriesHashed */
  &traverse_pipe,         /* STSelfCheckGuardRootSolvableFilter */
  &traverse_pipe,         /* STSelfCheckGuardSolvableFilter */
  &traverse_pipe,         /* STSelfCheckGuardRootDefenderFilter */
  &traverse_pipe,         /* STSelfCheckGuardAttackerFilter */
  &traverse_pipe,         /* STSelfCheckGuardDefenderFilter */
  &traverse_pipe,         /* STSelfCheckGuardHelpFilter */
  &traverse_pipe,         /* STSelfCheckGuardSeriesFilter */
  &traverse_branch_fork,  /* STDirectDefense */
  &traverse_reflex_guard, /* STReflexHelpFilter */
  &traverse_reflex_guard, /* STReflexSeriesFilter */
  &traverse_reflex_guard, /* STReflexAttackerFilter */
  &traverse_reflex_guard, /* STReflexDefenderFilter */
  &traverse_branch_fork,  /* STSelfAttack */
  &traverse_branch_fork,  /* STSelfDefense */
  &traverse_pipe,         /* STRestartGuardRootDefenderFilter */
  &traverse_pipe,         /* STRestartGuardHelpFilter */
  &traverse_pipe,         /* STRestartGuardSeriesFilter */
  &traverse_pipe,         /* STIntelligentHelpFilter */
  &traverse_pipe,         /* STIntelligentSeriesFilter */
  &traverse_pipe,         /* STGoalReachableGuardHelpFilter */
  &traverse_pipe,         /* STGoalReachableGuardSeriesFilter */
  &traverse_pipe,         /* STKeepMatingGuardRootDefenderFilter */
  &traverse_pipe,         /* STKeepMatingGuardAttackerFilter */
  &traverse_pipe,         /* STKeepMatingGuardDefenderFilter */
  &traverse_pipe,         /* STKeepMatingGuardHelpFilter */
  &traverse_pipe,         /* STKeepMatingGuardSeriesFilter */
  &traverse_pipe,         /* STMaxFlightsquares */
  &traverse_pipe,         /* STDegenerateTree */
  &traverse_pipe,         /* STMaxNrNonTrivial */
  &traverse_pipe,         /* STMaxThreatLength */
  &traverse_pipe,         /* STMaxTimeRootDefenderFilter */
  &traverse_pipe,         /* STMaxTimeDefenderFilter */
  &traverse_pipe,         /* STMaxTimeHelpFilter */
  &traverse_pipe,         /* STMaxTimeSeriesFilter */
  &traverse_pipe,         /* STMaxSolutionsRootSolvableFilter */
  &traverse_pipe,         /* STMaxSolutionsRootDefenderFilter */
  &traverse_pipe,         /* STMaxSolutionsHelpFilter */
  &traverse_pipe,         /* STMaxSolutionsSeriesFilter */
  &traverse_pipe,         /* STStopOnShortSolutionsRootSolvableFilter */
  &traverse_pipe,         /* STStopOnShortSolutionsHelpFilter */
  &traverse_pipe          /* STStopOnShortSolutionsSeriesFilter */
};

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 * @return if the children of si have been successfully traversed
 */
boolean slice_traverse_children(slice_index si, slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = dispatch_to_slice(si,&traversers,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
