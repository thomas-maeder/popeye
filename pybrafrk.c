#include "pybrafrk.h"
#include "pyhelp.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBranchFork slice.
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_fork_slice(slice_index next, slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slices[result].type = STBranchFork;
  slices[result].u.branch_fork.next = next;
  slices[result].u.branch_fork.next_towards_goal = towards_goal;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at the move
 * @return true iff >=1 solution was found
 */
boolean branch_fork_help_solve_in_n(slice_index si,
                                    stip_length_type n,
                                    Side side_at_move)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_solve(slices[si].u.branch_fork.next_towards_goal);
  else
  {
    slice_index const next = slices[si].u.branch_fork.next;
    result = help_solve_in_n(next,n,side_at_move);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean branch_fork_help_has_solution_in_n(slice_index si,
                                           stip_length_type n,
                                           Side side_at_move)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_has_solution(slices[si].u.branch_fork.next_towards_goal);
  else
  {
    slice_index const next = slices[si].u.branch_fork.next;
    result = help_has_solution_in_n(next,n,side_at_move);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
void branch_fork_help_solve_continuations_in_n(table continuations,
                                               slice_index si,
                                               stip_length_type n,
                                               Side side_at_move)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    slice_solve_continuations(continuations,
                              slices[si].u.branch_fork.next_towards_goal);
  else
  {
    slice_index const next = slices[si].u.branch_fork.next;
    help_solve_continuations_in_n(continuations,next,n,side_at_move);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param side starting side of leaf
 */
void branch_fork_impose_starter(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",side);
  TraceFunctionParamListEnd();

  slice_impose_starter(slices[si].u.branch_fork.next_towards_goal,side);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the starter in a help stipulation in n half-moves
 * @param si identifies slice
 * @param n number of half-moves
 * @param return starting side
 */
Side branch_fork_help_starter_in_n(slice_index si, stip_length_type n)
{
  Side result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = help_starter_in_n(slices[si].u.branch_fork.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_fork_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_must_starter_resign(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void branch_fork_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_write_unsolvability(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_fork_has_non_starter_solved(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean branch_fork_has_starter_apriori_lost(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_apriori_lost(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_fork_has_starter_won(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_won(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_fork_has_starter_reached_goal(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_reached_goal(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_fork_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",just_moved);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,
                                 slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_fork_detect_starter(slice_index si,
                                                  boolean same_side_as_root)
{
  who_decides_on_starter result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  result = slice_detect_starter(slices[si].u.branch_fork.next_towards_goal,
                                same_side_as_root);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 */
void branch_fork_solve_postkey(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_solve_postkey(slices[si].u.branch_fork.next_towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @param just_moved side that has just moved
 * @return true iff no chance is left
 */
boolean branch_fork_must_starter_resign_hashed(slice_index si, Side just_moved)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",just_moved);
  TraceFunctionParamListEnd();

  result = slice_must_starter_resign_hashed(slices[si].u.branch_fork.next_towards_goal,
                                            just_moved);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Locate the slice after the help play branch and its associated slices
 * @param si identifies slice visited in traversal
 * @param st address of structure defining traversal
 */
static void slice_behind_branch_finder_branch_fork(slice_index si,
                                                   slice_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* The slice we look for is the one at the towards_goal end of a
   * help_hashed slice. Save it and don't recurse further.
   */
  *result = slices[si].u.branch_fork.next_towards_goal;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const slice_behind_branch_finders[] =
{
  &slice_traverse_children,               /* STBranchDirect */
  &slice_traverse_children,               /* STBranchDirectDefender */
  &slice_traverse_children,               /* STBranchHelp */
  &slice_traverse_children,               /* STBranchSeries */
  &slice_behind_branch_finder_branch_fork,/* STBranchFork */
  &slice_traverse_children,               /* STLeafDirect */
  &slice_traverse_children,               /* STLeafHelp */
  &slice_traverse_children,               /* STLeafSelf */
  &slice_traverse_children,               /* STLeafForced */
  &slice_traverse_children,               /* STReciprocal */
  &slice_traverse_children,               /* STQuodlibet */
  &slice_traverse_children,               /* STNot */
  &slice_traverse_children,               /* STMoveInverter */
  &slice_traverse_children                /* STHelpHashed */
};

/* Find the slice representing the play after a branch
 * @param branch identifies the branch
 * @return identifier for branch representing the play after the branch
 */
slice_index branch_find_slice_behind_fork(slice_index branch)
{
  slice_index result;
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&slice_behind_branch_finders,&result);
  traverse_slices(branch,&st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue deallocating a branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 */
static void traverse_and_deallocate(slice_index si, slice_traversal *st)
{
  slice_traverse_children(si,st);
  dealloc_slice_index(si);
}

/* Store slice representing play after branch in object representing
 * traversal, then continue deallocating the branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 */
static void traverse_and_deallocate_branch_fork(slice_index si,
                                                slice_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *result = slices[si].u.branch_fork.next_towards_goal;

  traverse_slices(slices[si].u.branch_fork.next,st);
  dealloc_slice_index(si);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const slice_to_fork_deallocators[] =
{
  &traverse_and_deallocate,             /* STBranchDirect */
  &traverse_and_deallocate,             /* STBranchDirectDefender */
  &traverse_and_deallocate,             /* STBranchHelp */
  &traverse_and_deallocate,             /* STBranchSeries */
  &traverse_and_deallocate_branch_fork, /* STBranchFork */
  0,                                    /* STLeafDirect */
  0,                                    /* STLeafHelp */
  0,                                    /* STLeafSelf */
  0,                                    /* STLeafForced */
  0,                                    /* STReciprocal */
  0,                                    /* STQuodlibet */
  0,                                    /* STNot */
  0,                                    /* STMoveInverter */
  &traverse_and_deallocate              /* STHelpHashed */
};

/* Deallocate a branch
 * @param branch identifies branch
 * @return index of slice representing the play after the branch
 */
slice_index branch_deallocate_to_fork(slice_index branch)
{
  slice_index result;
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&slice_to_fork_deallocators,&result);
  traverse_slices(branch,&st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
