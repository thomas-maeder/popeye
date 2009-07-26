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
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch_fork.towards_goal = towards_goal;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean branch_fork_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_solve(slices[si].u.pipe.u.branch_fork.towards_goal);
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = help_solve_in_n(next,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_fork_help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_has_solution(slices[si].u.pipe.u.branch_fork.towards_goal);
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = help_has_solution_in_n(next,n);
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
 */
void branch_fork_help_solve_continuations_in_n(table continuations,
                                               slice_index si,
                                               stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    slice_solve_continuations(continuations,
                              slices[si].u.pipe.u.branch_fork.towards_goal);
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    help_solve_continuations_in_n(continuations,next,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean branch_fork_impose_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  result = traverse_slices(slices[si].u.pipe.u.branch_fork.towards_goal,st);

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

  result = slice_must_starter_resign(slices[si].u.pipe.u.branch_fork.towards_goal);

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
  slice_index const towards_goal = slices[si].u.pipe.u.branch_fork.towards_goal;
  who_decides_on_starter result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  result = slice_detect_starter(towards_goal,same_side_as_root);
  slices[si].starter = slices[towards_goal].starter;
  TraceValue("%u\n",slices[si].starter);

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

  slice_solve_postkey(slices[si].u.pipe.u.branch_fork.towards_goal);

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

  result = slice_must_starter_resign_hashed(slices[si].u.pipe.u.branch_fork.towards_goal,
                                            just_moved);

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
                                 slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue deallocating a branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 * @return true iff si and its children have been successfully
 *         deallocated
 */
static boolean traverse_and_deallocate(slice_index si, slice_traversal *st)
{
  boolean const result = slice_traverse_children(si,st);
  dealloc_slice_index(si);
  return result;
}

/* Store slice representing play after branch in object representing
 * traversal, then continue deallocating the branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 */
static boolean traverse_and_deallocate_branch_fork(slice_index si,
                                                   slice_traversal *st)
{
  boolean result;
  slice_index * const to_be_found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *to_be_found = slices[si].u.pipe.u.branch_fork.towards_goal;

  result = traverse_slices(slices[si].u.pipe.next,st);
  dealloc_slice_index(si);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean traverse_and_deallocate_leaf(slice_index si,
                                            slice_traversal *st)
{
  boolean const result = true;
  slice_index * const to_be_found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *to_be_found = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_to_fork_deallocators[] =
{
  &traverse_and_deallocate,             /* STBranchDirect */
  &traverse_and_deallocate,             /* STBranchDirectDefender */
  &traverse_and_deallocate,             /* STBranchHelp */
  &traverse_and_deallocate,             /* STBranchSeries */
  &traverse_and_deallocate_branch_fork, /* STBranchFork */
  &traverse_and_deallocate_leaf,        /* STLeafDirect */
  &traverse_and_deallocate_leaf,        /* STLeafHelp */
  &traverse_and_deallocate_leaf,        /* STLeafSelf */
  &traverse_and_deallocate_leaf,        /* STLeafForced */
  0,                                    /* STReciprocal */
  0,                                    /* STQuodlibet */
  0,                                    /* STNot */
  0,                                    /* STMoveInverter */
  &traverse_and_deallocate,             /* STHelpRoot */
  &traverse_and_deallocate,             /* STHelpAdapter */
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

/* Find the next slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(SliceType type, slice_index si)
{
  slice_index result = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  do
  {
    result = slices[result].u.pipe.next;
  } while (result!=no_slice
           && result!=si
           && slices[result].type!=type);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
