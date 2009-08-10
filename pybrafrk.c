#include "pybrafrk.h"
#include "pydirect.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBranchFork slice.
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_fork_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index next,
                                    slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slices[result].type = STBranchFork;
  slices[result].starter = slices[towards_goal].starter;
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch_fork.length = length;
  slices[result].u.pipe.u.branch_fork.min_length = min_length;
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

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean branch_fork_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    result = slice_solve(slices[si].u.pipe.u.branch_fork.towards_goal);
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = series_solve_in_n(next,n);
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
boolean branch_fork_series_has_solution_in_n(slice_index si,
                                             stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    result = slice_has_solution(slices[si].u.pipe.u.branch_fork.towards_goal);
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = series_has_solution_in_n(next,n);
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
void branch_fork_series_solve_continuations_in_n(table continuations,
                                                 slice_index si,
                                                 stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    slice_solve_continuations(continuations,
                              slices[si].u.pipe.u.branch_fork.towards_goal);
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    series_solve_continuations_in_n(continuations,next,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  result = slice_has_starter_apriori_lost(slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_fork_has_starter_won(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_won(slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_fork_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_fork_are_threats_refuted_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n,
                                             int curr_max_nr_nontrivial)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (n==slack_length_direct)
  {
    slice_index const togoal = slices[si].u.pipe.u.branch_fork.towards_goal;
    result = slice_are_threats_refuted(threats,togoal);
  }
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = direct_are_threats_refuted_in_n(threats,
                                             next,
                                             n,
                                             curr_max_nr_nontrivial);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff >= 1 solution has been found
 */
boolean branch_fork_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      int curr_max_nr_nontrivial)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const togoal = slices[si].u.pipe.u.branch_fork.towards_goal;
  stip_length_type const moves_played = (slices[si].u.pipe.u.branch_fork.length
                                         -n
                                         +slack_length_direct);
  stip_length_type const
      min_length = slices[si].u.pipe.u.branch_fork.min_length;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",moves_played);
  
  if (moves_played>min_length && slice_has_non_starter_solved(togoal))
    result = true;
  else if (moves_played>=min_length && slice_has_solution(togoal))
    result = true;
  else if (n>slack_length_direct
           && direct_has_solution_in_n(next,n,curr_max_nr_nontrivial))
    result = true;

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
void branch_fork_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_fork_root_write_key(slice_index si, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  slice_root_write_key(slices[si].u.pipe.u.branch_fork.towards_goal,type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_fork_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_write_unsolvability(slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_fork_has_non_starter_solved(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(slices[si].u.pipe.u.branch_fork.towards_goal);

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

  result = slice_has_starter_reached_goal(slices[si].u.pipe.u.branch_fork.towards_goal);

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

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_fork_solve_continuations(table continuations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_solve_continuations(continuations,
                            slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff no chance is left
 */
boolean branch_fork_must_starter_resign_hashed(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_must_starter_resign_hashed(slices[si].u.pipe.u.branch_fork.towards_goal);

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

/* Write that the non-starter has solved (i.e. in a self stipulation)
 * @param si slice index
 */
void branch_fork_write_non_starter_has_solved(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_write_non_starter_has_solved(slices[si].u.pipe.u.branch_fork.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  &traverse_and_deallocate,             /* STDirectRoot */
  &traverse_and_deallocate,             /* STDirectAdapter */
  &traverse_and_deallocate,             /* STDirectDefenderRoot */
  &traverse_and_deallocate,             /* STHelpRoot */
  &traverse_and_deallocate,             /* STHelpAdapter */
  &traverse_and_deallocate,             /* STHelpHashed */
  &traverse_and_deallocate,             /* STSeriesRoot */
  &traverse_and_deallocate,             /* STSeriesAdapter */
  &traverse_and_deallocate,             /* STSeriesHashed */
  0,                                    /* STSelfCheckGuard */
  0,                                    /* STReflexGuard */
  0,                                    /* STRestartGuard */
  0,                                    /* STGoalReachableGuard */
  &traverse_and_deallocate              /* STKeepMatingGuard */
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
