#include "pybrad.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybranch.h"
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
  slices[result].u.branch.starter = no_side; 
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;
  slices[result].u.branch.next = next;

  {
    slice_index const defender_slice = copy_slice(result);
    slices[defender_slice].type = STBranchDirectDefender;
    --slices[defender_slice].u.branch.length;
    --slices[defender_slice].u.branch.min_length;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Count all non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * @return number of defender's non-trivial moves minus 1 (TODO: why?)
 */
static int count_non_trivial_defenses(slice_index si)
{
  Side const attacker = slices[si].u.branch.starter;
  Side const defender = advers(attacker);
  int result = -1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  genmove(defender);

  TraceValue("%u",nbcou);
  TraceValue("%u",nbply);
  TraceValue("%u",repere[nbply]);
  TraceValue("%u",max_nr_nontrivial);
  TraceValue("%d\n",result);
  
  while (encore() && max_nr_nontrivial>=result)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender))
    {
      if (min_length_nontrivial==0)
        ++result;
      else
      {
        (*encode)();
        if (branch_d_has_solution_in_n(si,
                                       2*min_length_nontrivial
                                       +slack_length_direct)
            ==branch_d_no_solution)
          ++result;
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the threat after the attacker's move just played
 * is too long respective to user input.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return true iff threat is too long
 */
static boolean is_threat_too_long(slice_index si, stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  Side const defender = advers(attacker);
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  TraceValue("%u\n",2*max_len_threat);
  if (n>2*max_len_threat+slack_length_direct
      && !echecc(nbply,defender))
  {
    (*encode)();
    result = (branch_d_has_solution_in_n(si,2*max_len_threat)
              ==branch_d_no_solution);
  }
  else
    /* remainder of play is too short for max_len_threat to apply */
    result = false;

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
  Side const attacker = slices[si].u.branch.starter;
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
        && branch_d_defender_does_defender_win(si+1,n-1)>=loss)
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
    stip_length_type const moves_played = slices[si].u.branch.length-n;
    stip_length_type const min_length = slices[si].u.branch.min_length;

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  if (slice_must_starter_resign(si))
    ; /* intentionally nothing */
  else
  {
    stip_length_type const moves_played = slices[si].u.branch.length-n;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    TraceValue("%u",moves_played);
    TraceValue("%u\n",min_length);
    if (moves_played+slack_length_direct>min_length
        && slice_has_non_starter_solved(slices[si].u.branch.next))
      result = branch_d_already_solved;
    else if (moves_played+slack_length_direct>=min_length
             && slice_has_solution(slices[si].u.branch.next))
      result = branch_d_next_solves;
    else if (n>slack_length_direct
             && have_we_solution_in_n_hashed(si,n))
      result = branch_d_we_solve;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Collect refutations at root level
 * @param t table where to add refutations
 * @param si slice index
 * @param (odd) number of half moves until goal
 * @return true iff defender is immobile
 */
static boolean root_collect_refutations(table refutations,
                                        slice_index si,
                                        stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  Side const defender = advers(attacker);
  boolean is_defender_immobile = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  if (n-1>slack_length_direct+2)
    move_generation_mode= move_generation_mode_opti_per_side[defender];
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore()
         && table_length(refutations)<=max_nr_refutations)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender))
    {
      is_defender_immobile = false;
      (*encode)();
      if (branch_d_has_solution_in_n(si,n-1)==branch_d_no_solution)
      {
        append_to_top_table();
        coupfort();
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",is_defender_immobile);
  return is_defender_immobile;
}

/* Collect non-trivial defenses at root level
 * @param non_trivial table where to add non-trivial defenses
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return max_nr_refutations+1 if defender is immobile or there are
 *                              too many non-trivial defenses respective
 *                              to user input
 *         number of non-trivial defenses otherwise
 */
static unsigned int root_collect_non_trivial(table non_trivial,
                                             slice_index si,
                                             stip_length_type n)
{
  unsigned int result;
  int non_trivial_count;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  non_trivial_count = count_non_trivial_defenses(si);
  if (max_nr_nontrivial<non_trivial_count)
    result = max_nr_refutations+1;
  else
  {
    max_nr_nontrivial -= non_trivial_count;
    result = (root_collect_refutations(non_trivial,si,n)
              ? max_nr_refutations+1
              : table_length(non_trivial));
    max_nr_nontrivial += non_trivial_count;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find refutations after a move of the attacking side at root level.
 * @param t table where to store refutations
 * @param si slice index
 * @return max_nr_refutations+1 if
 *            if the defending side is immobile (it shouldn't be here!)
 *            if the defending side has more non-trivial moves than allowed
 *            if the defending king has more flights than allowed
 *            if there is no threat in <= the maximal threat length
 *               as entered by the user
 *         number (0..max_nr_refutations) of refutations otherwise
 */
static unsigned int root_find_refutations(table refutations, slice_index si)
{
  Side const defender = advers(slices[si].u.branch.starter);
  unsigned int result;
  stip_length_type const n = slices[si].u.branch.length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (is_threat_too_long(si,n-1))
    result = max_nr_refutations+1;
  else if (n-1>slack_length_direct+2
           && OptFlag[solflights] && has_too_many_flights(defender))
    result = max_nr_refutations+1;
  else if (n-1>2*min_length_nontrivial+slack_length_direct)
    result = root_collect_non_trivial(refutations,si,n);
  else
    result = (root_collect_refutations(refutations,si,n)
              ? max_nr_refutations+1
              : table_length(refutations));

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
  Side const attacker = slices[si].u.branch.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  if (n==slack_length_direct)
    slice_solve_continuations(continuations,slices[si].u.branch.next);
  else
  {
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !echecc(nbply,attacker))
      {
        d_defender_win_type const
            defender_success = branch_d_defender_does_defender_win(si+1,n-1);
        TraceValue("%u\n",defender_success);
        if (defender_success>=loss)
        {
          write_attack(attack_regular);

          if (defender_success==already_lost)
            slice_solve_postkey(slices[si].u.branch.next);
          else
            branch_d_defender_solve_postkey_in_n(si+1,n-1);

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
                                    slices[si].u.branch.length);

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

/* Solve the solutions and tries
 * @param si slice index
 */
static void root_solve_real_play(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (slice_must_starter_resign(si))
    slice_write_unsolvability(slices[si].u.branch.next);
  else if (slices[si].u.branch.length==slack_length_direct)
    slice_root_solve(slices[si].u.branch.next);
  else
  {
    Side const attacker = slices[si].u.branch.starter;
    genmove(attacker);

    output_start_continuation_level();

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && !echecc(nbply,attacker))
      {
        table refutations = allocate_table();

        if (slices[si].u.branch.min_length<=slack_length_direct
            && slice_has_starter_reached_goal(slices[si].u.branch.next))
        {
          slice_root_write_key(slices[si].u.branch.next,attack_key);
          slice_root_solve_postkey(refutations,slices[si].u.branch.next);
          write_end_of_solution();
        }
        else
        {
          unsigned int const nr_refutations =
              root_find_refutations(refutations,si);
          TraceValue("%u",nr_refutations);
          TraceValue("%u\n",max_nr_refutations);
          if (nr_refutations<=max_nr_refutations)
          {
            attack_type const type = (nr_refutations==0
                                      ? attack_key
                                      : attack_try);
            write_attack(type);
            branch_d_defender_root_solve(refutations,si+1);
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

/* Solve at non-root level.
 * @param si slice index
 */
boolean branch_d_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.branch.length;
  branch_d_solution_degree how_is_solved;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",n);

  how_is_solved = branch_d_has_solution_in_n(si,n);
  TraceValue("%u\n",how_is_solved);

  if (how_is_solved==branch_d_already_solved)
    slice_write_non_starter_has_solved(slices[si].u.branch.next);
  else if (how_is_solved<=branch_d_we_solve)
  {
    stip_length_type i;
    table const continuations = allocate_table();
    stip_length_type min_len = slices[si].u.branch.min_length;

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
  stip_length_type const n = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (n%2==1)
  {
    if (echecc(nbply,slices[si].u.branch.starter))
      ErrorMsg(SetAndCheck);
    else
    {
      output_start_continuation_level();
      branch_d_defender_solve_postkey_in_n(si+1,n);
      output_end_continuation_level();
    }
  }
  else
  {
    if (echecc(nbply,advers(slices[si].u.branch.starter)))
      ErrorMsg(KingCapture);
    else
      root_solve_real_play(si);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Spin off a set play slice
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_d_root_make_setplay_slice(slice_index si)
{
  slice_index const next = slices[si].u.branch.next;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (slices[si].u.branch.length%2==1)
    result = no_slice;
  else if (slices[si].u.branch.length==slack_length_direct)
    result = slice_root_make_setplay_slice(next);
  else
  {
    slice_index next_in_setplay;
    if (slices[si].u.branch.length==slack_length_direct+2)
      next_in_setplay = next;
    else
    {
      next_in_setplay = copy_slice(si);
      slices[next_in_setplay].u.branch.length -= 2;
      slices[next_in_setplay].u.branch.min_length -= 2;
      hash_slice_is_derived_from(next_in_setplay,si);
      copy_slice(next_in_setplay);
    }

    result = alloc_branch_h_slice(slack_length_help+1,
                                  slack_length_help+1,
                                  next_in_setplay);
    slices[result].u.branch.starter = advers(slices[si].u.branch.starter);
  }

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
  who_decides_on_starter result = dont_know_who_decides_on_starter;
  slice_index const next = slices[si].u.branch.next;
  slice_index next_relevant = next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",same_side_as_root);
  
  if (slices[next].type==STMoveInverter)
    next_relevant = slices[next].u.move_inverter.next;

  TraceValue("%u\n",next_relevant);

  result = slice_detect_starter(next,same_side_as_root);
  if (slice_get_starter(next)==no_side)
  {
    /* next can't tell - let's tell him */
    switch (slices[next_relevant].type)
    {
      case STLeafDirect:
        slices[si].u.branch.starter =  White;
        TraceValue("%u\n",slices[si].u.branch.starter);
        slice_impose_starter(next,slices[si].u.branch.starter);
        break;

      case STLeafSelf:
        slices[si].u.branch.starter = White;
        TraceValue("%u\n",slices[si].u.branch.starter);
        slice_impose_starter(next,slices[si].u.branch.starter);
        break;

      case STLeafHelp:
        slices[si].u.branch.starter = White;
        TraceValue("%u\n",slices[si].u.branch.starter);
        slice_impose_starter(next,advers(slices[si].u.branch.starter));
        break;

      default:
        slices[si].u.branch.starter = no_side;
        break;
    }
  }
  else
    slices[si].u.branch.starter = slice_get_starter(next);

  slices[si+1].u.branch.starter = slices[si].u.branch.starter;

  TraceValue("%u\n",slices[si].u.branch.starter);

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
  slices[si].u.branch.starter = s;
  slices[si+1].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,s);
}
