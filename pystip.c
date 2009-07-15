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

  assert(slices[si].type!=STLeafDirect
         && slices[si].type!=STLeafSelf
         && slices[si].type!=STLeafHelp);

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

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si)
{
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = (slices[si].u.pipe.u.branch_d_defender.length+1
                +get_max_nr_moves(slices[si].u.pipe.u.branch_d_defender.towards_goal));
      break;

    case STBranchHelp:
    case STBranchSeries:
      result = (slices[si].u.pipe.u.branch.length
                +get_max_nr_moves(slices[si].u.pipe.next));
      break;

    case STBranchFork:
      result = get_max_nr_moves(slices[si].u.pipe.u.branch_fork.towards_goal);
      break;

    case STLeafSelf:
      result = 2;
      break;
      
    case STLeafDirect:
    case STLeafHelp:
    case STLeafForced:
      result = 1;
      break;

    case STReciprocal:
    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.fork.op1;
      stip_length_type const result1 = get_max_nr_moves(op1);

      slice_index const op2 = slices[si].u.fork.op2;
      stip_length_type const result2 = get_max_nr_moves(op2);

      result = result1>result2 ? result1 : result2;
      break;
    }

    case STBranchDirect:
    case STNot:
    case STMoveInverter:
    case STHelpHashed:
      result = get_max_nr_moves(slices[si].u.pipe.next);
      break;

    case STHelpRoot:
      result = get_max_nr_moves(slices[si].u.pipe.u.root_branch.full_length);
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

