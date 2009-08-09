#include "pybraser.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "pyslice.h"
#include "pymovein.h"
#include "pyseries.h"
#include "pybrafrk.h"
#include "pyreflxg.h"
#include "pypipe.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Allocate a STBranchSeries slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_ser_slice(stip_length_type length,
                                   stip_length_type min_length,
                                   slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STBranchSeries; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;

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
who_decides_on_starter branch_ser_detect_starter(slice_index si,
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
boolean branch_ser_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n number of moves to reach the end state
 * @return true iff >= 1 solution was found
 */
boolean branch_ser_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const next_slice = slices[si].u.pipe.next;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  active_slice[nbply+1] = si;
  genmove(side_at_move);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_solve_in_n(next_slice,n-1))
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

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void branch_ser_solve_continuations_in_n(table continuations,
                                         slice_index si,
                                         stip_length_type n)
{
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_solve_in_n(slices[si].u.pipe.next,n-1))
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

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_ser_has_solution_in_n(slice_index si, stip_length_type n)
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
        && series_has_solution_in_n(slices[si].u.pipe.next,n-1))
      result = true;

    repcoup();
  }
    
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/** series adapter *****************************************************/

/* Allocate a STSeriesAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_series_adapter_slice(stip_length_type length,
                                       stip_length_type min_length,
                                       slice_index fork,
                                       slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STSeriesAdapter; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.series_adapter.length = length;
  slices[result].u.pipe.u.series_adapter.min_length = min_length;
  slices[result].u.pipe.u.series_adapter.fork = fork;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Promote a slice that was created as STSeriesAdapter to STSeriesRoot
 * because the assumption that the slice is nested in some other slice
 * turned out to be wrong.
 * @param adapter identifies slice to be promoted
 */
