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
 * @param towards_goal identifies slice that leads towards goal
 * @return index of allocated slice
 */
static slice_index alloc_branch_ser_slice(stip_length_type length,
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

  slices[result].type = STBranchSeries; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.help_root.length = length;
  slices[result].u.pipe.u.help_root.min_length = min_length;
  slices[result].u.pipe.u.help_root.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Promote a slice that was created as STBranchSeries to STSeriesRoot
 * because the assumption that the slice is nested in some other slice
 * turned out to be wrong.
 * @param branch identifies slice to be promoted
 */
void branch_ser_promote_to_toplevel(slice_index branch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  assert(slices[branch].u.pipe.u.help_root.length-slack_length_series==1);
  assert(slices[branch].type==STBranchSeries);

  slices[branch].type = STSeriesRoot;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_ser_detect_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  stip_detect_starter_param_type * const param = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    boolean const save_same_starter_as_root = param->same_starter_as_root;
    param->same_starter_as_root = !save_same_starter_as_root;
    result = slice_traverse_children(si,st);
    param->same_starter_as_root = save_same_starter_as_root;
    slices[si].starter = (slices[next].starter==no_side
                          ? no_side
                          : advers(slices[next].starter));
  }
  else
    result = true;

  TraceValue("%u\n",slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
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

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void branch_ser_solve_threats_in_n(table threats,
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

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_ser_are_threats_refuted(table threats,
                                       stip_length_type len_threat,
                                       slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n = slices[si].u.pipe.u.help_root.length;
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
          && series_has_solution_in_n(next,n-1))
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

/*************** root *****************/

/* Allocate a STSeriesRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_root_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index next,
                                    slice_index towards_goal,
                                    slice_index short_sols)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParam("%u",short_sols);
  TraceFunctionParamListEnd();

  slices[result].type = STSeriesRoot; 
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

/* Shorten a series pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
static void shorten_series_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.pipe.u.help_root.length;
  if (slices[pipe].u.pipe.u.help_root.min_length
      >slices[pipe].u.pipe.u.help_root.length)
    --slices[pipe].u.pipe.u.help_root.min_length;
  TraceValue("%u\n",slices[pipe].u.pipe.u.help_root.length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean series_root_make_setplay_slice(slice_index si,
                                       struct slice_traversal *st)
{
  boolean const result = true;
  slice_index * const next_set_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *next_set_slice = slices[si].u.pipe.u.help_root.towards_goal;

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
  slice_index fork;
  slice_index branch;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  fork = branch_find_slice(STBranchFork,root);
  branch = slices[fork].u.pipe.next;
  assert(fork!=no_slice);
  assert(slices[branch].type==STBranchSeries);
  if (slices[root].u.pipe.u.help_root.length==slack_length_series+2)
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
  assert(slices[root].u.pipe.u.help_root.length>slack_length_series);

  if (slices[root].u.pipe.u.help_root.length==slack_length_series+1)
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
 * @param root slice index
 * @param n exact number of moves to reach the end state
 * @return true iff >= 1 solution was found
 */
boolean series_root_solve_in_n(slice_index root, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slices[root].u.pipe.u.help_root.length)
    result = branch_ser_solve_in_n(root,n);
  else
    result = series_solve_in_n(slices[root].u.pipe.u.help_root.short_sols,n);

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
      full_length = slices[root].u.pipe.u.help_root.length;
  stip_length_type len = (OptFlag[restart]
                          ? full_length
                          : slices[root].u.pipe.u.help_root.min_length);
  slice_index const fork = branch_find_slice(STBranchFork,root);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  init_output(root);

  TraceValue("%u",slices[root].u.pipe.u.help_root.min_length);
  TraceValue("%u\n",slices[root].u.pipe.u.help_root.length);

  assert(fork!=no_slice);
  assert(slices[root].u.pipe.u.help_root.min_length>=slack_length_series);

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
has_solution_type series_root_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.pipe.u.help_root.length;
  stip_length_type len = slices[si].u.pipe.u.help_root.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<=full_length && result==has_no_solution)
  {
    branch_ser_has_solution_in_n(si,len);
    ++len;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a top level series branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_toplevel_series_branch(stip_length_type length,
                                                stip_length_type min_length,
                                                slice_index towards_goal)
{
  slice_index result;
  slice_index fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_series);

  fork = alloc_branch_fork_slice(length-1,min_length-1,no_slice,towards_goal);
  result = alloc_series_root_slice(length,min_length,
                                   fork,towards_goal,
                                   fork);

  if (length-slack_length_series>1)
  {
    slice_index const branch = alloc_branch_ser_slice(length,min_length,
                                                      fork,towards_goal);
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
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_nested_series_branch(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index towards_goal)
{
  slice_index result;
  slice_index fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_series);

  fork = alloc_branch_fork_slice(length-1,min_length-1,no_slice,towards_goal);
  result = alloc_branch_ser_slice(length,min_length,fork,towards_goal);

  if (length-slack_length_series>1)
    slices[fork].u.pipe.next = alloc_move_inverter_slice(result);

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
 * @param towards_goal identifies slice leading towards goal
 * @return index of adapter slice of allocated series branch
 */
slice_index alloc_series_branch(branch_level level,
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
    result = alloc_toplevel_series_branch(length,min_length,towards_goal);
  else
    result = alloc_nested_series_branch(length,min_length,towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
