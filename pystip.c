#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

Slice slices[max_nr_slices];

slice_index root_slice;

Side regular_starter;

static slice_index next_slice;

/* Allocate a slice index
 * @return a so far unused slice index
 */
slice_index alloc_slice_index(void)
{
  return next_slice++;
}

/* Allocate a branch slice.
 * @param type type of slice
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_slice(SliceType type,
                               stip_length_type length,
                               stip_length_type min_length,
                               slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",type);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u\n",next);

  assert(type==STBranchDirect
         || type==STBranchHelp
         || type==STBranchSeries);

  slices[result].type = type; 
  slices[result].u.branch.starter = no_side; 
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;
  slices[result].u.branch.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(SliceType type, square s)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceText("\n");

  assert(type==STLeafDirect
         || type==STLeafHelp
         || type==STLeafSelf);

  slices[result].type = type; 
  slices[result].u.leaf.starter = no_side; 
  slices[result].u.leaf.goal = goal_target;
  slices[result].u.leaf.target = s;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",goal);

  assert(type==STLeafDirect
         || type==STLeafHelp
         || type==STLeafSelf);

  slices[result].type = type; 
  slices[result].u.leaf.starter = no_side; 
  slices[result].u.leaf.goal = goal;
  slices[result].u.leaf.target = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceText("\n");

  slices[result] = slices[original];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Release all slices
 */
void release_slices(void)
{
  next_slice = 0;
}

/* Set the min_length field of a composite slice.
 * @param si index of composite slice
 * @param min_length value to be set
 * @return previous value of min_length field
 */
