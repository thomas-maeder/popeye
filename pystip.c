#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
#include "pybrad.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybraser.h"
#include "pyleaf.h"
#include "pyrecipr.h"
#include "pyquodli.h"
#include "pybrafrk.h"
#include "pynot.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pymovein.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pydirctg.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME SliceType
#define ENUMERATORS \
  ENUMERATOR(STBranchDirect),      /* M-N moves of direct play */         \
    ENUMERATOR(STBranchDirectDefender),                                 \
    ENUMERATOR(STBranchHelp),      /* M-N moves of help play */         \
    ENUMERATOR(STBranchSeries),    /* M-N moves of series play */       \
    ENUMERATOR(STBranchFork),      /* decides when play in branch is over */ \
                                                                        \
    ENUMERATOR(STLeafDirect),      /* goal in 1 */                      \
    ENUMERATOR(STLeafHelp),        /* help-goal in 1 */                 \
    ENUMERATOR(STLeafForced),      /* forced goal in 1 half move */     \
                                                                        \
    ENUMERATOR(STReciprocal),      /* logical AND */                    \
    ENUMERATOR(STQuodlibet),       /* logical OR */                     \
    ENUMERATOR(STNot),             /* logical NOT */                    \
                                                                        \
    ENUMERATOR(STMoveInverter),    /* 0 length, inverts side at move */ \
                                                                        \
    ENUMERATOR(STDirectRoot),      /* root level of direct play */        \
    ENUMERATOR(STDirectDefenderRoot), /* root level of postkey direct play */ \
    ENUMERATOR(STDirectHashed),    /* direct play with hash table */      \
                                                         \
    ENUMERATOR(STHelpRoot),        /* root level of help play */        \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
                                                                        \
    ENUMERATOR(STSeriesRoot),      /* root level of series play */        \
    ENUMERATOR(STSeriesHashed),    /* series play with hash table */      \
                                                    \
    ENUMERATOR(STSelfCheckGuard),  /* stop when a side exposes its king */ \
                                                                        \
    ENUMERATOR(STDirectDefense),   /* direct play, just played defense */ \
    ENUMERATOR(STReflexGuard),     /* stop when wrong side can reach goal */ \
    ENUMERATOR(STSelfAttack),      /* self play, just played attack */  \
    ENUMERATOR(STSelfDefense),     /* self play, just played defense */  \
                                                                        \
    ENUMERATOR(STRestartGuard),    /* stop when wrong side can reach goal */ \
                                                                        \
    ENUMERATOR(STGoalReachableGuard), /* deals with intelligent mode */ \
    ENUMERATOR(STKeepMatingGuard), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */ \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
                                                                        \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


Slice slices[max_nr_slices];

slice_index root_slice;

static slice_index free_indices[max_nr_slices];

static slice_index first_free_index;

