#include "pybrah.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "pyslice.h"
#include "pyhelp.h"
#include "pybrafrk.h"
#include "pyreflxg.h"
#include "pypipe.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Allocate a STBranchHelp slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies slice theat leads towards the goal
 * @return index of allocated slice
 */
static slice_index alloc_branch_h_slice(stip_length_type length,
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

  slices[result].type = STBranchHelp; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.u.branch.towards_goal = towards_goal;

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
who_decides_on_starter branch_h_detect_starter(slice_index si,
                                               boolean same_side_as_root)
{
  who_decides_on_starter result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    result = slice_detect_starter(next,!same_side_as_root);
    slices[si].starter = (slices[next].starter==no_side
                          ? no_side
                          : advers(slices[next].starter));
  }
  else
    result = leaf_decides_on_starter;

  TraceValue("%u\n",slices[si].starter);

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
boolean branch_h_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,
                                 slices[si].u.pipe.u.branch.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_solve_in_n(slice_index si, stip_length_type n)

{
  boolean result = false;
  slice_index const next_slice = slices[si].u.pipe.next;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help_solve_in_n(next_slice,n-1))
      result = true;

    repcoup();

    if (OptFlag[maxsols] && solutions>=maxsolutions)
      break;

    if (periods_counter>=nr_periods)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void branch_h_solve_threats_in_n(table threats,
                                 slice_index si,
                                 stip_length_type n)
{
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help_solve_in_n(slices[si].u.pipe.next,n-1))
    {
      append_to_top_table();
      coupfort();
    }

    repcoup();

    if (OptFlag[maxsols] && solutions>=maxsolutions)
      break;

    if (periods_counter>=nr_periods)
      break;
  }
    
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_h_are_threats_refuted(table threats, slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;

    active_slice[nbply+1] = si;
    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats)
          && help_has_solution_in_n(next,n-1))
        ++nr_successful_threats;

      repcoup();
    }

    finply();

    /* this happens if >=1 threat no longer works or some threats can
     * no longer be played after the defense.
     */
    result = nr_successful_threats<table_length(threats);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_has_solution_in_n(slice_index si, stip_length_type n)
{
  Side const side_at_move = slices[si].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  while (encore() && !result)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help_has_solution_in_n(slices[si].u.pipe.next,n-1))
      result = true;

    repcoup();
  }
    
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/** help adapter *****************************************************/

/* Allocate a STHelpAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param next identifies slice that leads towards the goal
 * @return index of allocated slice
 */
