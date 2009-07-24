#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
#include "pybrad.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybraser.h"
#include "pyleaf.h"
#include "pyleafs.h"
#include "pyrecipr.h"
#include "pyquodli.h"
#include "pybrafrk.h"
#include "pynot.h"
#include "pyhelpha.h"
#include "pymovein.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

Slice slices[max_nr_slices];

slice_index root_slice;

static slice_index next_slice;

/* Allocate a slice index
 * @return a so far unused slice index
 */
slice_index alloc_slice_index(void)
{
  assert(next_slice<max_nr_slices);
  return next_slice++;
}

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice_index(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* TODO reuse all deallocated slice indices, not just the last
   * allocated one */

  if (slices[si].type==STBranchDirect)
    dealloc_slice_index(slices[si].u.pipe.next);

  if (next_slice==si+1)
    --next_slice;

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
         || type==STLeafSelf
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
         || type==STLeafSelf
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
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",original);
  TraceFunctionParamListEnd();

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
  next_slice = 0;
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

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchHelp:
      result = slices[si].u.pipe.u.branch.min_length;
      min_length *= 2;
      if (result%2==1)
        --min_length;
      if (min_length<=slices[si].u.pipe.u.branch.length)
        slices[si].u.pipe.u.branch.min_length = min_length;
      break;

    case STBranchSeries:
      result = slices[si].u.pipe.u.branch.min_length;
      if (min_length+1<=slices[si].u.pipe.u.branch.length)
        slices[si].u.pipe.u.branch.min_length = min_length+1;
      break;

    case STBranchDirect:
      result = slices[si].u.pipe.u.branch.min_length;
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

  result = traverse_slices(slices[si].u.pipe.u.branch_d_defender.towards_goal,
                           st);
  *max_nr += slices[si].u.pipe.u.branch_d_defender.length+1;

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

  result = traverse_slices(slices[si].u.pipe.u.branch_d_defender.towards_goal,
                           st);
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

  result = traverse_slices(slices[si].u.pipe.u.branch_fork.towards_goal,st);

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
static boolean get_max_nr_moves_leaf_self(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  stip_length_type * const max_nr = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *max_nr = 2;

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
  &get_max_nr_moves_leaf_self,       /* STLeafSelf */
  &get_max_nr_moves_leaf,            /* STLeafForced */
  &get_max_nr_moves_other,           /* STReciprocal */
  &get_max_nr_moves_other,           /* STQuodlibet */
  &get_max_nr_moves_other,           /* STNot */
  &get_max_nr_moves_other,           /* STMoveInverter */
  &get_max_nr_moves_other,           /* STHelpRoot */
  &get_max_nr_moves_other,           /* STHelpAdapter */
  &get_max_nr_moves_other            /* STHelpHashed */
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
  &slice_traverse_children,          /* STBranchDirect */
  &slice_traverse_children,          /* STBranchDirectDefender */
  &slice_traverse_children,          /* STBranchHelp */
  &slice_traverse_children,          /* STBranchSeries */
  &slice_traverse_children,          /* STBranchFork */
  &find_unique_goal_leaf,            /* STLeafDirect */
  &find_unique_goal_leaf,            /* STLeafHelp */
  &find_unique_goal_leaf,            /* STLeafSelf */
  &find_unique_goal_leaf,            /* STLeafForced */
  &slice_traverse_children,          /* STReciprocal */
  &slice_traverse_children,          /* STQuodlibet */
  &slice_traverse_children,          /* STNot */
  &slice_traverse_children,          /* STMoveInverter */
  &slice_traverse_children,          /* STHelpRoot */
  &slice_traverse_children,          /* STHelpAdapter */
  &slice_traverse_children           /* STHelpHashed */
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

static boolean transform_to_quodlibet_leaf_self(slice_index si,
                                                slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* Copy si to a new slot.
   * Then construct a new quodlibet slice over si; its operators
   * are a newly constructed direct leaf slice and the new slot.
   */
  {
    Goal const goal = slices[si].u.leaf.goal;
    slice_index const direct_leaf = alloc_leaf_slice(STLeafDirect,goal);
    slice_index const new_slot = copy_slice(si);
    make_quodlibet_slice(si,direct_leaf,new_slot);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean transform_to_quodlibet_help_adapter(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* Copy si to a new slot.
   * Then construct a new quodlibet slice over si; its operators
   * are a newly constructed direct leaf slice and the new slot.
   */
  {
    slice_index const unique_goal_slice = find_unique_goal(si);
    Goal const goal = slices[unique_goal_slice].u.leaf.goal;
    slice_index const direct_leaf = alloc_leaf_slice(STLeafDirect,goal);
    slice_index const new_slot = copy_slice(si);
    assert(unique_goal_slice!=no_slice);
    assert(slices[unique_goal_slice].type==STLeafHelp);
    make_quodlibet_slice(si,direct_leaf,new_slot);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const to_quodlibet_transformers[] =
{
  &slice_traverse_children,             /* STBranchDirect */
  &slice_traverse_children,             /* STBranchDirectDefender */
  0,                                    /* STBranchHelp */
  &slice_traverse_children,             /* STBranchSeries */
  0,                                    /* STBranchFork */
  0,                                    /* STLeafDirect */
  0,                                    /* STLeafHelp */
  &transform_to_quodlibet_leaf_self,    /* STLeafSelf */
  0,                                    /* STLeafForced */
  &slice_traverse_children,             /* STReciprocal */
  &slice_traverse_children,             /* STQuodlibet */
  0,                                    /* STNot */
  0,                                    /* STMoveInverter */
  0,                                    /* STHelpRoot */
  &transform_to_quodlibet_help_adapter, /* STHelpAdapter */
  0                                     /* STHelpHashed */
};

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal. 
 */
void transform_to_quodlibet(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&to_quodlibet_transformers,0);
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
  &leaf_ends_only_in,       /* STLeafSelf */
  &leaf_ends_only_in,       /* STLeafForced */
  &slice_traverse_children, /* STReciprocal */
  &slice_traverse_children, /* STQuodlibet */
  &slice_traverse_children, /* STNot */
  &slice_traverse_children, /* STMoveInverter */
  &slice_traverse_children, /* STHelpRoot */
  &slice_traverse_children, /* STHelpAdapter */
  &slice_traverse_children  /* STHelpHashed */
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
  &slice_ends_in_one_of_leaf, /* STLeafSelf */
  &slice_ends_in_one_of_leaf, /* STLeafForced */
  &slice_traverse_children,   /* STReciprocal */
  &slice_traverse_children,   /* STQuodlibet */
  &slice_traverse_children,   /* STNot */
  &slice_traverse_children,   /* STMoveInverter */
  &slice_traverse_children,   /* STHelpRoot */
  &slice_traverse_children,   /* STHelpAdapter */
  &slice_traverse_children    /* STHelpHashed */
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

/* Make a branch exact
 * @param branch identifies the branch
 * @param st address of structure defining traversal
 */
static boolean make_exact_branch_direct_defender(slice_index branch,
                                                 slice_traversal *st)
{
  slices[branch].u.pipe.u.branch_d_defender.min_length
      = slices[branch].u.pipe.u.branch_d_defender.length;

  return slice_traverse_children(branch,st);
}

static slice_operation const exact_makers[] =
{
  &make_exact_branch,                 /* STBranchDirect */
  &make_exact_branch_direct_defender, /* STBranchDirectDefender */
  &make_exact_branch,                 /* STBranchHelp */
  &make_exact_branch,                 /* STBranchSeries */
  &slice_traverse_children,           /* STBranchFork */
  &slice_traverse_children,           /* STLeafDirect */
  &slice_traverse_children,           /* STLeafHelp */
  &slice_traverse_children,           /* STLeafSelf */
  &slice_traverse_children,           /* STLeafForced */
  &slice_traverse_children,           /* STReciprocal */
  &slice_traverse_children,           /* STQuodlibet */
  &slice_traverse_children,           /* STNot */
  &slice_traverse_children,           /* STMoveInverter */
  &make_exact_branch,                 /* STHelpRoot */
  &make_exact_branch,                 /* STHelpAdapter */
  0                                   /* STHelpHashed */
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

static slice_operation const starter_imposers[] =
{
  &branch_d_impose_starter,          /* STBranchDirect */
  &branch_d_defender_impose_starter, /* STBranchDirectDefender */
  &branch_h_impose_starter,          /* STBranchHelp */
  &branch_ser_impose_starter,        /* STBranchSeries */
  &branch_fork_impose_starter,       /* STBranchFork */
  &leaf_impose_starter,              /* STLeafDirect */
  &leaf_impose_starter,              /* STLeafHelp */
  &leaf_s_impose_starter,            /* STLeafSelf */
  &leaf_impose_starter,              /* STLeafForced */
  &reci_impose_starter,              /* STReciprocal */
  &quodlibet_impose_starter,         /* STQuodlibet */
  &not_impose_starter,               /* STNot */
  &move_inverter_impose_starter,     /* STMoveInverter */
  &branch_h_impose_starter,          /* STHelpRoot */
  &branch_h_impose_starter,          /* STHelpAdapter */
  0                                  /* STHelpHashed */
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

  TraceValue("%u\n",slices[si].type);
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
  return traverse_slices(slices[pipe].u.pipe.next,st);
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff branch and its children have been successfully
 *         traversed
 */
static boolean traverse_branch_direct_defender(slice_index branch,
                                               slice_traversal *st)
{
  boolean const result_pipe = traverse_pipe(branch,st);
  slice_index const tg = slices[branch].u.pipe.u.branch_d_defender.towards_goal;
  boolean const result_toward_goal = traverse_slices(tg,st);
  return result_pipe && result_toward_goal;
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
  slice_index const tg = slices[branch].u.pipe.u.branch_fork.towards_goal;
  boolean const result_toward_goal = traverse_slices(tg,st);
  return result_pipe && result_toward_goal;
}

static slice_operation const traversers[] =
{
  &traverse_pipe,                   /* STBranchDirect */
  &traverse_branch_direct_defender, /* STBranchDirectDefender */
  &traverse_pipe,                   /* STBranchHelp */
  &traverse_pipe,                   /* STBranchSeries */
  &traverse_branch_fork,            /* STBranchFork */
  &slice_operation_noop,            /* STLeafDirect */
  &slice_operation_noop,            /* STLeafHelp */
  &slice_operation_noop,            /* STLeafSelf */
  &slice_operation_noop,            /* STLeafForced */
  &traverse_fork,                   /* STReciprocal */
  &traverse_fork,                   /* STQuodlibet */
  &traverse_pipe,                   /* STNot */
  &traverse_pipe,                   /* STMoveInverter */
  &traverse_pipe,                   /* STHelpRoot */
  &traverse_pipe,                   /* STHelpAdapter */
  &traverse_pipe                    /* STHelpHashed */
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