void series_adapter_promote_to_toplevel(slice_index adapter)
{
  slice_index const branch = slices[adapter].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STSeriesAdapter);
  assert(slices[adapter].u.pipe.u.series_adapter.length-slack_length_series==1);
  assert(slices[branch].type==STBranchSeries);

  slices[adapter].type = STSeriesRoot;
  slices[adapter].u.pipe.next = slices[branch].u.pipe.next;
  dealloc_slice_index(branch);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_adapter_solve(slice_index si)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const full_length = slices[si].u.pipe.u.series_adapter.length;
  stip_length_type len = slices[si].u.pipe.u.series_adapter.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_series);

  if (len==slack_length_series)
  {
    if (series_solve_in_n(slices[si].u.pipe.u.series_adapter.fork,len))
    {
      result = true;
      FlagShortSolsReached = true;
    }
    else
      ++len;
  }
  
  while (len<full_length && !result)
  {
    if (series_solve_in_n(next,len))
    {
      result = true;
      FlagShortSolsReached = true;
    }

    ++len;
  }

  result = result || series_solve_in_n(next,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 */
void series_adapter_solve_postkey(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.pipe.u.series_adapter.length==slack_length_series+1);
  slice_solve_postkey(slices[si].u.pipe.u.series_adapter.fork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void series_adapter_solve_continuations(table continuations, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.pipe.u.series_adapter.length;
  stip_length_type len = slices[si].u.pipe.u.series_adapter.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<full_length && !solution_found)
  {
    series_solve_continuations_in_n(continuations,next,len);
    if (table_length(continuations)>0)
    {
      solution_found = true;
      FlagShortSolsReached = true;
    }

    ++len;
  }

  if (!solution_found)
    series_solve_continuations_in_n(continuations,next,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void series_adapter_root_write_key(slice_index si, attack_type type)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  slice_root_write_key(fork,type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void series_adapter_write_unsolvability(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[fork].type==STBranchFork);
  slice_write_unsolvability(to_goal);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean series_adapter_must_starter_resign(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[fork].type==STBranchFork);
  result = slice_must_starter_resign(to_goal);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean series_adapter_has_non_starter_solved(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;
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

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean series_adapter_has_starter_apriori_lost(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_apriori_lost(to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean series_adapter_has_starter_won(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_won(to_goal);

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
boolean series_adapter_has_starter_reached_goal(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;
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
boolean series_adapter_is_goal_reached(Side just_moved, slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;
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

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean series_adapter_has_solution(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.pipe.u.series_adapter.length;
  stip_length_type len = slices[si].u.pipe.u.series_adapter.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<=full_length)
    if (series_has_solution_in_n(slices[si].u.pipe.next,len))
    {
      result = true;
      break;
    }
    else
      ++len;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/*************** root *****************/

/* Allocate a STSeriesRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_series_root_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index fork)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  slices[result].type = STSeriesRoot; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = fork;
  slices[result].u.pipe.u.series_adapter.length = length;
  slices[result].u.pipe.u.series_adapter.min_length = min_length;
  slices[result].u.pipe.u.series_adapter.fork = fork;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a series pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
static void shorten_series_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.pipe.u.series_adapter.length;
  if (slices[pipe].u.pipe.u.series_adapter.min_length
      >slices[pipe].u.pipe.u.series_adapter.length)
    --slices[pipe].u.pipe.u.series_adapter.min_length;
  TraceValue("%u\n",slices[pipe].u.pipe.u.series_adapter.length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index series_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index const fork = slices[si].u.pipe.u.series_adapter.fork;
  slice_index const to_goal = slices[fork].u.pipe.u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = to_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a root series branch. Reduces the length members of
 * slices[root] and resets the next member to the appropriate
 * position.
 * @param root index of the series root slice
 */
static void shorten_root_branch(slice_index root)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  slice_index const fork = slices[root].u.pipe.u.series_adapter.fork;
  slice_index const branch = slices[fork].u.pipe.next;
  assert(slices[fork].type==STBranchFork);
  assert(slices[fork].type==STBranchSeries);
  if (slices[root].u.pipe.u.series_adapter.length==slack_length_series+2)
  {
    slices[fork].u.pipe.next = no_slice;
    dealloc_slice_index(branch);
  }

  shorten_series_pipe(root);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a series branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return - no_slice if not applicable (already shortened)
 *         - slice representing subsequent play if root has 1 half-move only
 *         - root (shortened) otherwise
 */
slice_index series_root_shorten_series_play(slice_index root)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root].type==STSeriesRoot);
  assert(slices[root].u.pipe.u.series_adapter.length>slack_length_series);

  if (slices[root].u.pipe.u.series_adapter.length==slack_length_series+1)
  {
    result = branch_deallocate_to_fork(root);
    if (slices[result].type==STMoveInverter)
    {
      slice_index const mi = result;
      result = slices[result].u.pipe.next;
      dealloc_slice_index(mi);
    }
  }
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

/* Solve a composite slice with series play at root level
 * @param n number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean series_root_solve_in_n(slice_index root, stip_length_type n)
{
  boolean result;
  slice_index const fork = slices[root].u.pipe.u.series_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slices[root].u.pipe.u.series_adapter.length)
    result = branch_ser_solve_in_n(root,n);
  else
    result = series_solve_in_n(fork,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a branch slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_root_solve(slice_index root)
{
  boolean result = false;
  stip_length_type const
      full_length = slices[root].u.pipe.u.series_adapter.length;
  stip_length_type len = (OptFlag[restart]
                          ? full_length
                          : slices[root].u.pipe.u.series_adapter.min_length);
  slice_index const fork = slices[root].u.pipe.u.series_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  init_output(root);

  TraceValue("%u",slices[root].u.pipe.u.series_adapter.min_length);
  TraceValue("%u\n",slices[root].u.pipe.u.series_adapter.length);

  assert(slices[root].u.pipe.u.series_adapter.min_length>=slack_length_series);

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
      if (series_solve_in_n(fork,len))
        result = true;
    }

    ++len;
  }

  if (result && OptFlag[stoponshort])
  {
    TraceText("aborting because of short solutions\n");
    FlagShortSolsReached = true;
  }
  else if (isIntelligentModeActive)
    result = Intelligent(root,full_length,full_length);
  else
    result = branch_ser_solve_in_n(root,full_length);

  if (OptFlag[maxsols] && solutions>=maxsolutions)
    /* signal maximal number of solutions reached to outer world */
    FlagMaxSolsReached = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean series_root_has_solution(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.pipe.u.series_adapter.length;
  stip_length_type len = slices[si].u.pipe.u.series_adapter.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<=full_length)
    if (branch_ser_has_solution_in_n(si,len))
    {
      result = true;
      break;
    }
    else
      ++len;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a top level series branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
static slice_index alloc_toplevel_series_branch(stip_length_type length,
                                                stip_length_type min_length,
                                                slice_index next)
{
  slice_index result;
  slice_index fork;
  stip_length_type const fork_min_length = (min_length==slack_length_series
                                            ? slack_length_series
                                            : min_length-1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  assert(length>slack_length_series);

  fork = alloc_branch_fork_slice(length-1,fork_min_length,no_slice,next);
  result = alloc_series_root_slice(length,min_length,fork);

  if (length-slack_length_series>1)
  {
    slice_index const branch = alloc_branch_ser_slice(length,min_length,fork);
    shorten_series_pipe(branch);
    slices[fork].u.pipe.next = alloc_move_inverter_slice(branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a nested series branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
static slice_index alloc_nested_series_branch(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index next)
{
  slice_index result;
  slice_index fork;
  slice_index branch;
  stip_length_type const fork_min_length = (min_length==slack_length_series
                                            ? slack_length_series
                                            : min_length-1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  assert(length>slack_length_series);

  fork = alloc_branch_fork_slice(length-1,fork_min_length,no_slice,next);
  branch = alloc_branch_ser_slice(length,min_length,fork);
  result = alloc_series_adapter_slice(length,min_length,fork,branch);

  if (length-slack_length_series>1)
    slices[fork].u.pipe.next = alloc_move_inverter_slice(branch);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a series branch.
 * @param level is this a top-level branch or one nested into another
 *              branch?
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of adapter slice of allocated series branch
 */
slice_index alloc_series_branch(branch_level level,
                                stip_length_type length,
                                stip_length_type min_length,
                                slice_index next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",level);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  if (level==toplevel_branch)
    result = alloc_toplevel_series_branch(length,min_length,next);
  else
    result = alloc_nested_series_branch(length,min_length,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