static boolean mark_reachable_slice(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  boolean (* const leaked)[max_nr_slices] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*leaked)[si] = false;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const reachable_slices_markers[] =
{
  &mark_reachable_slice, /* STBranchDirect */
  &mark_reachable_slice, /* STBranchDirectDefender */
  &mark_reachable_slice, /* STBranchHelp */
  &mark_reachable_slice, /* STBranchSeries */
  &mark_reachable_slice, /* STBranchFork */
  &mark_reachable_slice, /* STLeafDirect */
  &mark_reachable_slice, /* STLeafHelp */
  &mark_reachable_slice, /* STLeafForced */
  &mark_reachable_slice, /* STReciprocal */
  &mark_reachable_slice, /* STQuodlibet */
  &mark_reachable_slice, /* STNot */
  &mark_reachable_slice, /* STMoveInverter */
  &mark_reachable_slice, /* STDirectRoot */
  &mark_reachable_slice, /* STDirectDefenderRoot */
  &mark_reachable_slice, /* STDirectHashed */
  &mark_reachable_slice, /* STHelpRoot */
  &mark_reachable_slice, /* STHelpHashed */
  &mark_reachable_slice, /* STSeriesRoot */
  &mark_reachable_slice, /* STSeriesHashed */
  &mark_reachable_slice, /* STSelfCheckGuard */
  &mark_reachable_slice, /* STDirectDefense */
  &mark_reachable_slice, /* STReflexGuard */
  &mark_reachable_slice, /* STSelfAttack */
  &mark_reachable_slice, /* STSelfDefense */
  &mark_reachable_slice, /* STRestartGuard */
  &mark_reachable_slice, /* STGoalReachableGuard */
  &mark_reachable_slice, /* STKeepMatingGuard */
  &mark_reachable_slice, /* STMaxFlightsquares */
  &mark_reachable_slice, /* STDegenerateTree */
  &mark_reachable_slice, /* STMaxNrNonTrivial */
  &mark_reachable_slice  /* STMaxThreatLength */
};

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slice_indices(void)
{
  boolean leaked[max_nr_slices];
  unsigned int i;
  slice_traversal st;

  for (i = 0; i!=max_nr_slices; ++i)
    leaked[i] = true;

  for (i = first_free_index; i!=max_nr_slices; ++i)
    leaked[free_indices[i]] = false;

  slice_traversal_init(&st,&reachable_slices_markers,&leaked);
  traverse_slices(root_slice,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    assert(!leaked[i]);
}

/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
void init_slice_index_allocator(void)
{
  release_slices();
}

/* Allocate a slice index
 * @return a so far unused slice index
 */
slice_index alloc_slice_index(void)
{
  slice_index const result = free_indices[first_free_index++];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(first_free_index<=max_nr_slices);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice_index(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].type==STBranchDirect)
    dealloc_slice_index(slices[si].u.pipe.next);

  assert(first_free_index>0);
  free_indices[--first_free_index] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(SliceType type, square s)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(type==STLeafDirect
         || type==STLeafHelp
         || type==STLeafForced);

  slices[result].type = type; 
  slices[result].starter = no_side; 
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
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",type);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  assert(type==STLeafDirect
         || type==STLeafHelp
         || type==STLeafForced);

  slices[result].type = type; 
  slices[result].starter = no_side; 
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

  result = alloc_slice_index();
  slices[result] = slices[original];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Release all slices
 */
