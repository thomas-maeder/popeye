#include "pystip.h"
#include "pydata.h"
#include "trace.h"
#include "pyleaf.h"
#include "pybrad.h"
#include "pybrah.h"
#include "pybraser.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pybranch.h"

#include <assert.h>
#include <stdlib.h>

Slice slices[max_nr_slices];

Side regular_starter;

static slice_index next_slice;

/* Allocate a slice index
 * @return a so far unused slice index
 */
slice_index alloc_slice_index(void)
{
  return next_slice++;
}

/* Allocate a composite slice.
 * @return index of allocated slice
 */
slice_index alloc_branch_slice(SliceType type)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",type);

  slices[result].type = type; 
  slices[result].u.branch.starter = no_side; 
  slices[result].u.branch.length = 0;
  slices[result].u.branch.next = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Allocate a target leaf slice.
 * Initializes type to STLeaf and leaf fields according to arguments
 * @return index of allocated slice
 */
slice_index alloc_target_leaf_slice(End end, square s)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result].type = STLeaf; 
  slices[result].u.leaf.starter = no_side; 
  slices[result].u.leaf.end = end;
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
slice_index alloc_leaf_slice(End end, Goal goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",end);
  TraceFunctionParam("%u\n",goal);

  slices[result].type = STLeaf; 
  slices[result].u.leaf.starter = no_side; 
  slices[result].u.leaf.end = end;
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

  assert(slices[si].type!=STLeaf);

  if (slices[si].type==STBranchHelp)
  {
    min_length *= 2;
    if (result%2==1)
      --min_length;
  }

  if (min_length<=slices[si].u.branch.length)
    slices[si].u.branch.min_length = min_length;

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
    case STLeaf:
      if (slices[index].u.leaf.end==ESelf
          || slices[index].u.leaf.end==EHelp)
      {
        /* Insert a new quodlibet node at *hook's current position.
         * Move *hook to positon op2 of the new quodlibet node, and
         * add a new direct leaf at op1 of that node.
         * op1 is tested before op2, so it is more efficient to make
         * op1 the new direct leaf.
         */
        Goal const goal = slices[index].u.leaf.goal;
        *hook = alloc_quodlibet_slice(alloc_leaf_slice(EDirect,goal),
                                      index);
        TraceValue("allocated quodlibet slice %u\n",*hook);
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

    case STBranchDirect:
    case STBranchHelp:
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

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
    {
      slice_index const next = slices[si].u.branch.next;
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
  return (find_unique_goal_recursive(0,&found_so_far)
          ? found_so_far
          : no_slice);
}

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean slice_is_unsolvable(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_is_unsolvable(si);
      break;

    case STReciprocal:
      result = reci_is_unsolvable(si);
      break;
      
    case STQuodlibet:
      result = quodlibet_is_unsolvable(si);
      break;
      
    case STNot:
      result = not_is_unsolvable(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_end_is_unsolvable(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a slice has >=1 solution
 * @param si slice index
 * @return true iff slice has >=1 solution(s)
 */
boolean slice_is_solvable(slice_index si)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_is_solvable(si);
      break;

    case STNot:
      result = not_is_solvable(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void slice_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      leaf_solve_continuations(table,si);
      break;
    
    case STQuodlibet:
      quodlibet_solve_continuations(table,si);
      break;

    case STReciprocal:
      reci_solve_continuations(table,si);
      break;

    case STNot:
      not_solve_continuations(table,si);
      break;

    case STBranchDirect:
      branch_d_solve_continuations_in_n(table,si,slices[si].u.branch.length);
      break;

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean slice_root_solve_setplay(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_root_solve_setplay(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_solve_setplay(si);
      break;

    case STBranchDirect:
      result = branch_d_root_solve_setplay(si);
      break;

    case STBranchHelp:
      result = branch_h_root_solve_setplay(si);
      break;

    case STBranchSeries:
      /* TODO implement branch_ser_root_solve_setplay() */
      break;

    case STReciprocal:
      result = reci_root_solve_setplay(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean slice_root_end_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_root_solve_complete_set(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_root_end_solve_complete_set(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_solve_complete_set(si);
      break;

    case STReciprocal:
      /* not really meaningful */
      break;
      
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key just played, then continue solving in the slice
 * to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void slice_root_write_key_solve_postkey(slice_index si, attack_type type)
{
  switch (slices[si].type)
  {
    case STLeaf:
      leaf_root_write_key_solve_postkey(si,type);
      break;

    case STQuodlibet:
      quodlibet_root_write_key_solve_postkey(si,type);
      break;

    case STBranchDirect:
    {
      int const refutations = alloctab();
      branch_d_root_write_key_solve_postkey(refutations,si,type);
      freetab();
      break;
    }

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    case STReciprocal:
    {
      reci_root_write_key_solve_postkey(si,type);
      break;
    }

    default:
      assert(0);
      break;
  }
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si)
{
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      solution_found = leaf_solve(si);
      break;

    case STQuodlibet:
      solution_found = quodlibet_solve(si);
      break;

    case STBranchDirect:
      /* TODO */
      break;

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      solution_found = branch_ser_solve(si);
      break;

    case STReciprocal:
      solution_found = reci_solve(si);
      break;

    case STNot:
      solution_found = not_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
void slice_root_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      /* TODO add leaf_root_solve() without return value?? */
      leaf_solve(si);
      break;

    case STQuodlibet:
      quodlibet_root_solve(si);
      break;

    case STReciprocal:
      reci_root_solve(si);
      break;

    case STNot:
      not_root_solve(si);
      break;

    case STBranchDirect:
      branch_d_root_solve(si);
      break;

    case STBranchHelp:
      branch_h_root_solve(si);
      break;

    case STBranchSeries:
      branch_ser_root_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void slice_root_solve_in_n(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  switch (slices[si].type)
  {
    case STBranchHelp:
      branch_h_root_solve_in_n(si,n);
      break;

    case STBranchSeries:
      branch_ser_root_solve_in_n(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether a composite slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean slice_has_solution(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_has_solution(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_solution(si);
      break;

    case STReciprocal:
      result = reci_has_solution(si);
      break;

    case STNot:
      result = not_has_solution(si);
      break;

    case STBranchDirect:
      result = branch_d_has_solution_in_n(si,slices[si].u.branch.length);
      break;

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write variations
 * @param si slice index
 */
void slice_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      leaf_solve_variations(si);
      break;

    case STQuodlibet:
      quodlibet_solve_variations(si);
      break;

    case STBranchDirect:
      branch_d_solve_variations(si);
      break;

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    case STReciprocal:
      reci_solve_variations(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean slice_has_non_starter_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_has_non_starter_solved(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_end_has_non_starter_solved(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_non_starter_solved(si);
      break;

    case STReciprocal:
      result = reci_has_non_starter_solved(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the non-starter has refuted with his move just
 * played independently of the starter's possible play during the
 * current slice.
 * Example: in direct play, the defender has just captured that last
 * piece that could deliver mate.
 * @param si slice identifier
 * @return true iff the non-starter has refuted
 */
boolean slice_end_has_non_starter_refuted(slice_index si)
{
  boolean result = false;

  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_is_unsolvable(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_non_starter_refuted(si);
      break;

    case STReciprocal:
      result = reci_has_non_starter_refuted(si);
      break;

    case STNot:
      result = not_has_non_starter_refuted(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_end_has_non_starter_refuted(si);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Determine whether the starting side has lost with its move just
 * played independently of his possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean slice_end_has_starter_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_has_starter_lost(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_end_has_starter_lost(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_starter_lost(si);
      break;

    case STReciprocal:
      result = reci_has_starter_lost(si);
      break;

    case STNot:
      result = not_has_starter_lost(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean slice_end_has_starter_won(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeaf:
      result = leaf_has_starter_won(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_end_has_starter_won(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_starter_won(si);
      break;

    case STReciprocal:
      result = reci_has_starter_won(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void slice_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      leaf_write_unsolvability(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      branch_write_unsolvability(si);
      break;

    case STQuodlibet:
      quodlibet_write_unsolvability(si);
      break;

    case STReciprocal:
      reci_write_unsolvability(si);
      break;

    case STNot:
      not_write_unsolvability(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean slice_is_threat_refuted(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = !leaf_has_starter_solved(si);
      break;

    case STQuodlibet:
      result = quodlibet_is_threat_refuted(si);
      break;

    case STReciprocal:
      result = reci_is_threat_refuted(si);
      break;

    case STNot:
      result = not_is_threat_refuted(si);
      break;

    case STBranchDirect:
      result = branch_d_is_threat_in_n_refuted(si,
                                               slices[si].u.branch.length);
      break;

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void slice_detect_starter(slice_index si, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      leaf_detect_starter(si,is_duplex);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      branch_detect_starter(si,is_duplex);
      break;

    case STReciprocal:
      reci_detect_starter(si,is_duplex);
      break;

    case STQuodlibet:
      quodlibet_detect_starter(si,is_duplex);
      break;

    case STNot:
      not_detect_starter(si,is_duplex);
      break;

    default:
      assert(0);
      break;
  }

  if (si==0)
  {
    regular_starter = slice_get_starter(0);
    TraceValue("%u\n",regular_starter);
  }

  if (slices[si].type==STBranchDirect
      || slices[si].type==STBranchHelp
      || slices[si].type==STBranchSeries)
  {
    TraceValue("%u\n",slices[si].u.branch.length);
    if (slices[si].type==STBranchHelp
        && slices[si].u.branch.length%2 == 1)
    {
      if (slice_get_starter(si)==no_side)
        slice_impose_starter(si,no_side);
      else
        slice_impose_starter(si,advers(slice_get_starter(si)));
    }
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void slice_impose_starter(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",side);

  switch (slices[si].type)
  {
    case STLeaf:
      leaf_impose_starter(si,side);
      break;

    case STBranchDirect:
      branch_d_impose_starter(si,side);
      break;

    case STBranchHelp:
      branch_h_impose_starter(si,side);
      break;

    case STBranchSeries:
      branch_ser_impose_starter(si,side);
      break;

    case STReciprocal:
      reci_impose_starter(si,side);
      break;

    case STQuodlibet:
      quodlibet_impose_starter(si,side);
      break;

    case STNot:
      not_impose_starter(si,side);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side slice_get_starter(slice_index si)
{
  Side result = no_side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = slices[si].u.leaf.starter;
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = slices[si].u.branch.starter;
      break;

    case STReciprocal:
      assert(slice_get_starter(slices[si].u.reciprocal.op1)
             ==slice_get_starter(slices[si].u.reciprocal.op2));
      result = slice_get_starter(slices[si].u.reciprocal.op1);
      break;

    case STQuodlibet:
      assert(slice_get_starter(slices[si].u.quodlibet.op1)
             ==slice_get_starter(slices[si].u.quodlibet.op2));
      result = slice_get_starter(slices[si].u.quodlibet.op1);
      break;

    case STNot:
      result = slice_get_starter(slices[si].u.not.op);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
