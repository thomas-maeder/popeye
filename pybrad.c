#include "pybrad.h"
#include "pybradd.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "pyslice.h"
#include "pytable.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Allocate a STBranchDirect slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_d_slice(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u\n",next);

  slices[result].type = STBranchDirect; 
  slices[result].u.branch_d.starter = no_side; 
  slices[result].u.branch_d.length = length;
  slices[result].u.branch_d.min_length = min_length;
  slices[result].u.branch_d.next = next;

  slices[result].u.branch_d.peer = copy_slice(result);
  slices[slices[result].u.branch_d.peer].u.branch_d.peer = result;

  slices[slices[result].u.branch_d.peer].type = STBranchDirectDefender;
  --slices[slices[result].u.branch_d.peer].u.branch_d.length;
  --slices[slices[result].u.branch_d.peer].u.branch_d.min_length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_d_write_unsolvability(slice_index si)
{
  branch_d_defender_write_unsolvability(slices[si].u.branch_d.peer);
}

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;
  slice_index const peer = slices[si].u.branch_d.peer;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = branch_d_defender_is_goal_reached(just_moved,peer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n (even) number of half moves until goal
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si, stip_length_type n)
{
  Side const attacker = slices[si].u.branch_d.starter;
  slice_index const peer = slices[si].u.branch_d.peer;
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  genmove(attacker);

  while (!solution_found && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker)
        && branch_d_defender_does_defender_win(peer,n-1)>=loss)
    {
      solution_found = true;
      coupfort();
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
  TraceValue("%u",n);
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
}

/* Determine whether this slice has a solution in n half moves
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return true iff this slice has a solution
 */