stip_length_type set_min_length(slice_index si, stip_length_type min_length)
{
  stip_length_type const result = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",min_length);

  assert(slices[si].type!=STLeafDirect
         && slices[si].type!=STLeafSelf
         && slices[si].type!=STLeafHelp);

  switch (slices[si].type)
  {
    case STBranchHelp:
      min_length *= 2;
      if (result%2==1)
        --min_length;
      if (min_length<=slices[si].u.branch.length)
        slices[si].u.branch.min_length = min_length;
      break;

    case STBranchSeries:
      if (min_length+1<=slices[si].u.branch.length)
        slices[si].u.branch.min_length = min_length+1;
      break;

    default:
      /* nothing */
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

static void transform_to_quodlibet_recursive(slice_index *hook)
{
  slice_index const index = *hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",*hook);

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
    }
    break;

    case STQuodlibet:
      transform_to_quodlibet_recursive(&slices[index].u.quodlibet.op1);
      transform_to_quodlibet_recursive(&slices[index].u.quodlibet.op2);
      break;

    case STReciprocal:
      transform_to_quodlibet_recursive(&slices[index].u.reciprocal.op1);
      transform_to_quodlibet_recursive(&slices[index].u.reciprocal.op2);
      break;

    case STBranchHelp:
    {
      /* Insert a new quodlibet node at *hook's current position.
       * Move *hook to positon op2 of the new quodlibet node, and
       * add a new direct leaf at op1 of that node.
       * op1 is tested before op2, so it is more efficient to make
       * op1 the new direct leaf.
         */
      slice_index const next = slices[index].u.branch.next;
      Goal const goal = slices[next].u.leaf.goal;
      assert(slices[next].type==STLeafHelp);
      *hook = alloc_quodlibet_slice(alloc_leaf_slice(STLeafDirect,goal),
                                    index);
      TraceValue("allocated quodlibet slice %u for reflex play\n",*hook);
    }
    break;

    case STBranchDirect:
    case STBranchSeries:
      transform_to_quodlibet_recursive(&slices[index].u.branch.next);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal. 
 */
void transform_to_quodlibet(void)
{
  slice_index start = root_slice;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  transform_to_quodlibet_recursive(&start);
  assert(start==root_slice);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Does a leaf have one of a set of goals?
 * @param goals set of goals
 * @param nrGoals number of elements of goals
 * @param si leaf slice identifier
 * @return true iff the leaf has as goal one of the elements of goals.
 */
static boolean leaf_ends_in_one_of(Goal const  goals[],
                                   unsigned int nrGoals,
                                   slice_index si)
{
  Goal const goal = slices[si].u.leaf.goal;

  unsigned int i;
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
                                  unsigned int nrGoals,
                                  slice_index si)
{
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.quodlibet.op1;
      slice_index const op2 = slices[si].u.quodlibet.op2;
      return (slice_ends_only_in(goals,nrGoals,op1)
              && slice_ends_only_in(goals,nrGoals,op2));
    }

    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.reciprocal.op1;
      slice_index const op2 = slices[si].u.reciprocal.op2;
      return (slice_ends_only_in(goals,nrGoals,op1)
              && slice_ends_only_in(goals,nrGoals,op2));
    }

    case STNot:
      return !slice_ends_only_in(goals,nrGoals,slices[si].u.not.op);

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    {
      slice_index const next = slices[si].u.branch.next;
      return slice_ends_only_in(goals,nrGoals,next);
    }

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      return slice_ends_only_in(goals,nrGoals,next);
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
boolean stip_ends_only_in(Goal const  goals[], unsigned int nrGoals)
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
                             unsigned int nrGoals,
                             slice_index si)
{
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.quodlibet.op1;
      slice_index const op2 = slices[si].u.quodlibet.op2;
      return (slice_ends_in(goals,nrGoals,op1)
              || slice_ends_in(goals,nrGoals,op2));
    }

    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.reciprocal.op1;
      slice_index const op2 = slices[si].u.reciprocal.op2;
      return (slice_ends_in(goals,nrGoals,op1)
              || slice_ends_in(goals,nrGoals,op2));
    }

    case STNot:
    {
      slice_index const op = slices[si].u.not.op;
      return slice_ends_in(goals,nrGoals,op);
    }

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    {
      slice_index const next = slices[si].u.branch.next;
      return slice_ends_in(goals,nrGoals,next);
    }

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      return slice_ends_in(goals,nrGoals,next);
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
boolean stip_ends_in(Goal const goals[], unsigned int nrGoals)
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

  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      if (*active)
      {
        if (slices[si].u.leaf.goal==goal)
          result = si;
      }
      else
        *active = si==start;
      break;

    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.quodlibet.op1;
      slice_index const op2 = slices[si].u.quodlibet.op2;
      result = find_goal_recursive(goal,start,active,op1);
      if (result==no_slice)
        result = find_goal_recursive(goal,start,active,op2);
      break;
    }

    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.reciprocal.op1;
      slice_index const op2 = slices[si].u.reciprocal.op2;
      result = find_goal_recursive(goal,start,active,op1);
      if (result==no_slice)
        result = find_goal_recursive(goal,start,active,op2);
      break;
    }

    case STNot:
    {
      slice_index const op = slices[si].u.not.op;
      result = find_goal_recursive(goal,start,active,op);
      break;
    }

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    {
      slice_index const next = slices[si].u.branch.next;
      result = find_goal_recursive(goal,start,active,next);
      break;
    }

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      result = find_goal_recursive(goal,start,active,next);
      break;
    }

    default:
      assert(0);
      exit(1);
  }

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

  assert(start<next_slice);

  /* Either this is the first run (-> si==0) or we start from the
   * previous result, which must have been a leaf. */
  assert(start==root_slice
         || slices[start].type==STLeafDirect
         || slices[start].type==STLeafSelf
         || slices[start].type==STLeafHelp);

  return find_goal_recursive(goal,start,&active,root_slice);
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
  switch (slices[current_slice].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      if (*found_so_far==no_slice)
      {
        *found_so_far = current_slice;
        return true;
      }
      else
        return are_goals_equal(*found_so_far,current_slice);
    
    case STQuodlibet:
    {
      slice_index const op1 = slices[current_slice].u.quodlibet.op1;
      slice_index const op2 = slices[current_slice].u.quodlibet.op2;
      return (find_unique_goal_recursive(op1,found_so_far)
              && find_unique_goal_recursive(op2,found_so_far));
    }

    case STReciprocal:
    {
      slice_index const op1 = slices[current_slice].u.reciprocal.op1;
      slice_index const op2 = slices[current_slice].u.reciprocal.op2;
      return (find_unique_goal_recursive(op1,found_so_far)
              && find_unique_goal_recursive(op2,found_so_far));
    }
    
    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    {
      slice_index const next = slices[current_slice].u.branch.next;
      return find_unique_goal_recursive(next,found_so_far);
    }

    default:
      assert(0);
      return false;
  }
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
