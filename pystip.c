#include "pystip.h"
#include "pydata.h"
#include "pyquodli.h"
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
  /* TODO reuse all deallocated slice indices, not just the last
   * allocated one */

  if (slices[si].type==STBranchDirect)
    dealloc_slice_index(slices[si].u.branch_d.peer);

  if (next_slice==si+1)
    --next_slice;
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
         || type==STLeafSelf
         || type==STLeafForced);

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
         || type==STLeafSelf
         || type==STLeafForced);

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
  TraceFunctionParam("%u\n",original);

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
  TraceFunctionParam("%u\n",min_length);

  assert(slices[si].type!=STLeafDirect
         && slices[si].type!=STLeafSelf
         && slices[si].type!=STLeafHelp);

  switch (slices[si].type)
  {
    case STBranchHelp:
      result = slices[si].u.branch.min_length;
      min_length *= 2;
      if (result%2==1)
        --min_length;
      if (min_length<=slices[si].u.branch.length)
        slices[si].u.branch.min_length = min_length;
      break;

    case STBranchSeries:
      result = slices[si].u.branch.min_length;
      if (min_length+1<=slices[si].u.branch.length)
        slices[si].u.branch.min_length = min_length+1;
      break;

    case STBranchDirect:
      result = slices[si].u.branch_d.min_length;
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STBranchDirect:
      result = get_max_nr_moves(slices[si].u.branch_d.peer);
      break;

    case STBranchDirectDefender:
      result = (slices[si].u.branch_d_defender.length+1
                +get_max_nr_moves(slices[si].u.branch_d_defender.next));
      break;

    case STBranchHelp:
    case STBranchSeries:
      result = (slices[si].u.branch.length
                +get_max_nr_moves(slices[si].u.branch.next));
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
    {
      slice_index const op1 = slices[si].u.reciprocal.op1;
      stip_length_type const result1 = get_max_nr_moves(op1);

      slice_index const op2 = slices[si].u.reciprocal.op2;
      stip_length_type const result2 = get_max_nr_moves(op2);

      result = result1>result2 ? result1 : result2;
      break;
    }
    
    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.quodlibet.op1;
      stip_length_type const result1 = get_max_nr_moves(op1);

      slice_index const op2 = slices[si].u.quodlibet.op2;
      stip_length_type const result2 = get_max_nr_moves(op2);

      result = result1>result2 ? result1 : result2;
      break;
    }

    case STNot:
      result = get_max_nr_moves(slices[si].u.not.op);
      break;
  
    case STMoveInverter:
      result = get_max_nr_moves(slices[si].u.move_inverter.next);
      break;

    default:
      assert(0);
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
      break;
    }

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
      break;
    }

    case STBranchDirect:
    {
      slice_index peer = slices[index].u.branch_d.peer;
      slice_index next = slices[peer].u.branch_d_defender.next;
      assert(slices[peer].type==STBranchDirectDefender);
      transform_to_quodlibet_recursive(&next);
      slices[peer].u.branch_d_defender.next = next;
      break;
    }

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
    {
      slice_index const peer = slices[si].u.branch_d.peer;
      return slice_ends_only_in(goals,nrGoals,peer);
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[si].u.branch_d_defender.next;
      return slice_ends_only_in(goals,nrGoals,next);
    }

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
    {
      slice_index const peer = slices[si].u.branch_d.peer;
      return slice_ends_in(goals,nrGoals,peer);
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[si].u.branch_d_defender.next;
      return slice_ends_in(goals,nrGoals,next);
    }

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
  TraceFunctionParam("%u\n",si);

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
    {
      slice_index const peer = slices[si].u.branch_d.peer;
      result = find_goal_recursive(goal,start,active,peer);
      break;
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[si].u.branch_d_defender.next;
      result = find_goal_recursive(goal,start,active,next);
      break;
    }

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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",start);

  TraceValue("%u",next_slice);
  TraceValue("%u\n",slices[start].type);

  assert(start<next_slice);

  /* Either this is the first run (-> si==0) or we start from the
   * previous result, which must have been a leaf. */
  assert(start==root_slice
         || slices[start].type==STLeafDirect
         || slices[start].type==STLeafSelf
         || slices[start].type==STLeafHelp
         || slices[start].type==STLeafForced);

  result = find_goal_recursive(goal,start,&active,root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",*found_so_far);

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
    {
      slice_index const op1 = slices[current_slice].u.quodlibet.op1;
      slice_index const op2 = slices[current_slice].u.quodlibet.op2;
      result = (find_unique_goal_recursive(op1,found_so_far)
                && find_unique_goal_recursive(op2,found_so_far));
      break;
    }

    case STReciprocal:
    {
      slice_index const op1 = slices[current_slice].u.reciprocal.op1;
      slice_index const op2 = slices[current_slice].u.reciprocal.op2;
      result = (find_unique_goal_recursive(op1,found_so_far)
                && find_unique_goal_recursive(op2,found_so_far));
      break;
    }
    
    case STNot:
    {
      slice_index const op = slices[current_slice].u.not.op;
      result = find_unique_goal_recursive(op,found_so_far);
      break;
    }

    case STBranchDirect:
    {
      slice_index const peer = slices[current_slice].u.branch_d.peer;
      /* prevent infinite recursion */
      if (peer<current_slice)
        result = find_unique_goal_recursive(peer,found_so_far);
      break;
    }

    case STBranchDirectDefender:
    {
      slice_index const next = slices[current_slice].u.branch_d_defender.next;
      slice_index const peer = slices[current_slice].u.branch_d_defender.peer;
      result = find_unique_goal_recursive(next,found_so_far);
      /* prevent infinite recursion */
      if (peer<current_slice)
        result = result && find_unique_goal_recursive(peer,found_so_far);
      break;
    }

    case STBranchHelp:
    case STBranchSeries:
    {
      slice_index const next = slices[current_slice].u.branch.next;
      result = find_unique_goal_recursive(next,found_so_far);
      break;
    }

    case STMoveInverter:
    {
      slice_index const next = slices[current_slice].u.move_inverter.next;
      result = find_unique_goal_recursive(next,found_so_far);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
 * @param dummy ignored
 */
static void make_exact_branch_direct(slice_index branch, void *dummy)
{
  slices[branch].u.branch.min_length = slices[branch].u.branch_d.length;
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param dummy ignored
 */
static void make_exact_branch_direct_defender(slice_index branch, void *dummy)
{
  slices[branch].u.branch_d_defender.min_length
      = slices[branch].u.branch_d_defender.length;
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param dummy ignored
 */
static void make_exact_branch_help(slice_index branch, void *dummy)
{
  slices[branch].u.branch.min_length = slices[branch].u.branch.length;
}

/* Make a branch exact
 * @param branch identifies the branch
 * @param dummy ignored
 */
static void make_exact_branch_series(slice_index branch, void *dummy)
{
  slices[branch].u.branch.min_length = slices[branch].u.branch.length;
}

static slice_operation const exact_makers[] =
{
  &make_exact_branch_direct,          /* STBranchDirect */
  &make_exact_branch_direct_defender, /* STBranchDirectDefender */
  &make_exact_branch_help,            /* STBranchHelp */
  &make_exact_branch_series,          /* STBranchSeries */
  &slice_operation_noop,              /* STLeafDirect */
  &slice_operation_noop,              /* STLeafHelp */
  &slice_operation_noop,              /* STLeafSelf */
  &slice_operation_noop,              /* STLeafForced */
  &slice_operation_noop,              /* STReciprocal */
  &slice_operation_noop,              /* STQuodlibet */
  &slice_operation_noop,              /* STNot */
  &slice_operation_noop               /* STMoveInverter */
};

/* Make the stipulation exact
 */
void stip_make_exact(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  traverse_slices(&exact_makers,0);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table fro dispatch_to_slice()
 */
void slice_operation_noop(slice_index si, void *userdata)
{
}

/* Dispatch an operation to a slice based on the slice's type
 * @param si identifies slice
 * @param ops address of array mapping slice tpye to operation
 * @param param address of data structure holding parameters for the operation
 */
void dispatch_to_slice(slice_index si, operation_mapping ops, void *param)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p\n",param);

  TraceValue("%u\n",slices[si].type);
  assert(slices[si].type<=nr_slice_types);

  {
    slice_operation const operation = (*ops)[slices[si].type];
    assert(operation!=0);
    (*operation)(si,param);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

typedef struct
{
    boolean visited[max_nr_slices];
    slice_operation const (*ops)[nr_slice_types];
    void *op_param;
} traversal_params;

/* Declaration of traverse_recursive().
 * Used by the subsequent traverse_*() functions, which are indirectly
 * recursive.
 */
static void traverse_recursive(slice_index si, traversal_params *data);

/* Traverse a subtree
 * @param quodlibet root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_quodlibet(slice_index quodlibet, void *param)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",quodlibet);
  TraceFunctionParam("%p\n",param);

  TraceValue("%p\n",((traversal_params *)param)->op_param);

  traverse_recursive(slices[quodlibet].u.quodlibet.op1,param);
  traverse_recursive(slices[quodlibet].u.quodlibet.op2,param);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Traverse a subtree
 * @param reciprocal root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_reciprocal(slice_index reciprocal, void *param)
{
  traverse_recursive(slices[reciprocal].u.reciprocal.op1,param);
  traverse_recursive(slices[reciprocal].u.reciprocal.op2,param);
}

/* Traverse a subtree
 * @param not root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_not(slice_index not, void *param)
{
  traverse_recursive(slices[not].u.not.op,param);
}

/* Traverse a subtree
 * @param mi root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_move_inverter(slice_index mi, void *param)
{
  traverse_recursive(slices[mi].u.move_inverter.next,param);
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_branch_direct(slice_index branch, void *param)
{
  traverse_recursive(slices[branch].u.branch_d.peer,param);
}

/* Traverse a subtree
 * @param defender root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_branch_direct_defender(slice_index defender, void *param)
{
  traverse_recursive(slices[defender].u.branch_d_defender.peer,param);
  traverse_recursive(slices[defender].u.branch_d_defender.next,param);
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_branch_help(slice_index branch, void *param)
{
  traverse_recursive(slices[branch].u.branch.next,param);
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param param address of state of slice properties initialisation
 * @note this is an indirectly recursive function
 */
static void traverse_branch_series(slice_index branch, void *param)
{
  traverse_recursive(slices[branch].u.branch.next,param);
}

static slice_operation const traversers[] =
{
  &traverse_branch_direct,          /* STBranchDirect */
  &traverse_branch_direct_defender, /* STBranchDirectDefender */
  &traverse_branch_help,            /* STBranchHelp */
  &traverse_branch_series,          /* STBranchSeries */
  &slice_operation_noop,            /* STLeafDirect */
  &slice_operation_noop,            /* STLeafHelp */
  &slice_operation_noop,            /* STLeafSelf */
  &slice_operation_noop,            /* STLeafForced */
  &traverse_reciprocal,             /* STReciprocal */
  &traverse_quodlibet,              /* STQuodlibet */
  &traverse_not,                    /* STNot */
  &traverse_move_inverter           /* STMoveInverter */
};

/* (Approximately) depth-first traversl of the stipulation subtree
 * @param root root of subtree
 * @param ops mapping from slice types to operations
 * @param param address of data structure holding additional data
 *              for the operation; passed to the selected operation
 */
static void traverse_recursive(slice_index root, traversal_params *param)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p\n",param);

  if (!param->visited[root])
  {
    param->visited[root] = true;
    dispatch_to_slice(root,&traversers,param);
    dispatch_to_slice(root,param->ops,param->op_param);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* (Approximately) depth-first traversl of the stipulation
 * @param ops mapping from slice types to operations
 * @param param address of data structure holding parameters for the operation
 */
void traverse_slices(slice_operation const (*ops)[nr_slice_types], void *param)
{
  /* C89 doesn't allow initialising struct members with local
   * variables (C99 does)
   */
  traversal_params traversal_param = { {false}, 0, 0 };
  traversal_param.ops = ops;
  traversal_param.op_param = param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p\n",param);

  traverse_recursive(root_slice,&traversal_param);

  TraceFunctionExit(__func__);
  TraceText("\n");
}