void release_slices(void)
{
  slice_index si;
  for (si = 0; si!=max_nr_slices; ++si)
    free_indices[si] = si;

  first_free_index = 0;

  root_slice = no_slice;
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
      result = slices[si].u.pipe.u.branch.min_length;
      min_length *= 2;
      if (result%2==1)
        --min_length;
      if (min_length<=slices[si].u.pipe.u.branch.length)
        slices[si].u.pipe.u.branch.min_length = min_length;
      break;

    case STSeriesRoot:
      result = slices[si].u.pipe.u.branch.min_length;
      if (min_length+1<=slices[si].u.pipe.u.branch.length)
        slices[si].u.pipe.u.branch.min_length = min_length+1;
      break;

    case STBranchDirect:
      result = slices[si].u.pipe.u.branch.min_length;
      break;

    case STSelfCheckGuard:
    case STReflexGuard:
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
  *max_nr += slices[si].u.pipe.u.branch.length+1;

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

  result = traverse_slices(slices[si].u.pipe.u.branch.towards_goal,st);
  *max_nr += slices[si].u.pipe.u.branch.length;

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

  result = traverse_slices(slices[si].u.pipe.u.branch.towards_goal,st);

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

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param address of structure representing traversal
 * @return true iff traversal of si was successful
 */
static boolean get_max_nr_moves_other(slice_index si, slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const get_max_nr_moves_functions[] =
{
  &get_max_nr_moves_other,           /* STBranchDirect */
  &get_max_nr_moves_direct_defender, /* STBranchDirectDefender */
  &get_max_nr_moves_branch,          /* STBranchHelp */
  &get_max_nr_moves_branch,          /* STBranchSeries */
  &get_max_nr_moves_branch_fork,     /* STBranchFork */
  &get_max_nr_moves_leaf,            /* STLeafDirect */
  &get_max_nr_moves_leaf,            /* STLeafHelp */
  &get_max_nr_moves_leaf,            /* STLeafForced */
  &get_max_nr_moves_other,           /* STReciprocal */
  &get_max_nr_moves_other,           /* STQuodlibet */
  &get_max_nr_moves_other,           /* STNot */
  &get_max_nr_moves_other,           /* STMoveInverter */
  &get_max_nr_moves_other,           /* STDirectRoot */
  &get_max_nr_moves_other,           /* STDirectDefenderRoot */
  &get_max_nr_moves_other,           /* STDirectHashed */
  &get_max_nr_moves_other,           /* STHelpRoot */
  &get_max_nr_moves_other,           /* STHelpHashed */
  &get_max_nr_moves_other,           /* STSeriesRoot */
  &get_max_nr_moves_other,           /* STSeriesHashed */
  &get_max_nr_moves_other,           /* STSelfCheckGuard */
  &get_max_nr_moves_other,           /* STDirectDefense */
  &get_max_nr_moves_other,           /* STReflexGuard */
  &get_max_nr_moves_other,           /* STSelfAttack */
  &get_max_nr_moves_other,           /* STSelfDefense */
  &get_max_nr_moves_other,           /* STRestartGuard */
  &get_max_nr_moves_other,           /* STGoalReachableGuard */
  &get_max_nr_moves_other,           /* STKeepMatingGuard */
  &get_max_nr_moves_other,           /* STMaxFlightsquares */
  &get_max_nr_moves_other,           /* STDegenerateTree */
  &get_max_nr_moves_other,           /* STMaxNrNonTrivial */
  &get_max_nr_moves_other            /* STMaxThreatLength */
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
  &slice_traverse_children, /* STBranchDirect */
  &slice_traverse_children, /* STBranchDirectDefender */
  &slice_traverse_children, /* STBranchHelp */
  &slice_traverse_children, /* STBranchSeries */
  &slice_traverse_children, /* STBranchFork */
  &find_unique_goal_leaf,   /* STLeafDirect */
  &find_unique_goal_leaf,   /* STLeafHelp */
  &find_unique_goal_leaf,   /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverter */
  &slice_traverse_children, /* STDirectRoot */
  &slice_traverse_children, /* STDirectDefenderRoot */
  &slice_traverse_children, /* STDirectHashed */
  &slice_traverse_children, /* STHelpRoot */
  &slice_traverse_children, /* STHelpHashed */
  &slice_traverse_children, /* STSeriesRoot */
  &slice_traverse_children, /* STSeriesHashed */
  &slice_traverse_children, /* STSelfCheckGuard */
  &slice_traverse_children, /* STDirectDefense */
  &slice_traverse_children, /* STReflexGuard */
  &slice_traverse_children, /* STSelfAttack */
  &slice_traverse_children, /* STSelfDefense */
  &slice_traverse_children, /* STRestartGuard */
  &slice_traverse_children, /* STGoalReachableGuard */
  &slice_traverse_children, /* STKeepMatingGuard */
  &slice_traverse_children, /* STMaxFlightsquares */
  &slice_traverse_children, /* STDegenerateTree */
  &slice_traverse_children, /* STMaxNrNonTrivial */
  &slice_traverse_children  /* STMaxThreatLength */
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
 * @param si address of root of sub-tree
 * @param copies address of array remembering what copies have already
 *               been made
 */
static void deep_copy_recursive(slice_index *si, copies_type *copies)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*si);
  TraceFunctionParamListEnd();

  if ((*copies)[*si]==no_slice)
  {
    (*copies)[*si] = copy_slice(*si);
    *si = (*copies)[*si];

    TraceEnumerator(SliceType,slices[*si].type,"\n");
    switch (slices[*si].type)
    {
      case STLeafDirect:
      case STLeafHelp:
      case STLeafForced:
        /* nothing */
        break;

      case STBranchDirect:
      case STBranchDirectDefender:
      case STBranchHelp:
      case STBranchSeries:
      case STBranchFork:
      case STDirectRoot:
      case STDirectDefenderRoot:
      case STSeriesRoot:
      case STDirectDefense:
      case STReflexGuard:
      case STSelfAttack:
      case STSelfDefense:
        deep_copy_recursive(&slices[*si].u.pipe.u.branch.towards_goal,copies);
        deep_copy_recursive(&slices[*si].u.pipe.next,copies);
        break;

      case STHelpRoot:
        deep_copy_recursive(&slices[*si].u.pipe.u.help_root.towards_goal,
                            copies);
        deep_copy_recursive(&slices[*si].u.pipe.next,copies);
        deep_copy_recursive(&slices[*si].u.pipe.u.help_root.short_sols,copies);
        break;

      case STNot:
      case STMoveInverter:
      case STDirectHashed:
      case STHelpHashed:
      case STSeriesHashed:
      case STSelfCheckGuard:
      case STRestartGuard:
      case STGoalReachableGuard:
      case STKeepMatingGuard:
      case STMaxFlightsquares:
      case STMaxNrNonTrivial:
      case STMaxThreatLength:
        deep_copy_recursive(&slices[*si].u.pipe.next,copies);
        break;

      case STQuodlibet:
      case STReciprocal:
        deep_copy_recursive(&slices[*si].u.fork.op1,copies);
        deep_copy_recursive(&slices[*si].u.fork.op2,copies);
        break;

      default:
        assert(0);
        break;
    }
  }
  else
    *si = (*copies)[*si];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
static slice_index deep_copy(slice_index si)
{
  copies_type copies;
  slice_index i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    copies[i] = no_slice;

  deep_copy_recursive(&si,&copies);


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",si);
  TraceFunctionResultEnd();
  return si;
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
  &slice_traverse_children,   /* STBranchDirect */
  &slice_traverse_children,   /* STBranchDirectDefender */
  &slice_traverse_children,   /* STBranchHelp */
  &slice_traverse_children,   /* STBranchSeries */
  &slice_traverse_children,   /* STBranchFork */
  &slice_operation_noop,      /* STLeafDirect */
  &make_leaf_direct,          /* STLeafHelp */
  &make_leaf_direct,          /* STLeafForced */
  &slice_traverse_children,   /* STReciprocal */
  &slice_traverse_children,   /* STQuodlibet */
  &slice_traverse_children,   /* STNot */
  &slice_traverse_children,   /* STMoveInverter */
  &slice_traverse_children,   /* STDirectRoot */
  &slice_traverse_children,   /* STDirectDefenderRoot */
  &slice_traverse_children,   /* STDirectHashed */
  &slice_traverse_children,   /* STHelpRoot */
  &slice_traverse_children,   /* STHelpHashed */
  &slice_traverse_children,   /* STSeriesRoot */
  &slice_traverse_children,   /* STSeriesHashed */
  &slice_traverse_children,   /* STSelfCheckGuard */
  &slice_traverse_children,   /* STDirectDefense */
  &slice_traverse_children,   /* STReflexGuard */
  &slice_traverse_children,   /* STSelfAttack */
  &slice_traverse_children,   /* STSelfDefense */
  &slice_traverse_children,   /* STRestartGuard */
  &slice_traverse_children,   /* STGoalReachableGuard */
  &slice_traverse_children,   /* STKeepMatingGuard */
  &slice_traverse_children,   /* STMaxFlightsquares */
  &slice_traverse_children,   /* STDegenerateTree */
  &slice_traverse_children,   /* STMaxNrNonTrivial */
  &slice_traverse_children    /* STMaxThreatLength */
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

static void make_successor_direct_defense(slice_index successor,
                                          slice_index towards_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",successor);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  if (slices[successor].type==STDirectDefense)
    /* already done*/ ;
  else
  {
    slices[successor].u.pipe.next = copy_slice(successor);
    slices[successor].type = STDirectDefense;
    slices[successor].u.pipe.u.branch.towards_goal = towards_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean transform_to_quodlibet_direct_root(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index * const new_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *new_to_goal = deep_copy(to_goal);
  make_leaves_direct(*new_to_goal);

  slice_traverse_children(si,st);

  *new_to_goal = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean transform_to_quodlibet_branch_direct_defender(slice_index si,
                                                             slice_traversal *st)
{
  boolean const result = true;
  slice_index successor = slices[si].u.pipe.next;
  slice_index const * const new_to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  assert(*new_to_goal!=no_slice);

  /* If we are instrumenting a s#N, append a STDirectDefense slice
   * after each STSelfDefense.
   * Otherwise (i.e. if we are instrumenting a semi-r#n), insert the
   * STDirectDefense slice directly after each STBranchDirectDefender
   * slice.
   */
  if (slices[successor].type==STSelfDefense)
    successor = slices[successor].u.pipe.next;
  make_successor_direct_defense(successor,*new_to_goal);

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
  &slice_traverse_children,                       /* STBranchDirect */
  &transform_to_quodlibet_branch_direct_defender, /* STBranchDirectDefender */
  0,                                              /* STBranchHelp */
  &slice_traverse_children,                       /* STBranchSeries */
  &transform_to_quodlibet_branch_fork,            /* STBranchFork */
  &slice_operation_noop,                          /* STLeafDirect */
  &slice_operation_noop,                          /* STLeafHelp */
  &slice_operation_noop,                          /* STLeafForced */
  &slice_traverse_children,                       /* STReciprocal */
  &slice_traverse_children,                       /* STQuodlibet */
  0,                                              /* STNot */
  0,                                              /* STMoveInverter */
  &transform_to_quodlibet_direct_root,            /* STDirectRoot */
  &transform_to_quodlibet_branch_direct_defender, /* STDirectDefenderRoot */
  0,                                              /* STDirectHashed */
  0,                                              /* STHelpRoot */
  0,                                              /* STHelpHashed */
  0,                                              /* STSeriesRoot */
  0,                                              /* STSeriesHashed */
  0,                                              /* STSelfCheckGuard */
  &slice_traverse_children,                       /* STDirectDefense */
  &transform_to_quodlibet_branch_fork,            /* STReflexGuard */
  &slice_traverse_children,                       /* STSelfAttack */
  &slice_traverse_children,                       /* STSelfDefense */
  0,                                              /* STRestartGuard */
  0,                                              /* STGoalReachableGuard */
  0,                                              /* STKeepMatingGuard */
  0,                                              /* STMaxFlightsquares */
  0,                                              /* STDegenerateTree */
  0,                                              /* STMaxNrNonTrivial */
  0                                               /* STMaxThreatLength */
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

  TraceStipulation();

  slice_traversal_init(&st,&to_quodlibet_transformers,&towards_goal);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  &slice_traverse_children, /* STBranchDirect */
  &slice_traverse_children, /* STBranchDirectDefender */
  &slice_traverse_children, /* STBranchHelp */
  &slice_traverse_children, /* STBranchSeries */
  &slice_traverse_children, /* STBranchFork */
  &leaf_ends_only_in,       /* STLeafDirect */
  &leaf_ends_only_in,       /* STLeafHelp */
  &leaf_ends_only_in,       /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverter */
  &slice_traverse_children, /* STDirectRoot */
  &slice_traverse_children, /* STDirectDefenderRoot */
  &slice_traverse_children, /* STDirectHashed */
  &slice_traverse_children, /* STHelpRoot */
  &slice_traverse_children, /* STHelpHashed */
  &slice_traverse_children, /* STSeriesRoot */
  &slice_traverse_children, /* STSeriesHashed */
  &slice_traverse_children, /* STSelfCheckGuard */
  &slice_traverse_children, /* STDirectDefense */
  &slice_traverse_children, /* STReflexGuard */
  &slice_traverse_children, /* STSelfAttack */
  &slice_traverse_children, /* STSelfDefense */
  &slice_traverse_children, /* STRestartGuard */
  &slice_traverse_children, /* STGoalReachableGuard */
  &slice_traverse_children, /* STKeepMatingGuard */
  &slice_traverse_children, /* STMaxFlightsquares */
  &slice_traverse_children, /* STDegenerateTree */
  &slice_traverse_children, /* STMaxNrNonTrivial */
  &slice_traverse_children  /* STMaxThreatLength */
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
  &slice_traverse_children,   /* STBranchDirect */
  &slice_traverse_children,   /* STBranchDirectDefender */
  &slice_traverse_children,   /* STBranchHelp */
  &slice_traverse_children,   /* STBranchSeries */
  &slice_traverse_children,   /* STBranchFork */
  &slice_ends_in_one_of_leaf, /* STLeafDirect */
  &slice_ends_in_one_of_leaf, /* STLeafHelp */
  &slice_ends_in_one_of_leaf, /* STLeafForced */
  &slice_traverse_children,   /* STReciprocal */
  &slice_traverse_children,   /* STQuodlibet */
  &slice_traverse_children,   /* STNot */
  &slice_traverse_children,   /* STMoveInverter */
  &slice_traverse_children,   /* STDirectRoot */
  &slice_traverse_children,   /* STDirectDefenderRoot */
  &slice_traverse_children,   /* STDirectHashed */
  &slice_traverse_children,   /* STHelpRoot */
  &slice_traverse_children,   /* STHelpHashed */
  &slice_traverse_children,   /* STSeriesRoot */
  &slice_traverse_children,   /* STSeriesHashed */
  &slice_traverse_children,   /* STSelfCheckGuard */
  &slice_traverse_children,   /* STDirectDefense */
  &slice_traverse_children,   /* STReflexGuard */
  &slice_traverse_children,   /* STSelfAttack */
  &slice_traverse_children,   /* STSelfDefense */
  &slice_traverse_children,   /* STRestartGuard */
  &slice_traverse_children,   /* STGoalReachableGuard */
  &slice_traverse_children,   /* STKeepMatingGuard */
  &slice_traverse_children,   /* STMaxFlightsquares */
  &slice_traverse_children,   /* STDegenerateTree */
  &slice_traverse_children,   /* STMaxNrNonTrivial */
  &slice_traverse_children    /* STMaxThreatLength */
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
  slices[branch].u.pipe.u.branch.min_length
      = slices[branch].u.pipe.u.branch.length;

  return slice_traverse_children(branch,st);
}

static slice_operation const exact_makers[] =
{
  &make_exact_branch,       /* STBranchDirect */
  &make_exact_branch,       /* STBranchDirectDefender */
  &make_exact_branch,       /* STBranchHelp */
  &make_exact_branch,       /* STBranchSeries */
  &make_exact_branch,       /* STBranchFork */
  &slice_traverse_children, /* STLeafDirect */
  &slice_traverse_children, /* STLeafHelp */
  &slice_traverse_children, /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverter */
  &make_exact_branch,       /* STDirectRoot */
  &make_exact_branch,       /* STDirectDefenderRoot */
  0,                        /* STDirectHashed */
  &make_exact_branch,       /* STHelpRoot */
  0,                        /* STHelpHashed */
  &make_exact_branch,       /* STSeriesRoot */
  0,                        /* STSeriesHashed */
  &make_exact_branch,       /* STSelfCheckGuard */
  &make_exact_branch,       /* STDirectDefense */
  &make_exact_branch,       /* STReflexGuard */
  &make_exact_branch,       /* STSelfAttack */
  &make_exact_branch,       /* STSelfDefense */
  &make_exact_branch,       /* STRestartGuard */
  &make_exact_branch,       /* STGoalReachableGuard */
  &make_exact_branch,       /* STKeepMatingGuard */
  &make_exact_branch,       /* STMaxFlightsquares */
  &make_exact_branch,       /* STDegenerateTree */
  &make_exact_branch,       /* STMaxNrNonTrivial */
  &make_exact_branch        /* STMaxThreatLength */
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

/* Detect the starting side from the stipulation
 */
void stip_detect_starter(void)
{
  boolean const same_starter_as_root = true;
  slice_detect_starter(root_slice,same_starter_as_root);
}

static slice_operation const starter_imposers[] =
{
  &pipe_impose_inverted_starter,  /* STBranchDirect */
  &pipe_impose_inverted_starter,  /* STBranchDirectDefender */
  &pipe_impose_inverted_starter,  /* STBranchHelp */
  &pipe_impose_inverted_starter,  /* STBranchSeries */
  &branch_fork_impose_starter,    /* STBranchFork */
  &leaf_impose_starter,           /* STLeafDirect */
  &leaf_impose_starter,           /* STLeafHelp */
  &leaf_impose_starter,           /* STLeafForced */
  &reci_impose_starter,           /* STReciprocal */
  &quodlibet_impose_starter,      /* STQuodlibet */
  &pipe_impose_starter,           /* STNot */
  &pipe_impose_inverted_starter,  /* STMoveInverter */
  &pipe_impose_inverted_starter,  /* STDirectRoot */
  &pipe_impose_inverted_starter,  /* STDirectDefenderRoot */
  &pipe_impose_starter,           /* STDirectHashed */
  &pipe_impose_inverted_starter,  /* STHelpRoot */
  &pipe_impose_starter,           /* STHelpHashed */
  &pipe_impose_inverted_starter,  /* STSeriesRoot */
  &pipe_impose_starter,           /* STSeriesHashed */
  &pipe_impose_starter,           /* STSelfCheckGuard */
  &direct_defense_impose_starter, /* STDirectDefense */
  &reflex_guard_impose_starter,   /* STReflexGuard */
  &self_attack_impose_starter,    /* STSelfAttack */
  &self_defense_impose_starter,   /* STSelfDefense */
  &pipe_impose_starter,           /* STRestartGuard */
  &pipe_impose_starter,           /* STGoalReachableGuard */
  &pipe_impose_starter,           /* STKeepMatingGuard */
  &pipe_impose_starter,           /* STMaxFlightsquares */
  &pipe_impose_starter,           /* STDegenerateTree */
  &pipe_impose_starter,           /* STMaxNrNonTrivial */
  &pipe_impose_starter            /* STMaxThreatLength */
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

  result1 = traverse_slices(slices[fork].u.fork.op1,st);
  result2 = traverse_slices(slices[fork].u.fork.op2,st);

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
 * @return true iff branch_fork and its children have been successfully
 *         traversed
 */
static boolean traverse_branch_fork(slice_index branch, slice_traversal *st)
{
  boolean const result_pipe = traverse_pipe(branch,st);
  slice_index const towards_goal = slices[branch].u.pipe.u.branch.towards_goal;
  boolean const result_toward_goal = traverse_slices(towards_goal,st);
  return result_pipe && result_toward_goal;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff branch_fork and its children have been successfully
 *         traversed
 */
static boolean traverse_guard(slice_index branch, slice_traversal *st)
{
  boolean const result_pipe = traverse_pipe(branch,st);
  slice_index const togoal = slices[branch].u.pipe.u.branch.towards_goal;
  boolean const result_togoal = traverse_slices(togoal,st);
  return result_pipe && result_togoal;
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
  slice_index const avoided = slices[branch].u.pipe.u.reflex_guard.avoided;
  boolean const result_not_slice = traverse_slices(avoided,st);
  return result_pipe && result_not_slice;
}

static slice_operation const traversers[] =
{
  &traverse_pipe,         /* STBranchDirect */
  &traverse_pipe,         /* STBranchDirectDefender */
  &traverse_pipe,         /* STBranchHelp */
  &traverse_pipe,         /* STBranchSeries */
  &traverse_branch_fork,  /* STBranchFork */
  &slice_operation_noop,  /* STLeafDirect */
  &slice_operation_noop,  /* STLeafHelp */
  &slice_operation_noop,  /* STLeafForced */
  &traverse_fork,         /* STReciprocal */
  &traverse_fork,         /* STQuodlibet */
  &traverse_pipe,         /* STNot */
  &traverse_pipe,         /* STMoveInverter */
  &traverse_pipe,         /* STDirectRoot */
  &traverse_pipe,         /* STDirectDefenderRoot */
  &traverse_pipe,         /* STDirectHashed */
  &traverse_pipe,         /* STHelpRoot */
  &traverse_pipe,         /* STHelpHashed */
  &traverse_pipe,         /* STSeriesRoot */
  &traverse_pipe,         /* STSeriesHashed */
  &traverse_pipe,         /* STSelfCheckGuard */
  &traverse_guard,        /* STDirectDefense */
  &traverse_reflex_guard, /* STReflexGuard */
  &traverse_guard,        /* STSelfAttack */
  &traverse_guard,        /* STSelfDefense */
  &traverse_pipe,         /* STRestartGuard */
  &traverse_pipe,         /* STGoalReachableGuard */
  &traverse_pipe,         /* STKeepMatingGuard */
  &traverse_pipe,         /* STMaxFlightsquares */
  &traverse_pipe,         /* STDegenerateTree */
  &traverse_pipe,         /* STMaxNrNonTrivial */
  &traverse_pipe          /* STMaxThreatLength */
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
