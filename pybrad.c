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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Set the peer slice of a STBranchDirect slice
 * @param si index of the STBranchDirect slice
 * @param slice index of the new peer
 */
void branch_d_set_peer(slice_index si, slice_index peer)
{
  slices[si].u.branch_d.peer = peer;
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
        && !echecc(nbply,attacker))
    {
      if (branch_d_defender_has_starter_apriori_lost(peer))
        /* nothing */;
      else if ((slices[si].u.branch_d.length-n
                >slices[si].u.branch_d.min_length
                && branch_d_defender_has_starter_reached_goal(peer))
               || !branch_d_defender_does_defender_win(peer,n-1))
      {
        solution_found = true;
        coupfort();
      }
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

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return true iff attacker can end in n half moves
 */
boolean branch_d_has_solution_in_n(slice_index si, stip_length_type n)
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

boolean is_threat_too_long(slice_index si, stip_length_type n);
boolean too_many_non_trivial_defenses(slice_index si,
                                      stip_length_type n);

typedef enum
{
  defender_is_immobile,
  defender_has_refutation,
  defender_has_no_refutation
} defender_has_refutation_type;
defender_has_refutation_type has_defender_refutation(slice_index si,
                                                     stip_length_type n);

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
  Side const attacker = slices[si].u.branch_d.starter;
  slice_index const peer = slices[si].u.branch_d.peer;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);
  assert(n>slack_length_direct);

  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker))
    {
      if (branch_d_defender_has_starter_apriori_lost(peer))
        ; /* nothing */
      else if (slices[si].u.branch_d.length-n
               >slices[si].u.branch_d.min_length
               && branch_d_defender_has_starter_reached_goal(peer))
      {
        write_attack(attack_regular);
        branch_d_defender_write_solution_next(peer);
        append_to_top_table();
        coupfort();
      }
      else if (!branch_d_defender_does_defender_win(peer,n-1))
      {
        write_attack(attack_regular);
        branch_d_defender_solve_postkey_in_n(peer,n-1);
        append_to_top_table();
        coupfort();
      }
    }

    repcoup();
  }

  finply();

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
  else if (branch_d_defender_solve_next(slices[si].u.branch_d.peer))
    result = true;
  else if (n>slack_length_direct
           && branch_d_has_solution_in_n(si,n))
  {
    /* TODO does branch_d_has_solution_in_n 'know' how many
     * moves are needed? */
    stip_length_type i;
    table const continuations = allocate_table();
    stip_length_type min_len = slices[si].u.branch_d.min_length;

    if (min_len<=slack_length_direct)
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
          && !echecc(nbply,attacker)
          && !branch_d_defender_finish_solution_next(peer))
      {
        table refutations = allocate_table();

        unsigned int const nr_refutations =
            branch_d_defender_find_refutations(refutations,peer);
        if (nr_refutations<=max_nr_refutations)
        {
          write_attack(nr_refutations==0 ? attack_key : attack_try);
          branch_d_defender_root_solve_postkey(refutations,peer);
          write_end_of_solution();
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
