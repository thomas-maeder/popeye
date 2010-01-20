#include "pybrad.h"
#include "pydirect.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybrafrk.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "pyslice.h"
#include "pytable.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "platform/maxtime.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBranchDirect slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_branch_d_slice(stip_length_type length,
                                        stip_length_type min_length,
                                        slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_direct);
  assert(min_length>=slack_length_direct-1);
  assert((length%2)==(min_length%2));

  result = alloc_branch(STBranchDirect,length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STDirectRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_direct_root_branch(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(length>=slack_length_direct+1);
  assert(min_length>=slack_length_direct);
  assert((length%2)==(min_length%2));

  result = alloc_branch(STDirectRoot,length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a nested (i.e. non toplevel) direct branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return identifier for entry slice of allocated branch
 */
static slice_index alloc_nested_direct_branch(stip_length_type length,
                                              stip_length_type min_length,
                                              slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);
  assert(min_length>=slack_length_direct);
  assert(min_length%2==length%2);


  result = alloc_branch_d_slice(length,min_length,proxy_to_goal);

  if (length-slack_length_direct>1)
  {
    stip_length_type const defender_min_length = (min_length>slack_length_direct
                                                  ? min_length-1
                                                  : min_length+1);
    slice_index const def = alloc_branch_d_defender_slice(length-1,
                                                          defender_min_length,
                                                          proxy_to_goal);
    branch_link(result,def);
    branch_link(def,result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a toplevel battle play branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of STDirectRoot slice of allocated branch
 */
static slice_index alloc_toplevel_direct_branch(stip_length_type length,
                                                stip_length_type min_length,
                                                slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_direct);
  assert(min_length>=slack_length_direct);

  result = alloc_direct_root_branch(length,min_length,proxy_to_goal);

  if (length-slack_length_direct>=2)
  {
    stip_length_type const parity = (length-slack_length_direct)%2;
    stip_length_type defender_root_min_length = (min_length-slack_length_direct<1
                                                 ? slack_length_direct+parity-1
                                                 : min_length-1);
    slice_index const defender_root
        = alloc_branch_d_defender_root_slice(length-1,defender_root_min_length,
                                             proxy_to_goal);
    branch_link(result,defender_root);

    if (length-slack_length_direct>=3)
    {
      stip_length_type branch_min_length = (min_length-slack_length_direct<2
                                            ? slack_length_direct+parity
                                            : min_length-2);
      slice_index const nested
          = alloc_nested_direct_branch(length-2,branch_min_length,proxy_to_goal);
      if (length-slack_length_direct==3)
        branch_link(defender_root,nested);
      else
        pipe_set_successor(defender_root,nested);
    }
  }
     
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch that represents direct play
 * @param level is this a top-level branch or one nested into another
 *              branch?
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of entry slice to allocated branch
 */
slice_index alloc_direct_branch(branch_level level,
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
  assert(length>slack_length_direct);
  assert(slices[proxy_to_goal].type==STProxy);

  if (level==toplevel_branch)
    result = alloc_toplevel_direct_branch(length,min_length,proxy_to_goal);
  else
    result = alloc_nested_direct_branch(length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal (after the defense)
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_d_are_threats_refuted_in_n(table threats,
                                          stip_length_type len_threat,
                                          slice_index si,
                                          stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int const nr_refutations_allowed = 0;
  boolean result = true;
  unsigned int nr_successful_threats = 0;
  boolean defense_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore() && !defense_found)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && is_current_move_in_table(threats))
    {
      if (direct_defender_can_defend_in_n(next,
                                          len_threat-1,
                                          nr_refutations_allowed)
          >nr_refutations_allowed)
        defense_found = true;
      else
        ++nr_successful_threats;
    }

    repcoup();
  }

  finply();

  /* this happens if >=1 threat no longer works or some threats can
   * no longer be played after the defense.
   */
  result = nr_successful_threats<table_length(threats);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n maximum number of half moves until goal
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si, stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean solution_found = false;
  unsigned int const nr_refutations_allowed = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (!solution_found && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && (direct_defender_can_defend_in_n(next,n-1,nr_refutations_allowed)
            <=nr_refutations_allowed))
    {
      solution_found = true;
      coupfort();
    }

    repcoup();

    if (periods_counter>=nr_periods)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
  TraceValue("%u",n);
  TraceFunctionResult("%u",solution_found);
  TraceFunctionResultEnd();
  return solution_found;
}

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type branch_d_has_solution_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();
  
  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n_min<=slack_length_direct)
    n_min += 2;

  for (result = n_min; result<=n; result += 2)
    if (have_we_solution_in_n(si,result))
      break;
    else if (periods_counter>=nr_periods)
    {
      result = n+2;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void branch_d_solve_continuations_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_min)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = branch_d_solve_in_n(si,n,n_min);
  assert(result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve short threats (i.e. threats where the play immediately goes
 * on with the next branch/leaf)
 * @param si slice index
 * @return true iff >=1 threat was found
 */
static boolean solve_threats_short(slice_index si)
{
  boolean result = false;
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_defend_in_n(next,slack_length_direct))
    {
      append_to_top_table();
      coupfort();
      result = true;
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve long threats (i.e. threats where the play goes on in the
 * current branch)
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 threat was found
 */
static boolean solve_threats_long_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_defend_in_n(next,n-1))
    {
      result = true;
      append_to_top_table();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find long threats (i.e. solutions where the play goes on in the
 * current branch); solutions are looked for in increasing number of
 * half-moves from n_min to n.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 */
static stip_length_type solve_threats_long(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min)
{
  stip_length_type result = n_min;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  while (result<=n)
    if (solve_threats_long_in_n(si,result))
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type branch_d_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n_min<=slack_length_direct)
    n_min += 2;

  if (n_min==slack_length_direct+1)
  {
    if (solve_threats_short(si))
      result = slack_length_direct+1;
    else
      result = solve_threats_long(si,n,slack_length_direct+3);
  }
  else
    result = solve_threats_long(si,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/**************** slice interface ***********************/

/* Find short solutions (i.e. solutions where the play immediately
 * goes on in the next branch/leaf)
 * @param si slice index
 */
static boolean solve_short(slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_defend_in_n(next,slack_length_direct))
    {
      result = true;
      coupfort();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find long solutions (i.e. solutions where the play goes on in the
 * current branch)
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 solution in n half-moves was found
 */
static boolean solve_long_in_n(slice_index si, stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_defend_in_n(next,n-1))
      result = true;

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Find long solutions (i.e. solutions where the play goes on in the
 * current branch); solutions are looked for in increasing number of
 * half-moves from n_min to n.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 */
static stip_length_type solve_long(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min)
{
  stip_length_type result = n_min;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  while (result<=n)
    if (solve_long_in_n(si,result))
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type branch_d_solve_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  output_start_continuation_level();

  if (n_min<=slack_length_direct)
    n_min += 2;

  if (n_min==slack_length_direct+1)
  {
    if (solve_short(si))
      result = slack_length_direct+1;
    else
      result = solve_long(si,n,slack_length_direct+3);
  }
  else
    result = solve_long(si,n,n_min);

  output_end_continuation_level();

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
boolean branch_d_detect_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
    slices[si].starter = White;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/****************** root ************************/

/* Solve at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_root_root_solve(slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  solutions = 0;

  active_slice[nbply+1] = si;
  genmove(attacker);

  output_start_continuation_level();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_root_defend(next))
      result = true;

    repcoup();

    if (OptFlag[maxsols] && solutions>=maxsolutions)
    {
      /* signal maximal number of solutions reached to outer world */
      FlagMaxSolsReached = true;
      break;
    }

    if (periods_counter>=nr_periods)
      break;
  }

  output_end_continuation_level();

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean direct_root_reduce_to_postkey_play(slice_index si,
                                           struct slice_traversal *st)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(next,st);

  if (*postkey_slice!=no_slice)
    dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