static slice_index alloc_help_adapter_slice(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index next,
                                            slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STHelpAdapter; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Promote a slice that was created as STHelpAdapter to STHelpRoot
 * because the assumption that the slice is nested in some other slice
 * turned out to be wrong.
 * @param adapter identifies slice to be promoted
 */
void help_adapter_promote_to_toplevel(slice_index adapter)
{
  slice_index const branch = slices[adapter].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);
  assert(slices[adapter].u.pipe.u.branch.length-slack_length_help==1);
  assert(slices[branch].type==STBranchHelp);

  slices[adapter].type = STHelpRoot;
  slices[adapter].u.pipe.next = slices[branch].u.pipe.next;
  dealloc_slice_index(branch);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_adapter_solve(slice_index si)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_help);

  while (len<full_length && !result)
  {
    if (help_solve_in_n(next,len))
    {
      result = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  result = result || help_solve_in_n(next,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_adapter_solve_postkey(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.pipe.u.branch.length==slack_length_help+1);
  result = slice_solve_postkey(slices[si].u.pipe.u.branch.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void help_adapter_solve_threats(table threats, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<full_length && !solution_found)
  {
    help_solve_threats_in_n(threats,next,len);
    if (table_length(threats)>0)
      solution_found = true;

    len += 2;
  }

  if (!solution_found)
    help_solve_threats_in_n(threats,next,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void help_adapter_write_unsolvability(slice_index si)
{
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_write_unsolvability(to_goal);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean help_adapter_has_non_starter_solved(slice_index si)
{
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(to_goal);

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
boolean help_adapter_has_starter_reached_goal(slice_index si)
{
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_reached_goal(to_goal);

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
boolean help_adapter_is_goal_reached(Side just_moved, slice_index si)
{
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean help_adapter_are_threats_refuted(table threats, slice_index si)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_are_threats_refuted(threats,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_adapter_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<=full_length && result==has_no_solution)
  {
    if (help_has_solution_in_n(slices[si].u.pipe.next,len))
      result = has_solution;

    len += 2;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/*************** root *****************/

/* Allocate a STHelpRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return index of allocated slice
 */
static slice_index alloc_help_root_slice(stip_length_type length,
                                         stip_length_type min_length,
                                         slice_index next,
                                         slice_index towards_goal,
                                         slice_index short_sols)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParam("%u",short_sols);
  TraceFunctionParamListEnd();

  slices[result].type = STHelpRoot; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.help_root.length = length;
  slices[result].u.pipe.u.help_root.min_length = min_length;
  slices[result].u.pipe.u.help_root.towards_goal = towards_goal;
  slices[result].u.pipe.u.help_root.short_sols = short_sols;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a help pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
static void shorten_help_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.pipe.u.branch.length;
  --slices[pipe].u.pipe.u.branch.min_length;
  if (slices[pipe].u.pipe.u.branch.min_length<slack_length_help)
    slices[pipe].u.pipe.u.branch.min_length += 2;
  slices[pipe].starter = (slices[pipe].starter==no_side
                          ? no_side
                          : advers(slices[pipe].starter));
  TraceValue("%u",slices[pipe].starter);
  TraceValue("%u",slices[pipe].u.pipe.u.branch.length);
  TraceValue("%u\n",slices[pipe].u.pipe.u.branch.min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help branch that is the root of set play. Reduces the
 * length members of slices[root] and rewires the members to the
 * appropriate positions.
 * @param root index of the help root slice to be shortened
 */
static void shorten_setplay_root_branch(slice_index root)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  if ((slices[root].u.pipe.u.help_root.length-slack_length_help)%2==0)
  {
    slice_index const selfcheck1 = slices[root].u.pipe.next;
    slice_index const branch1 = slices[selfcheck1].u.pipe.next;
    slice_index const selfcheck2 = slices[branch1].u.pipe.next;
    assert(slices[selfcheck1].type==STSelfCheckGuard);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[selfcheck2].type==STSelfCheckGuard);
    slices[root].u.pipe.next = selfcheck2;
    slices[root].u.pipe.u.help_root.short_sols = branch1;
  }
  else
  {
    slice_index const selfcheck1 = slices[root].u.pipe.next;
    slice_index const fork = slices[selfcheck1].u.pipe.next;
    slice_index const branch1 = slices[fork].u.pipe.next;
    slice_index const selfcheck2 = slices[branch1].u.pipe.next;
    assert(slices[selfcheck1].type==STSelfCheckGuard);
    assert(slices[fork].type==STBranchFork);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[selfcheck2].type==STSelfCheckGuard);
    slices[root].u.pipe.next = selfcheck2;
    slices[root].u.pipe.u.help_root.short_sols = fork;
  }

  shorten_help_pipe(root);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index help_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index const to_goal = slices[si].u.pipe.u.help_root.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation();

  assert(slices[si].type==STHelpRoot);

  assert(slices[si].u.pipe.u.help_root.length>slack_length_help);

  if (slices[si].u.pipe.u.help_root.length==slack_length_help+1)
    result = to_goal;
  else
  {
    result = copy_slice(si);
    shorten_setplay_root_branch(result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a root help branch. Reduces the length members of
 * slices[root] and resets the next member to the appropriate
 * position.
 * @param root index of the help root slice
 */
static void shorten_root_branch(slice_index root)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  if ((slices[root].u.pipe.u.help_root.length-slack_length_help)%2==0)
  {
    slice_index const branch1 = slices[root].u.pipe.next;
    slice_index const fork = slices[branch1].u.pipe.next;
    assert(slices[fork].type==STBranchFork);
    assert(slices[branch1].type==STBranchHelp);
    slices[root].u.pipe.next = fork;
    slices[root].u.pipe.u.help_root.short_sols = slices[fork].u.pipe.next;
    if (slices[root].u.pipe.u.help_root.length==slack_length_help+2)
    {
      assert(slices[fork].u.pipe.next==no_slice);
      dealloc_slice_index(branch1);
    }
    else
    {
      slices[branch1].u.pipe.u.help_root.length -= 2;
      slices[fork].u.pipe.u.help_root.length -= 2;
    }
  }
  else
  {
    slice_index const fork = slices[root].u.pipe.next;
    slice_index const branch1 = slices[fork].u.pipe.next;
    slice_index const branch2 = slices[branch1].u.pipe.next;
    assert(slices[fork].type==STBranchFork);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[branch2].type==STBranchHelp);
    slices[root].u.pipe.next = branch2;
    slices[root].u.pipe.u.help_root.short_sols = fork;
    if (slices[root].u.pipe.u.help_root.length==slack_length_help+3)
    {
      slices[fork].u.pipe.next = no_slice;
      dealloc_slice_index(branch1);
    }
    else
    {
      slices[branch1].u.pipe.u.help_root.length -= 2;
      slices[fork].u.pipe.u.help_root.length -= 2;
    }
  }

  shorten_help_pipe(root);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return - no_slice if not applicable (already shortened)
 *         - slice representing subsequent play if root has 1 half-move only
 *         - root (shortened) otherwise
 */
slice_index help_root_shorten_help_play(slice_index root)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root].type==STHelpRoot);
  assert(slices[root].u.pipe.u.help_root.length>slack_length_help);

  if (slices[root].u.pipe.u.help_root.length==slack_length_help+1)
    result = branch_deallocate_to_fork(root);
  else
  {
    shorten_root_branch(root);
    result = root;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve full-length solutions in exactly n in help play at root level
 * @param root slice index
 * @param n exact number of half moves
 * @return true iff >=1 solution was found
 */
boolean help_root_solve_in_n(slice_index root, stip_length_type n)
{
  boolean result;
  slice_index const short_sols = slices[root].u.pipe.u.help_root.short_sols;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slices[root].u.pipe.u.help_root.length)
    result = branch_h_solve_in_n(root,n);
  else
    result = help_solve_in_n(short_sols,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_root_solve(slice_index root)
{
  boolean result = false;
  slice_index const short_sols = slices[root].u.pipe.u.help_root.short_sols;
  stip_length_type const full_length = slices[root].u.pipe.u.help_root.length;
  stip_length_type len = (OptFlag[restart]
                          ? full_length
                          : slices[root].u.pipe.u.help_root.min_length);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  init_output(root);

  TraceValue("%u",slices[root].u.pipe.u.help_root.min_length);
  TraceValue("%u\n",slices[root].u.pipe.u.help_root.length);

  assert(slices[root].u.pipe.u.help_root.min_length>=slack_length_help);

  move_generation_mode = move_generation_not_optimized;

  FlagShortSolsReached = false;
  solutions = 0;

  while (len<full_length
         && !(OptFlag[stoponshort] && result))
  {
    if (isIntelligentModeActive)
    {
      if (Intelligent(root,len,full_length))
        result = true;
    }
    else
    {
      if (help_solve_in_n(short_sols,len))
        result = true;
    }

    len += 2;
  }

  if (result && OptFlag[stoponshort])
  {
    TraceText("aborting because of short solutions\n");
    FlagShortSolsReached = true;
  }
  else if (isIntelligentModeActive)
    result = Intelligent(root,full_length,full_length);
  else
    result = branch_h_solve_in_n(root,full_length);

  if (OptFlag[maxsols] && solutions>=maxsolutions)
    /* signal maximal number of solutions reached to outer world */
    FlagMaxSolsReached = true;

  write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_root_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.pipe.u.help_root.length;
  stip_length_type len = slices[si].u.pipe.u.help_root.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<=full_length && result==has_no_solution)
  {
    result = branch_h_has_solution_in_n(si,len);
    len += 2;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a top level help branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice that leads towards the goal
 * @return index of allocated slice
 */
static slice_index alloc_toplevel_help_branch(stip_length_type length,
                                              stip_length_type min_length,
                                              slice_index towards_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);

  if (length%2==0)
  {
    slice_index const fork = alloc_branch_fork_slice(length-2,min_length-2,
                                                     no_slice,towards_goal);
    if (length-slack_length_help==2)
    {
      slice_index const branch = alloc_branch_h_slice(length,min_length,
                                                      fork,towards_goal);
      result = alloc_help_root_slice(length,min_length,
                                     branch,towards_goal,fork);
    }
    else
    {
      slice_index const branch1 = alloc_branch_h_slice(length,min_length,
                                                       fork,towards_goal);
      slice_index const branch2 = alloc_branch_h_slice(length-2,min_length,
                                                       branch1,towards_goal);
      shorten_help_pipe(branch1);
      result = alloc_help_root_slice(length,min_length,
                                     branch1,towards_goal,fork);

      slices[fork].u.pipe.next = branch2;
      TraceValue("%u\n",slices[fork].u.pipe.next);
    }
  }
  else
  {
    slice_index const fork = alloc_branch_fork_slice(length-1,min_length-1,
                                                     no_slice,towards_goal);
    if (length-slack_length_help==1)
      result = alloc_help_root_slice(length,min_length,
                                     fork,towards_goal,no_slice);
    else
    {
      slice_index const branch1 = alloc_branch_h_slice(length-2,min_length,
                                                       fork,towards_goal);
      slice_index const branch2 = alloc_branch_h_slice(length,min_length,
                                                       branch1,towards_goal);
      shorten_help_pipe(branch2);
      result = alloc_help_root_slice(length,min_length,
                                     fork,towards_goal,branch1);

      slices[fork].u.pipe.next = branch2;
      TraceValue("%u\n",slices[fork].u.pipe.next);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a nested help branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice that leads towards the goal
 * @return index of allocated slice
 */
static slice_index alloc_nested_help_branch(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index towards_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);

  if (length%2==0)
  {
    slice_index const fork = alloc_branch_fork_slice(length-2,min_length-2,
                                                     no_slice,towards_goal);
    if (length-slack_length_help==2)
    {
      slice_index const branch1 = alloc_branch_h_slice(length,min_length,
                                                       fork,towards_goal);
      slice_index const branch2 = alloc_branch_h_slice(length,min_length,
                                                       branch1,towards_goal);
      shorten_help_pipe(branch1);
      result = alloc_help_adapter_slice(length,min_length,branch2,towards_goal);
    }
    else
    {
      slice_index const branch1 = alloc_branch_h_slice(length,min_length,
                                                       fork,towards_goal);
      slice_index const branch2 = alloc_branch_h_slice(length,min_length,
                                                       branch1,towards_goal);
      shorten_help_pipe(branch1);
      result = alloc_help_adapter_slice(length,min_length,branch2,towards_goal);

      slices[fork].u.pipe.next = branch2;
      TraceValue("%u\n",slices[fork].u.pipe.next);
    }
  }
  else
  {
    slice_index const fork = alloc_branch_fork_slice(length-1,min_length-1,
                                                     no_slice,towards_goal);
    if (length-slack_length_help==1)
    {
      slice_index const branch = alloc_branch_h_slice(length,min_length,
                                                      fork,towards_goal);
      result = alloc_help_adapter_slice(length,min_length,branch,towards_goal);
    }
    else
    {
      slice_index const branch1 = alloc_branch_h_slice(length,min_length,
                                                       fork,towards_goal);
      slice_index const branch2 = alloc_branch_h_slice(length,min_length,
                                                       branch1,towards_goal);
      shorten_help_pipe(branch2);
      result = alloc_help_adapter_slice(length,min_length,branch1,towards_goal);

      slices[fork].u.pipe.next = branch2;
      TraceValue("%u\n",slices[fork].u.pipe.next);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a help branch.
 * @param level is this a top-level branch or one nested into another
 *              branch?
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards the goal
 * @return index of adapter slice of allocated help branch
 */
slice_index alloc_help_branch(branch_level level,
                              stip_length_type length,
                              stip_length_type min_length,
                              slice_index towards_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",level);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  if (level==toplevel_branch)
    result = alloc_toplevel_help_branch(length,min_length,towards_goal);
  else
    result = alloc_nested_help_branch(length,min_length,towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