static void transform_to_quodlibet_recursive(slice_index *hook)
{
  slice_index const index = *hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*hook);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[index].type);
  switch (slices[index].type)
  {
    case STLeafSelf:
    {
      /* Insert a new quodlibet node at *hook's current position.
       * Move *hook to positon op2 of the new quodlibet node, and
       * add a new direct leaf at op1 of that node.
       * op1 is tested before op2, so it is more efficient to make
       * op1 the new direct leaf.
       */
      Goal const goal = slices[index].u.leaf.goal;
      *hook = alloc_quodlibet_slice(alloc_leaf_slice(STLeafDirect,goal),
                                    index);
      TraceValue("allocated quodlibet slice %u for self play\n",*hook);
      break;
    }

    case STQuodlibet:
    case STReciprocal:
      transform_to_quodlibet_recursive(&slices[index].u.fork.op1);
      transform_to_quodlibet_recursive(&slices[index].u.fork.op2);
      break;

    case STBranchHelp:
    {
      /* Insert a new quodlibet node at *hook's current position.
       * Move *hook to positon op2 of the new quodlibet node, and
       * add a new direct leaf at op1 of that node.
       * op1 is tested before op2, so it is more efficient to make
       * op1 the new direct leaf.
       */
      slice_index const fork = branch_find_fork(index);
      slice_index const to_goal
          = slices[fork].u.pipe.u.branch_fork.towards_goal;
      Goal const goal = slices[to_goal].u.leaf.goal;
      assert(slices[to_goal].type==STLeafHelp);
      *hook = alloc_quodlibet_slice(alloc_leaf_slice(STLeafDirect,goal),
                                    index);
      TraceValue("allocated quodlibet slice %u for reflex play\n",*hook);
      break;
    }

    case STBranchDirect:
    {
      slice_index peer = slices[index].u.pipe.next;
      slice_index towards_goal =
          slices[peer].u.pipe.u.branch_d_defender.towards_goal;
      assert(slices[peer].type==STBranchDirectDefender);
      transform_to_quodlibet_recursive(&towards_goal);
      slices[peer].u.pipe.u.branch_d_defender.towards_goal = towards_goal;
      break;
    }

    case STBranchSeries:
      transform_to_quodlibet_recursive(&slices[index].u.pipe.next);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal. 
 */
void transform_to_quodlibet(void)
{
  slice_index start = root_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  transform_to_quodlibet_recursive(&start);
  assert(start==root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Does a leaf have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si leaf slice identifier
 * @return true iff the leaf has as goal one of the elements of goals.
 */
static boolean leaf_ends_in_one_of(Goal const  goals[],
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

/* Do all leaves of a slice and its descendants have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si slice identifier
 * @return true iff all leaves have as goal one of the elements of goals.
 */
static boolean slice_ends_only_in(Goal const goals[],
                                  size_t nrGoals,
                                  slice_index si)
{
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafHelp:
    case STLeafSelf:
    case STLeafForced:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.fork.op1;
      slice_index const op2 = slices[si].u.fork.op2;
      return (slice_ends_only_in(goals,nrGoals,op1)
              && slice_ends_only_in(goals,nrGoals,op2));
    }

    case STNot:
      return !slice_ends_only_in(goals,nrGoals,slices[si].u.pipe.next);

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    case STMoveInverter:
    case STHelpHashed:
    {
      slice_index const peer = slices[si].u.pipe.next;
      return slice_ends_only_in(goals,nrGoals,peer);
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
      return slice_ends_only_in(goals,nrGoals,next);
    }

    case STBranchFork:
    {
      slice_index const next = slices[si].u.pipe.u.branch_fork.towards_goal;
      return slice_ends_only_in(goals,nrGoals,next);
    }

    case STHelpRoot:
    {
      slice_index const full_length = slices[si].u.pipe.u.root_branch.full_length;
      return slice_ends_only_in(goals,nrGoals,full_length);
    }

    default:
      assert(0);
      exit(1);
  }
}

/* Do all leaves of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff all leaves have as goal one of the elements of goals.
 */
boolean stip_ends_only_in(Goal const  goals[], size_t nrGoals)
{
  return slice_ends_only_in(goals,nrGoals,root_slice);
}


/* Does >= leaf of a slice and its descendants have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si slice identifier
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
static boolean slice_ends_in(Goal const goals[],
                             size_t nrGoals,
                             slice_index si)
{
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafHelp:
    case STLeafSelf:
    case STLeafForced:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.fork.op1;
      slice_index const op2 = slices[si].u.fork.op2;
      return (slice_ends_in(goals,nrGoals,op1)
              || slice_ends_in(goals,nrGoals,op2));
    }

    case STNot:
    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    case STMoveInverter:
    case STHelpHashed:
    {
      slice_index const peer = slices[si].u.pipe.next;
      return slice_ends_in(goals,nrGoals,peer);
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
      return slice_ends_in(goals,nrGoals,next);
    }

    case STBranchFork:
    {
      slice_index const next = slices[si].u.pipe.u.branch_fork.towards_goal;
      return slice_ends_in(goals,nrGoals,next);
    }

    case STHelpRoot:
    {
      slice_index const full_length = slices[si].u.pipe.u.root_branch.full_length;
      return slice_ends_in(goals,nrGoals,full_length);
    }

    default:
      assert(0);
      exit(1);
  }
}

/* Does >= 1 leaf of the current stipulation have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @return true iff >=1 leaf has as goal one of the elements of goals.
 */
boolean stip_ends_in(Goal const goals[], size_t nrGoals)
{
  return slice_ends_in(goals,nrGoals,root_slice);
}

/* Continue search for a goal. Cf. find_next_goal() */
static slice_index find_goal_recursive(Goal goal,
                                       slice_index start,
                                       boolean *active,
                                       slice_index si)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",*active);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafHelp:
    case STLeafSelf:
    case STLeafForced:
      if (*active)
      {
        if (slices[si].u.leaf.goal==goal)
          result = si;
      }
      else
        *active = si==start;
      break;

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.fork.op1;
      slice_index const op2 = slices[si].u.fork.op2;
      result = find_goal_recursive(goal,start,active,op1);
      if (result==no_slice)
        result = find_goal_recursive(goal,start,active,op2);
      break;
    }

    case STNot:
    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    case STMoveInverter:
    case STHelpHashed:
    {
      slice_index const peer = slices[si].u.pipe.next;
      result = find_goal_recursive(goal,start,active,peer);
      break;
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
      result = find_goal_recursive(goal,start,active,next);
      break;
    }

    case STBranchFork:
    {
      slice_index const next = slices[si].u.pipe.u.branch_fork.towards_goal;
      result = find_goal_recursive(goal,start,active,next);
      break;
    }

    case STHelpRoot:
    {
      slice_index const full_length = slices[si].u.pipe.u.root_branch.full_length;
      result = find_goal_recursive(goal,start,active,full_length);
      break;
    }

    default:
      assert(0);
      exit(1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the stipulation tree up to the next slice with a
 * specific goal. Repeated calls, with start set to the result of the
 * previous call, result in a complete traversal.
 * @param goal defines where to stop traversal
 * @param start traversal starts (continues) at the identified slice
 *              (excluding it, i.e. the result will be different from
 *              start); must be root_slice or the result of a previous
 *              call
 * @return if found, index of the next slice with the requested goal;
 *         no_slice otherwise
 */
slice_index find_next_goal(Goal goal, slice_index start)
{
  boolean active = start==root_slice;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  TraceValue("%u",next_slice);
  TraceValue("%u\n",slices[start].type);

  /* Either this is the first run (-> si==0) or we start from the
   * previous result, which must have been a leaf. */
  assert(start==root_slice
         || slices[start].type==STLeafDirect
         || slices[start].type==STLeafSelf
         || slices[start].type==STLeafHelp
         || slices[start].type==STLeafForced);

  result = find_goal_recursive(goal,start,&active,root_slice);

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

static boolean find_unique_goal_recursive(slice_index current_slice,
                                          slice_index *found_so_far)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_slice);
  TraceFunctionParam("%u",*found_so_far);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[current_slice].type);
  switch (slices[current_slice].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
    case STLeafForced:
      if (*found_so_far==no_slice)
      {
        *found_so_far = current_slice;
        result = true;
      }
      else
        result = are_goals_equal(*found_so_far,current_slice);
      break;
    
    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[current_slice].u.fork.op1;
      slice_index const op2 = slices[current_slice].u.fork.op2;
      result = (find_unique_goal_recursive(op1,found_so_far)
                && find_unique_goal_recursive(op2,found_so_far));
      break;
    }
    
    case STNot:
    case STBranchHelp:
    case STBranchSeries:
    case STMoveInverter:
    {
      slice_index const op = slices[current_slice].u.pipe.next;
      result = find_unique_goal_recursive(op,found_so_far);
      break;
    }

    case STBranchDirect:
    {
      slice_index const peer = slices[current_slice].u.pipe.next;
      /* prevent infinite recursion */
      if (peer<current_slice)
        result = find_unique_goal_recursive(peer,found_so_far);
      break;
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[current_slice].u.pipe.u.branch_d_defender.towards_goal;
      slice_index const peer = slices[current_slice].u.pipe.next;
      result = find_unique_goal_recursive(next,found_so_far);
      /* prevent infinite recursion */
      if (peer<current_slice)
        result = result && find_unique_goal_recursive(peer,found_so_far);
      break;
    }

    case STHelpRoot:
    {
      slice_index const full = slices[current_slice].u.pipe.u.root_branch.full_length;
      result = find_unique_goal_recursive(full,found_so_far);
      break;
    }

    case STBranchFork:
    {
      slice_index const
          next = slices[current_slice].u.pipe.u.branch_fork.towards_goal;
      result = find_unique_goal_recursive(next,found_so_far);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @return no_goal if goal is not unique; unique goal otherwise
 */
slice_index find_unique_goal(void)
{
  slice_index found_so_far = no_slice;
  return (find_unique_goal_recursive(root_slice,&found_so_far)
          ? found_so_far
          : no_slice);
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
  0,                                  /* STHelpRoot */
  &make_exact_branch                  /* STHelpHashed */
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
  &help_root_impose_starter,         /* STHelpRoot */
  &help_hashed_impose_starter        /* STHelpHashed */
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

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff pipe and its children have been successfully
 *         traversed
 */
static boolean traverse_help_root(slice_index root, slice_traversal *st)
{
  return traverse_slices(slices[root].u.pipe.u.root_branch.full_length,st);
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
  &traverse_help_root,              /* STHelpRoot */
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
