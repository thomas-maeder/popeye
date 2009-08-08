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
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Allocate a STBranchDirect slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param fork index of branch fork
 * @return index of allocated slice
 */
static slice_index alloc_branch_d_slice(stip_length_type length,
                                        stip_length_type min_length,
                                        slice_index defender,
                                        slice_index fork)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",defender);
  TraceFunctionParamListEnd();

  assert(length>=slack_length_direct+2);
  assert(min_length>=slack_length_direct);
  assert((length%2)==0);
  assert((min_length%2)==0);

  slices[result].type = STBranchDirect; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = defender;
  slices[result].u.pipe.u.branch_d.length = length;
  slices[result].u.pipe.u.branch_d.min_length = min_length;
  slices[result].u.pipe.u.branch_d.fork = fork;

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
 * @param next identifies next slice
 * @return index of adapter slice
 */
slice_index alloc_direct_branch(branch_level level,
                                stip_length_type length,
                                stip_length_type min_length,
                                slice_index next)
{
  slice_index defender;
  slice_index result;
  slice_index fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",level);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  fork = alloc_branch_fork_slice(no_slice,next);
  defender = alloc_branch_d_defender_slice(length-1,min_length-1,fork);
  result = alloc_branch_d_slice(length,min_length,defender,fork);
  slices[fork].u.pipe.next = result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_d_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.pipe.u.branch_d.fork);
}

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,slices[si].u.pipe.u.branch_d.fork);

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
boolean branch_d_has_starter_apriori_lost(slice_index si)
{
  return slice_has_starter_apriori_lost(slices[si].u.pipe.u.branch_d.fork);
}
 