static boolean have_we_solution_in_n_hashed(slice_index si,
                                            stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  /* It is more likely that a position has no solution. */
  /* Therefore let's check for "no solution" first.  TLi */
  if (inhash(si,DirNoSucc,n/2))
  {
    TraceText("inhash(si,DirNoSucc,n/2)\n");
    assert(!inhash(si,DirSucc,n/2-1));
  }
  else if (inhash(si,DirSucc,n/2-1))
  {
    TraceText("inhash(si,DirSucc,n/2-1)\n");
    result = true;
  }
  else
  {
    stip_length_type i;
    stip_length_type n_min = 2+slack_length_direct;
    stip_length_type const moves_played = slices[si].u.branch_d.length-n;
    stip_length_type const min_length = slices[si].u.branch_d.min_length;

    if (min_length>moves_played)
      n_min = min_length-moves_played;

    for (i = n_min; i<=n; i += 2)
    {
      if (i-2>2*max_len_threat+slack_length_direct
          || i>2*min_length_nontrivial+slack_length_direct)
        i = n;

      if (have_we_solution_in_n(si,i))
      {
        result = true;
        break;
      }
      else if (maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    if (result)
      addtohash(si,DirSucc,n/2-1);
    else
      addtohash(si,DirNoSucc,n/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether attacker can end in n half moves of direct play.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return true iff attacker can end in n half moves
 */
branch_d_solution_degree branch_d_has_solution_in_n(slice_index si,
                                                    stip_length_type n)
{
  branch_d_solution_degree result = branch_d_no_solution;
  slice_index const peer = slices[si].u.branch_d.peer;
  stip_length_type const moves_played = slices[si].u.branch_d.length-n;
  stip_length_type const min_length = slices[si].u.branch_d.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  TraceValue("%u",moves_played);
  TraceValue("%u\n",min_length);
  if (moves_played+slack_length_direct>min_length
      && branch_d_defender_has_non_starter_solved(peer))
    result = branch_d_already_solved;
  else if (moves_played+slack_length_direct>=min_length
           && slice_has_solution(slices[si].u.branch_d.next))
    result = branch_d_next_solves;
  else if (n>slack_length_direct
           && have_we_solution_in_n_hashed(si,n))
    result = branch_d_we_solve;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_d_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = !branch_d_defender_is_refuted(slices[si].u.branch_d.peer,
                                         slices[si].u.branch_d.length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param attacker attacking side
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 * @param n (even) number of half moves until goal
 */
void branch_d_solve_continuations_in_n(table continuations,
                                       slice_index si,
                                       stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  if (n==slack_length_direct)
    slice_solve_continuations(continuations,slices[si].u.branch_d.next);
  else
  {
    Side const attacker = slices[si].u.branch_d.starter;
    slice_index const peer = slices[si].u.branch_d.peer;

    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !echecc(nbply,attacker))
      {
        d_defender_win_type const
            defender_success = branch_d_defender_does_defender_win(peer,n-1);
        TraceValue("%u\n",defender_success);
        if (defender_success>=loss)
        {
          write_attack(attack_regular);

          if (defender_success==already_lost)
            slice_solve_postkey(slices[si].u.branch_d.next);
          else
            branch_d_defender_solve_postkey_in_n(peer,n-1);

          append_to_top_table();
          coupfort();
        }
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
void branch_d_solve_continuations(table continuations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  branch_d_solve_continuations_in_n(continuations,
                                    si,
                                    slices[si].u.branch_d.length);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_d_root_write_key(slice_index si, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",type);

  write_attack(type);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find solutions in next slice
 * @param si slice index
 * @return true iff >=1 solution has been found
 */
static boolean solve_next(slice_index si)
{
  slice_index const peer = slices[si].u.branch_d.peer;
  stip_length_type const n = slices[si].u.branch_d.length;
  stip_length_type const min_length = slices[si].u.branch_d.min_length;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(n%2==0);

  TraceValue("%u\n",min_length);
  if (min_length<slack_length_direct
      && branch_d_defender_has_non_starter_solved(peer))
  {
    slice_write_non_starter_has_solved(slices[si].u.branch_d.next);
    result = true;
  }
  else if (min_length<=slack_length_direct
           && slice_has_solution(slices[si].u.branch_d.next))
  {
    table const continuations = allocate_table();
    output_start_continuation_level();
    slice_solve_continuations(continuations,slices[si].u.branch_d.next);
    output_end_continuation_level();
    free_table();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve at non-root level.
 * @param si slice index
 */
boolean branch_d_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.branch_d.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",n);

  if (branch_d_defender_must_starter_resign(slices[si].u.branch_d.peer))
    ;
  else if (solve_next(si))
    result = true;
  else if (n>slack_length_direct
           && have_we_solution_in_n_hashed(si,n))
  {
    /* TODO does have_we_solution_in_n_hashed 'know' how many
     * moves are needed? */
    stip_length_type i;
    table const continuations = allocate_table();
    stip_length_type min_len = slices[si].u.branch_d.min_length;

    if (min_len<slack_length_direct)
      min_len = slack_length_direct;

    output_start_continuation_level();
  
    for (i = min_len; i<=n && !result; i += 2)
    {
      branch_d_solve_continuations_in_n(continuations,si,i);
      result = table_length(continuations)>0;
    }

    output_end_continuation_level();

    free_table();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve at root level
 * @param si slice index
 */
void branch_d_root_solve(slice_index si)
{
  Side const attacker = slices[si].u.branch_d.starter;
  slice_index const peer = slices[si].u.branch_d.peer;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(slices[si].u.branch_d.length%2==0);
  assert(slices[si].u.branch_d.length>slack_length_direct);

  if (echecc(nbply,advers(attacker)))
    ErrorMsg(KingCapture);
  else if (branch_d_defender_must_starter_resign(peer))
    branch_d_defender_write_unsolvability(peer);
  else
  {
    genmove(attacker);

    output_start_continuation_level();

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && !echecc(nbply,attacker))
      {
        table refutations = allocate_table();

        if (slices[si].u.branch_d.min_length<=slack_length_direct
            && branch_d_defender_has_starter_reached_goal(peer))
        {
          slice_index const next = slices[si].u.branch_d.next;
          slice_root_write_key(next,attack_key);
          slice_root_solve_postkey(refutations,next);
          write_end_of_solution();
        }
        else
        {
          unsigned int const nr_refutations =
              branch_d_defender_find_refutations(refutations,peer);
          if (nr_refutations<=max_nr_refutations)
          {
            write_attack(nr_refutations==0 ? attack_key : attack_try);
            branch_d_defender_root_solve_postkey(refutations,peer);
            write_end_of_solution();
          }
        }

        free_table();
      }

      if (OptFlag[movenbr])
        IncrementMoveNbr();

      repcoup();

      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    output_end_continuation_level();

    finply();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_d_root_make_setplay_slice(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(slices[si].u.branch_d.length%2==0);
  assert(slices[si].u.branch_d.length>slack_length_direct);

  result = branch_d_defender_make_setplay_slice(slices[si].u.branch_d.peer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_d_detect_starter(slice_index si,
                                               boolean same_side_as_root)
{
  who_decides_on_starter result;
  slice_index const peer = slices[si].u.branch_d.peer;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",same_side_as_root);

  result = branch_d_defender_detect_starter(peer,same_side_as_root);
  slices[si].u.branch_d.starter = slice_get_starter(peer);

  TraceValue("%u\n",slices[si].u.branch_d.starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_d_impose_starter(slice_index si, Side s)
{
  slices[si].u.branch_d.starter = s;
  branch_d_defender_impose_starter(slices[si].u.branch_d.peer,s);
}
