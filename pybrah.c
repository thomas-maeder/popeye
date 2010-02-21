#include "pybrah.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "pyslice.h"
#include "pyhelp.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "trace.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/shortcut.h"

#include <assert.h>

/* Allocate a STBranchHelp slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_branch_h_slice(stip_length_type length,
                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STBranchHelp,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STHelpRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies following branch silice
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return index of allocated slice
 */
slice_index alloc_help_root_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index next,
                                  slice_index short_sols)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpRoot,length,min_length);

  {
    slice_index const shortcut = alloc_help_shortcut(length,min_length,
                                                     short_sols);
    pipe_link(result,shortcut);
    pipe_link(shortcut,next);
  }

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

  --slices[pipe].u.branch.length;
  --slices[pipe].u.branch.min_length;
  if (slices[pipe].u.branch.min_length<slack_length_help)
    slices[pipe].u.branch.min_length += 2;
  slices[pipe].starter = (slices[pipe].starter==no_side
                          ? no_side
                          : advers(slices[pipe].starter));
  TraceValue("%u",slices[pipe].starter);
  TraceValue("%u",slices[pipe].u.branch.length);
  TraceValue("%u\n",slices[pipe].u.branch.min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_h_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;

    slice_index const root_branch = copy_slice(si);
    *root = alloc_help_root_slice(length,min_length,root_branch,prev);

    shorten_help_pipe(si);
    shorten_help_pipe(si);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    stip_length_type const length = slices[si].u.branch.length;

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

  if ((slices[root].u.shortcut.length-slack_length_help)%2==0)
  {
    slice_index const help_shortcut = slices[root].u.pipe.next;
    slice_index const root_branch = slices[help_shortcut].u.pipe.next;
    slice_index const proxy = slices[root_branch].u.pipe.next;
    slice_index const branch1 = slices[proxy].u.pipe.next;
    slice_index const fork = slices[branch1].u.pipe.next;
    assert(slices[help_shortcut].type==STHelpShortcut);
    assert(slices[root_branch].type==STBranchHelp);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[fork].type==STHelpFork);
    slices[root_branch].u.pipe.next = fork;
    slices[help_shortcut].u.shortcut.short_sols = branch1;
    shorten_help_pipe(root);
    shorten_help_pipe(help_shortcut);
    shorten_help_pipe(root_branch);
  }
  else
  {
    slice_index const help_shortcut = slices[root].u.pipe.next;
    slice_index const root_branch = slices[help_shortcut].u.pipe.next;
    slice_index const fork = slices[root_branch].u.pipe.next;
    slice_index const branch1 = slices[fork].u.pipe.next;
    slice_index const proxy = slices[branch1].u.pipe.next;
    assert(slices[help_shortcut].type==STHelpShortcut);
    assert(slices[root_branch].type==STBranchHelp);
    assert(slices[fork].type==STHelpFork);
    assert(slices[branch1].type==STBranchHelp);
    assert(slices[proxy].type==STProxy);
    slices[root_branch].u.pipe.next = proxy;
    slices[help_shortcut].u.shortcut.short_sols = fork;
    shorten_help_pipe(root);
    shorten_help_pipe(help_shortcut);
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

  assert(slices[si].u.shortcut.length>slack_length_help);

  prod->sibling = si;

  if (slices[si].u.shortcut.length==slack_length_help+1)
    pipe_traverse_next(si,st);
  else
  {
    slice_index const shortcut = slices[si].u.pipe.next;
    slice_index const root_branch = slices[shortcut].u.pipe.next;

    slice_index const shortcut_copy = copy_slice(shortcut);
    slice_index const root_branch_copy = copy_slice(root_branch);

    prod->setplay_slice = copy_slice(si);
    pipe_link(shortcut_copy,root_branch_copy);
    pipe_link(prod->setplay_slice,shortcut_copy);

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
  slice_index const help_shortcut = slices[root].u.pipe.next;
  slice_index const root_branch = slices[help_shortcut].u.pipe.next;
  slice_index const branch = slices[root_branch].u.pipe.next;
  slice_index const fork = slices[branch].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[help_shortcut].type==STHelpShortcut);
  assert(slices[root_branch].type==STBranchHelp);
  assert(slices[branch].type==STBranchHelp);
  assert(slices[fork].type==STHelpFork);

  if (slices[root].u.shortcut.length-slack_length_help>2)
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const branch2 = slices[fork].u.pipe.next;

    assert(slices[branch2].type==STBranchHelp);

    slices[branch].u.shortcut.length -= 2;
    slices[fork].u.shortcut.length -= 2;
    slices[help_shortcut].u.shortcut.short_sols = proxy;

    pipe_link(proxy,branch2);
  }

  pipe_set_successor(root_branch,fork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Perform the necessary work for shorting a branch from an odd to an
 * even length
 * @param root STHelpRoot slice of branch to be shortened
 */
static void shorten_root_branch_odd_to_even(slice_index root)
{
  slice_index const help_shortcut = slices[root].u.pipe.next;
  slice_index const root_branch = slices[help_shortcut].u.pipe.next;
  slice_index const fork = slices[root_branch].u.pipe.next;
  slice_index const branch = slices[fork].u.pipe.next;
  slice_index const proxy = slices[branch].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[help_shortcut].type==STHelpShortcut);
  assert(slices[root_branch].type==STBranchHelp);
  assert(slices[fork].type==STHelpFork);
  assert(slices[branch].type==STBranchHelp);
  assert(slices[proxy].type==STProxy);

  slices[help_shortcut].u.shortcut.short_sols = fork;

  if (slices[root].u.shortcut.length-slack_length_help==3)
    pipe_set_successor(root_branch,proxy);
  else
  {
    pipe_set_successor(root_branch,proxy);
    slices[branch].u.shortcut.length -= 2;
  }

  slices[fork].u.shortcut.length -= 2;

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
  slice_index const help_shortcut = slices[root].u.pipe.next;
  slice_index const root_branch = slices[help_shortcut].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[help_shortcut].type==STHelpShortcut);
  assert(slices[root_branch].type==STBranchHelp);

  if ((slices[root].u.shortcut.length-slack_length_help)%2==0)
    shorten_root_branch_even_to_odd(root);
  else
    shorten_root_branch_odd_to_even(root);

  shorten_help_pipe(root);
  shorten_help_pipe(help_shortcut);
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
  assert(slices[root].u.shortcut.length>slack_length_help);

  if (slices[root].u.shortcut.length==slack_length_help+1)
  {
    slice_index const proxy_to_goal = branch_deallocate_to_fork(root);
    assert(slices[proxy_to_goal].type==STProxy);
    result = slices[proxy_to_goal].u.pipe.next;
    dealloc_proxy_slice(proxy_to_goal);
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
  slice_index const next = slices[root].u.pipe.next;
  stip_length_type const full_length = slices[root].u.shortcut.length;
  stip_length_type len = slices[root].u.shortcut.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  init_output(root);

  move_generation_mode = move_generation_not_optimized;

  while (len<=full_length)
  {
    if (help_solve_in_n(next,len))
      result = true;
    len += 2;
  }

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
  stip_length_type const full_length = slices[si].u.shortcut.length;
  stip_length_type len = slices[si].u.shortcut.min_length;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<=full_length && result==has_no_solution)
  {
    result = help_has_solution_in_n(next,len);
    len += 2;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies slice leading towards goal
 * @return index of initial slice of allocated help branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length,
                              slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_help);

  if (slices[to_goal].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,to_goal);
    to_goal = proxy;
  }

  if ((length-slack_length_help)%2==0)
  {
    slice_index const branch1 = alloc_branch_h_slice(length,min_length);
    slice_index const branch2 = alloc_branch_h_slice(length,min_length);
    result = alloc_help_fork_slice(length,min_length,to_goal);

    shorten_help_pipe(branch2);

    pipe_link(result,branch1);
    pipe_link(branch1,branch2);
    pipe_link(branch2,result);
  }
  else
  {
    slice_index const fork = alloc_help_fork_slice(length,min_length,to_goal);
    slice_index const branch1 = alloc_branch_h_slice(length,min_length);
    slice_index const branch2 = alloc_branch_h_slice(length,min_length);
    result = alloc_proxy_slice();

    shorten_help_pipe(fork);
    shorten_help_pipe(branch1);

    pipe_link(result,branch2);
    pipe_link(branch2,fork);
    pipe_link(fork,branch1);
    pipe_link(branch1,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