/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_d_has_non_starter_solved(slice_index si)
{
  return slice_has_non_starter_solved(slices[si].u.pipe.u.branch_d.fork);
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_d_has_starter_won(slice_index si)
{
  return slice_has_starter_won(slices[si].u.pipe.next);
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_d_has_starter_reached_goal(slice_index si)
{
  return slice_has_starter_reached_goal(slices[si].u.pipe.u.branch_d.fork);
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * Tests do not rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_d_must_starter_resign(slice_index si)
{
  return slice_must_starter_resign(slices[si].u.pipe.u.branch_d.fork);
}

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si,
                                     stip_length_type n,
                                     int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  slice_index const peer = slices[si].u.pipe.next;
  slice_index const fork = slices[si].u.pipe.u.branch_d.fork;
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==0);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (!solution_found && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker))
    {
      if (slice_has_starter_apriori_lost(fork))
        /* nothing */;
      else if (!direct_defender_does_defender_win(peer,
                                                  n-1,
                                                  curr_max_nr_nontrivial))
      {
        solution_found = true;
        coupfort();
      }
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

/* Determine whether attacker can end short if full would be n half moves.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff attacker can end in n half moves
 */
static boolean have_we_solution_in_n_short(slice_index si,
                                           stip_length_type n,
                                           int curr_max_nr_nontrivial)
{
  boolean result = false;

  stip_length_type i;
  stip_length_type n_min = 2+slack_length_direct;
  stip_length_type n_max = n-2;
  stip_length_type const moves_played = slices[si].u.pipe.u.branch_d.length-n;
  stip_length_type const min_length = slices[si].u.pipe.u.branch_d.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (min_length>moves_played)
    n_min = min_length-moves_played;

  if (n_max>=2*max_len_threat+slack_length_direct)
    n_max = 2*max_len_threat+slack_length_direct;

  if (n_max>=2*min_length_nontrivial+slack_length_direct)
    n_max = 2*min_length_nontrivial+slack_length_direct;

  TraceValue("%u",moves_played);
  TraceValue("%u",min_length);
  TraceValue("%u",max_len_threat);
  TraceValue("%u",min_length_nontrivial);
  TraceValue("%u",n_min);
  TraceValue("%u\n",n_max);

  for (i = n_min; i<=n_max; i += 2)
    if (have_we_solution_in_n(si,i,curr_max_nr_nontrivial))
    {
      result = true;
      break;
    }
    else if (periods_counter>=nr_periods)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff attacker can end in n half moves
 */
static boolean have_we_solution_in_n_nohash(slice_index si,
                                            stip_length_type n,
                                            int curr_max_nr_nontrivial)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (have_we_solution_in_n_short(si,n,curr_max_nr_nontrivial))
    result = true;
  else if (periods_counter<nr_periods
           && have_we_solution_in_n(si,n,curr_max_nr_nontrivial))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether attacker can end in n half moves.
 * May consult the hash table.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff attacker can end in n half moves
 */
boolean branch_d_has_solution_in_n(slice_index si,
                                   stip_length_type n,
                                   int curr_max_nr_nontrivial)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

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
    result = have_we_solution_in_n_nohash(si,n,curr_max_nr_nontrivial);
    if (result)
      addtohash(si,DirSucc,n/2-1);
    else
      addtohash(si,DirNoSucc,n/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = !direct_defender_is_refuted(slices[si].u.pipe.next,
                                       slices[si].u.pipe.u.branch_d.length,
                                       max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  Side const attacker = slices[si].starter;
  slice_index const peer = slices[si].u.pipe.next;
  slice_index const fork = slices[si].u.pipe.u.branch_d.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==0);
  assert(n>slack_length_direct);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker))
    {
      if (slice_has_starter_apriori_lost(fork))
        ; /* nothing */
      else if (!direct_defender_does_defender_win(peer,n-1,max_nr_nontrivial))
      {
        write_attack(attack_regular);
        direct_defender_solve_postkey_in_n(peer,n-1);
        append_to_top_table();
        coupfort();
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  branch_d_solve_continuations_in_n(continuations,
                                    si,
                                    slices[si].u.pipe.u.branch_d.length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_d_root_write_key(slice_index si, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  write_attack(type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.pipe.u.branch_d.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch_d.min_length;
  slice_index const fork = slices[si].u.pipe.u.branch_d.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",n);

  if (slice_must_starter_resign(fork) || slice_must_starter_resign_hashed(fork))
    ;
  else if (min_length==slack_length_direct && slice_has_non_starter_solved(fork))
  {
    slice_write_non_starter_has_solved(fork);
    result = true;
  }
  else if (min_length==slack_length_direct && slice_has_solution(fork))
  {
    table const continuations = allocate_table();
    output_start_continuation_level();
    slice_solve_continuations(continuations,fork);
    output_end_continuation_level();
    free_table();
    result = true;
  }
  else if (branch_d_has_solution_in_n(si,n,max_nr_nontrivial))
  {
    stip_length_type i;
    table const continuations = allocate_table();
    stip_length_type min_len = slices[si].u.pipe.u.branch_d.min_length;

    if (min_len==slack_length_direct)
      min_len = slack_length_direct+2;

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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_root_solve(slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const peer = slices[si].u.pipe.next;
  slice_index const fork = slices[si].u.pipe.u.branch_d.fork;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.pipe.u.branch_d.length%2==0);

  init_output(si);

  if (echecc(nbply,advers(attacker)))
    ErrorMsg(KingCapture);
  else if (slice_must_starter_resign(fork)
           || slice_must_starter_resign_hashed(fork))
    slice_write_unsolvability(fork);
  else
  {
    solutions = 0;

    active_slice[nbply+1] = si;
    genmove(attacker);

    output_start_continuation_level();

    while (encore())
    {
      extern unsigned int MoveNbr; /* TODO remove */
      extern unsigned int RestartNbr; /* TODO remove */
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && !echecc(nbply,attacker)
          && !direct_defender_finish_solution_next(peer))
      {
        table refutations = allocate_table();

        unsigned int const nr_refutations =
            direct_defender_find_refutations(refutations,peer);
        if (nr_refutations<=max_nr_refutations)
        {
          write_attack(nr_refutations==0 ? attack_key : attack_try);
          direct_defender_root_solve_postkey(refutations,peer);
          write_end_of_solution();

          if (nr_refutations==0)
            result = true;
        }

        free_table();
      }

      if (OptFlag[movenbr])
        IncrementMoveNbr();

      repcoup();

      if (OptFlag[maxsols] && solutions>=maxsolutions)
      {
        TraceValue("%u",maxsolutions);
        TraceValue("%u",solutions);
        TraceText("aborting\n");

        /* signal maximal number of solutions reached to outer world */
        FlagMaxSolsReached = true;

        break;
      }

      if (periods_counter>=nr_periods)
        break;
    }

    output_end_continuation_level();

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_d_root_make_setplay_slice(slice_index si)
{
  slice_index const fork = slices[si].u.pipe.u.branch_d.fork;
  slice_index next_in_setplay;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.u.branch.length==slack_length_direct+2)
    next_in_setplay = slices[fork].u.pipe.u.branch_fork.towards_goal;
  else
  {
    slice_index const peer = branch_find_slice(STBranchDirectDefender,si);

    slice_index const next_in_setplay_peer = copy_slice(peer);
    slices[next_in_setplay_peer].u.pipe.u.branch.length -= 2;
    if (slices[peer].u.pipe.u.branch.min_length>slack_length_direct)
      slices[next_in_setplay_peer].u.pipe.u.branch.min_length -= 2;

    assert(peer!=no_slice);
    next_in_setplay = copy_slice(si);
    slices[next_in_setplay].u.pipe.u.branch.length -= 2;
    if (slices[si].u.pipe.u.branch.min_length>slack_length_direct)
      slices[next_in_setplay].u.pipe.u.branch.min_length -= 2;
    hash_slice_is_derived_from(next_in_setplay,si);

    slices[next_in_setplay].u.pipe.next = next_in_setplay_peer;
  }

  result = alloc_help_branch(toplevel_branch,
                             slack_length_help+1,slack_length_help+1,
                             next_in_setplay);
  slices[result].starter = advers(slices[si].starter);

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
who_decides_on_starter branch_d_detect_starter(slice_index si,
                                               boolean same_side_as_root)
{
  who_decides_on_starter const result = leaf_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
    slices[si].starter = White;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
