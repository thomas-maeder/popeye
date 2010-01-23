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
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Allocate a STBranchHelp slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_h_slice(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(proxy_to_goal==no_slice || slices[proxy_to_goal].type==STProxy);

  result = alloc_branch(STBranchHelp,length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STHelpRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return index of allocated slice
 */
static slice_index alloc_help_root_slice(stip_length_type length,
                                         stip_length_type min_length,
                                         slice_index proxy_to_goal,
                                         slice_index short_sols)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParam("%u",short_sols);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpRoot,length,min_length,proxy_to_goal);
  slices[result].u.pipe.u.help_root.short_sols = short_sols;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Promote a slice that was created as STBranchHelp to STHelpRoot
 * because the assumption that the slice is nested in some other slice
 * turned out to be wrong.
 * @param proxy identifies proxy to slice to be promoted
 * @return identifier of toplevel slice
 */
void branch_h_promote_to_toplevel(slice_index proxy)
{
  slice_index const branch = slices[proxy].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  assert(slices[branch].u.pipe.u.branch.length-slack_length_help==1);
  assert(slices[branch].type==STBranchHelp);

  {
    slice_index const root =
        alloc_help_root_slice(slices[branch].u.pipe.u.branch.length,
                              slices[branch].u.pipe.u.branch.min_length,
                              slices[branch].u.pipe.u.branch.towards_goal,
                              branch);
    branch_link(root,copy_slice(branch));

    slices[branch].u.pipe.u.branch.length -= 2;
    slices[branch].u.pipe.u.branch.min_length -=2;

    branch_link(proxy,root);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_h_detect_starter(slice_index si, slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    result = slice_traverse_children(si,st);
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
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;
    Side const attacker = slices[si].starter;
    slice_index const next = slices[si].u.pipe.next;
    stip_length_type const length = slices[si].u.pipe.u.branch.length;

    active_slice[nbply+1] = si;
    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats)
          && help_has_solution_in_n(next,length-1))
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

/*************** root *****************/

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
    slice_index const root_branch = slices[root].u.pipe.next;
    slice_index const branch1 = slices[root_branch].u.pipe.next;
    slice_index const fork = slices[branch1].u.pipe.next;
    assert(slices[root_branch].type==STBranchHelp);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[fork].type==STHelpFork);
    slices[root_branch].u.pipe.next = fork;
    slices[root].u.pipe.u.help_root.short_sols = branch1;
    shorten_help_pipe(root);
    shorten_help_pipe(root_branch);
  }
  else
  {
    slice_index const root_branch = slices[root].u.pipe.next;
    slice_index const fork = slices[root_branch].u.pipe.next;
    slice_index const branch1 = slices[fork].u.pipe.next;
    slice_index const proxy = slices[branch1].u.pipe.next;
    slice_index const branch2 = slices[proxy].u.pipe.next;
    assert(slices[root_branch].type==STBranchHelp);
    assert(slices[fork].type==STHelpFork);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[proxy].type==STProxy);
    assert(slices[branch2].type==STBranchHelp);
    slices[root_branch].u.pipe.next = branch2;
    slices[root].u.pipe.u.help_root.short_sols = fork;
    shorten_help_pipe(root);
    shorten_help_pipe(root_branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean help_root_make_setplay_slice(slice_index si,
                                     struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.pipe.u.help_root.length>slack_length_help);

  prod->sibling = si;

  if (slices[si].u.pipe.u.help_root.length==slack_length_help+1)
  {
    slice_index const proxy_to_goal = slices[si].u.pipe.u.help_root.towards_goal;
    assert(slices[proxy_to_goal].type==STProxy);
    prod->setplay_slice = slices[proxy_to_goal].u.pipe.next;
  }
  else
  {
    prod->setplay_slice = copy_slice(si);
    branch_link(prod->setplay_slice,copy_slice(slices[si].u.pipe.next));
    shorten_setplay_root_branch(prod->setplay_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Perform the necessary work for shorting a branch from an even to an
 * odd length
 * @param root STHelpRoot slice of branch to be shortened
 */
static void shorten_root_branch_even_to_odd(slice_index root)
{
  slice_index const root_branch = slices[root].u.pipe.next;
  slice_index const branch1 = slices[root_branch].u.pipe.next;
  slice_index const fork = slices[branch1].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root_branch].type==STBranchHelp);
  assert(slices[branch1].type==STBranchHelp);
  assert(slices[fork].type==STHelpFork);

  if (slices[root].u.pipe.u.help_root.length-slack_length_help==2)
  {
    assert(slices[fork].u.pipe.next==no_slice);
    dealloc_slice(branch1);
  }
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const branch2 = slices[fork].u.pipe.next;

    assert(slices[branch2].type==STBranchHelp);

    slices[branch1].u.pipe.u.help_root.length -= 2;
    slices[fork].u.pipe.u.help_root.length -= 2;
    slices[root].u.pipe.u.help_root.short_sols = proxy;

    branch_link(proxy,branch2);
  }

  branch_link(root_branch,fork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Perform the necessary work for shorting a branch from an odd to an
 * even length
 * @param root STHelpRoot slice of branch to be shortened
 */
static void shorten_root_branch_odd_to_even(slice_index root)
{
  slice_index const root_branch = slices[root].u.pipe.next;
  slice_index const fork = slices[root_branch].u.pipe.next;
  slice_index const branch1 = slices[fork].u.pipe.next;
  slice_index const proxy = slices[branch1].u.pipe.next;
  slice_index const branch2 = slices[proxy].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root_branch].type==STBranchHelp);
  assert(slices[fork].type==STHelpFork);
  assert(slices[branch1].type==STBranchHelp);
  assert(slices[proxy].type==STProxy);
  assert(slices[branch2].type==STBranchHelp);

  slices[root].u.pipe.u.help_root.short_sols = fork;
  dealloc_proxy_slice(proxy);

  if (slices[root].u.pipe.u.help_root.length-slack_length_help==3)
  {
    branch_link(root_branch,branch2);
    pipe_set_successor(fork,no_slice);
    dealloc_slice(branch1);
  }
  else
  {
    pipe_set_successor(root_branch,branch2);
    branch_link(branch1,branch2);
    slices[branch1].u.pipe.u.help_root.length -= 2;
  }

  slices[fork].u.pipe.u.help_root.length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a root help branch. Reduces the length members of
 * slices[root] and resets the next member to the appropriate
 * position.
 * @param root index of the help root slice
 */
static void shorten_root_branch(slice_index root)
{
  slice_index const root_branch = slices[root].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root_branch].type==STBranchHelp);

  if ((slices[root].u.pipe.u.help_root.length-slack_length_help)%2==0)
    shorten_root_branch_even_to_odd(root);
  else
    shorten_root_branch_odd_to_even(root);

  shorten_help_pipe(root);
  shorten_help_pipe(root_branch);

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

  TraceStipulation(root_slice);

  assert(slices[root].type==STHelpRoot);
  assert(slices[root].u.pipe.u.help_root.length>slack_length_help);

  if (slices[root].u.pipe.u.help_root.length==slack_length_help+1)
  {
    slice_index const proxy_to_goal = branch_deallocate_to_fork(root);
    assert(slices[proxy_to_goal].type==STProxy);
    result = slices[proxy_to_goal].u.pipe.next;
    dealloc_slice(proxy_to_goal);
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

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_root_root_solve(slice_index root)
{
  boolean result = false;
  slice_index const short_sols = slices[root].u.pipe.u.help_root.short_sols;
  slice_index const next = slices[root].u.pipe.next;
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
      if (Intelligent(short_sols,len,full_length))
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
    result = Intelligent(next,full_length,full_length);
  else
    result = help_solve_in_n(next,full_length);

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
  slice_index const next = slices[si].u.pipe.next;
  slice_index const short_sols = slices[si].u.pipe.u.help_root.short_sols;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<full_length && result==has_no_solution)
  {
    result = help_has_solution_in_n(short_sols,len);
    len += 2;
  }

  if (result==has_no_solution)
    result = help_has_solution_in_n(next,len);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a top level help branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_toplevel_help_branch(stip_length_type length,
                                              stip_length_type min_length,
                                              slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);
  assert(min_length>=slack_length_help);

  if ((length-slack_length_help)%2==0)
  {
    slice_index const fork = alloc_help_fork_slice(length-2,min_length-2,
                                                   proxy_to_goal);
    slice_index const help_root = alloc_help_root_slice(length,min_length,
                                                        proxy_to_goal,fork);
    slice_index const branch_root = alloc_branch_h_slice(length,min_length,
                                                         proxy_to_goal);
    slice_index const branch1 = alloc_branch_h_slice(length,min_length,
                                                     proxy_to_goal);
    shorten_help_pipe(branch1);

    branch_link(help_root,branch_root);
    branch_link(branch1,fork);

    if (length-slack_length_help==2)
      branch_link(branch_root,branch1);
    else
    {
      slice_index const branch2 = alloc_branch_h_slice(length-2,min_length,
                                                       proxy_to_goal);
      pipe_set_successor(branch_root,branch1);
      branch_link(fork,branch2);
      branch_link(branch2,branch1);
    }

    result = help_root;
  }
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const help_root = alloc_help_root_slice(length,min_length,
                                                        proxy_to_goal,proxy);
    slice_index const branch_root = alloc_branch_h_slice(length,min_length,
                                                         proxy_to_goal);
    slice_index const fork = alloc_help_fork_slice(length-1,min_length-1,
                                                   proxy_to_goal);
    branch_link(help_root,branch_root);

    if (length-slack_length_help==1)
      branch_link(branch_root,fork);
    else
    {
      slice_index const branch1 = alloc_branch_h_slice(length,min_length,
                                                       proxy_to_goal);
      slice_index const branch2 = alloc_branch_h_slice(length-2,min_length,
                                                       proxy_to_goal);
      shorten_help_pipe(branch1);

      pipe_set_successor(branch_root,fork);
      branch_link(fork,branch1);
      branch_link(branch1,proxy);
      branch_link(proxy,branch2);
      branch_link(branch2,fork);
    }

    result = help_root;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a nested help branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_nested_help_branch(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);

  if (length%2==0)
  {
    slice_index const fork = alloc_help_fork_slice(length-2,min_length-2,
                                                   proxy_to_goal);
    slice_index const branch = alloc_branch_h_slice(length,min_length,
                                                    proxy_to_goal);
    shorten_help_pipe(branch);

    result = alloc_branch_h_slice(length,min_length,proxy_to_goal);

    branch_link(branch,fork);
    branch_link(result,branch);
    branch_link(fork,result);
  }
  else
  {
    slice_index const fork = alloc_help_fork_slice(length-1,min_length-1,
                                                   proxy_to_goal);
    slice_index const branch = alloc_branch_h_slice(length,min_length,
                                                    proxy_to_goal);
    shorten_help_pipe(branch);

    result = alloc_branch_h_slice(length,min_length,proxy_to_goal);

    branch_link(result,fork);
    branch_link(fork,branch);
    branch_link(branch,result);
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
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of initial slice of allocated help branch
 */
slice_index alloc_help_branch(branch_level level,
                              stip_length_type length,
                              stip_length_type min_length,
                              slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(branch_level,level,"");
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);

  if (level==toplevel_branch)
    result = alloc_toplevel_help_branch(length,min_length,proxy_to_goal);
  else
    result = alloc_nested_help_branch(length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
