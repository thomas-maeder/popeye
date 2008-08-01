#include "pystip.h"
#include "pydata.h"
#include "trace.h"
#include "pyleaf.h"

#include <assert.h>
#include <stdlib.h>

Slice slices[max_nr_slices];

static slice_index next_slice;

/* Allocate a composite slice.
 * Initializes type to STSequence and composite fields to null values
 * @return index of allocated slice
 */
slice_index alloc_composite_slice(SliceType type, Play play)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",type);
  TraceFunctionParam("%d\n",play);

  slices[result].type = type; 
  slices[result].u.composite.play = play;
  slices[result].u.composite.length = 0;
  slices[result].u.composite.is_exact = false;
  slices[result].u.composite.op1 = no_slice;
  slices[result].u.composite.op2 = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(End end, square s)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result].type = STLeaf; 
  slices[result].u.leaf.end = end;
  slices[result].u.leaf.goal = goal_target;
  slices[result].u.leaf.target = s;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Allocate a (non-target) leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_leaf_slice(End end, Goal goal)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",end);
  TraceFunctionParam("%d\n",goal);

  slices[result].type = STLeaf; 
  slices[result].u.leaf.end = end;
  slices[result].u.leaf.goal = goal;
  slices[result].u.leaf.target = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original)
{
  slice_index const result = next_slice++;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result] = slices[original];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Release all slices
 */
void release_slices(void)
{
  next_slice = 0;
}

static void transform_to_quodlibet_recursive(slice_index *hook)
{
  slice_index const index = *hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",*hook);

  TraceValue("%d\n",slices[index].type);
  switch (slices[index].type)
  {
    case STLeaf:
      if (slices[index].u.leaf.end==ESelf
          || slices[index].u.leaf.end==EReflex
          || slices[index].u.leaf.end==ESemireflex)
      {
        Goal const goal = slices[index].u.leaf.goal;
        *hook = alloc_composite_slice(STQuodlibet,PDirect);
        TraceValue("allocated quodlibet slice %d\n",*hook);
        /* 1 is tested before 2, so make 1 EDirect and attach *hook at
         * 2 */
        slices[*hook].u.composite.op1 = alloc_leaf_slice(EDirect,goal);
        slices[*hook].u.composite.op2 = index;

        slices[*hook].u.composite.is_exact = false;
        slices[*hook].u.composite.length = (slices[0].u.composite.play==PHelp
                                            ? 2
                                            : 1); /* TODO */
      }
      break;

    case STQuodlibet:
    case STReciprocal:
      transform_to_quodlibet_recursive(&slices[index].u.composite.op1);
      transform_to_quodlibet_recursive(&slices[index].u.composite.op2);
      break;

    case STSequence:
      transform_to_quodlibet_recursive(&slices[index].u.composite.op1);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

void transform_to_quodlibet(void)
{
  slice_index start = 0;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  transform_to_quodlibet_recursive(&start);
  assert(start==0);

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
    case STLeaf:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;
      return (slice_ends_only_in(goals,nrGoals,op1)
              && slice_ends_only_in(goals,nrGoals,op2));
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      return slice_ends_only_in(goals,nrGoals,op1);
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
  return slice_ends_only_in(goals,nrGoals,0);
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
    case STLeaf:
      return leaf_ends_in_one_of(goals,nrGoals,si);

    case STQuodlibet:
    case STReciprocal:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;
      return (slice_ends_in(goals,nrGoals,op1)
              || slice_ends_in(goals,nrGoals,op2));
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      return slice_ends_in(goals,nrGoals,op1);
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
  return slice_ends_in(goals,nrGoals,0);
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
    case STLeaf:
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
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;
      result = find_goal_recursive(goal,start,active,op1);
      if (result==no_slice)
        result = find_goal_recursive(goal,start,active,op2);
      break;
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      result = find_goal_recursive(goal,start,active,op1);
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
 *              start); must be 0 or the result of a previous call
 * @return if found, index of the next slice with the requested goal;
 *         no_slice otherwise
 */
slice_index find_next_goal(Goal goal, slice_index start)
{
  boolean active = start==0;

  assert(start<next_slice);

  /* Either this is the first run (-> si==0) or we start from the
   * previous result, which must have been a leaf. */
  assert(start==0 || slices[start].type==STLeaf);

  return find_goal_recursive(goal,start,&active,0);
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
    case STLeaf:
      if (*found_so_far==no_slice)
      {
        *found_so_far = current_slice;
        return true;
      }
      else
        return are_goals_equal(*found_so_far,current_slice);

    case STReciprocal:
    case STQuodlibet:
    {
      slice_index const op1 = slices[current_slice].u.composite.op1;
      slice_index const op2 = slices[current_slice].u.composite.op2;
      return (find_unique_goal_recursive(op1,found_so_far)
              && find_unique_goal_recursive(op2,found_so_far));
    }
    
    case STSequence:
    {
      slice_index const op1 = slices[current_slice].u.composite.op1;
      return find_unique_goal_recursive(op1,found_so_far);
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
  return (find_unique_goal_recursive(0,&found_so_far)
          ? found_so_far
          : no_slice);
}

/* Determine whether a slice has >=1 solution
 * @param si slice index
 * @return true iff slice has >=1 solution(s)
 */
boolean slice_is_solvable(slice_index si)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_is_solvable(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Solve a slice (i.e. find and write all solutions)
 * @param side_at_move side doing the first move
 * @param si slice index
 * @return true iff slice has >=1 solution(s)
 */
boolean slice_solve(slice_index si)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}
