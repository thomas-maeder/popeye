#include "pybradd.h"
#include "pybrad.h"
#include "pybrah.h"
#include "pydata.h"
#include "pyslice.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBranchDirect defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_d_defender_slice(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STBranchDirectDefender; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.u.branch_d_defender.length = length-1;
  if (min_length==0)
    slices[result].u.pipe.u.branch_d_defender.min_length = 1;
  else
    slices[result].u.pipe.u.branch_d_defender.min_length = min_length-1;
  slices[result].u.pipe.next = no_slice;
  slices[result].u.pipe.u.branch_d_defender.towards_goal = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Set the peer slice of a STBranchDirect defender slice
 * @param si index of the STBranchDirect defender slice
 * @param slice index of the new peer
 */
void branch_d_defender_set_peer(slice_index si, slice_index peer)
{
  slices[si].u.pipe.next = peer;
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_d_defender_must_starter_resign(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_must_starter_resign(next)
            || slice_must_starter_resign_hashed(next,defender));
  
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
void branch_d_defender_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.pipe.u.branch_d_defender.towards_goal);
}

/* Let the next slice write the solution starting with the key just played
 * @param si slice index
 */
void branch_d_defender_write_solution_next(slice_index si)
{
  slice_solve_postkey(slices[si].u.pipe.u.branch_d_defender.towards_goal);
}

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_defender_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result =  slice_is_goal_reached(just_moved,
                                  slices[si].u.pipe.u.branch_d_defender.towards_goal);

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
boolean branch_d_defender_has_starter_apriori_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_apriori_lost(slices[si].u.pipe.u.branch_d_defender.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Is the defense just played a refutation?
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense is a refutation
 */
boolean branch_d_defender_is_refuted(slice_index si,
                                     stip_length_type n,
                                     int curr_max_nr_nontrivial)
{
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==0);

  if (slice_must_starter_resign(next)
      || slice_must_starter_resign_hashed(next,defender))
    result = true;
  else
  {
    stip_length_type const
        moves_played = slices[si].u.pipe.u.branch_d_defender.length-n;
    stip_length_type const
        min_length = slices[si].u.pipe.u.branch_d_defender.min_length;
    if (moves_played+slack_length_direct>min_length
        && slice_has_non_starter_solved(next))
      result = false;
    else if (moves_played+slack_length_direct>=min_length
             && slice_has_solution(next))
      result = false;
    else
    {
      slice_index const peer = slices[si].u.pipe.next;
      if (n>slack_length_direct
          && branch_d_has_solution_in_n(peer,n,curr_max_nr_nontrivial))
        result = false;
      else
        result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef enum
{
  defender_is_immobile,
  defender_has_refutation,
  defender_has_no_refutation
} defender_has_refutation_type;

/* Determine whether the defender has a refutation (including being
 * immobile) 
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return information about defender's possibilities
 */
static defender_has_refutation_type
has_defender_refutation(slice_index si,
                        stip_length_type n,
                        int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  defender_has_refutation_type result = defender_is_immobile;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==1);

  move_generation_mode =
      n-1>slack_length_direct
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (result!=defender_has_refutation && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender))
    {
      result = defender_has_no_refutation;
      if (branch_d_defender_is_refuted(si,n-1,curr_max_nr_nontrivial))
      {
        result = defender_has_refutation;
        coupfort();
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Count all non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * @return number of defender's non-trivial moves minus 1 (TODO: why?)
 */
static int count_all_nontrivial_defenses(slice_index si)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  int result = -1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  genmove(defender);

  TraceValue("%u",nbcou);
  TraceValue("%u",nbply);
  TraceValue("%u",repere[nbply]);
  TraceValue("%u",min_length_nontrivial);
  TraceValue("%d\n",result);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender))
    {
      if (min_length_nontrivial==0)
        ++result;
      else
      {
        if (branch_d_defender_is_refuted(si,
                                         2*min_length_nontrivial
                                         +slack_length_direct,
                                         max_nr_nontrivial))
          ++result;
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

/* Count non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * Stop counting when more than max_nr_nontrivial have been found
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return number of defender's non-trivial moves minus 1 (TODO: why?)
 */
static int count_enough_nontrivial_defenses(slice_index si,
                                            int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  int result = -1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  genmove(defender);

  TraceValue("%u",max_nr_nontrivial);
  TraceValue("%u\n",min_length_nontrivial);

  while (encore() && curr_max_nr_nontrivial>=result)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender))
    {
      if (min_length_nontrivial==0)
        ++result;
      else
      {
        if (branch_d_defender_is_refuted(si,
                                         2*min_length_nontrivial
                                         +slack_length_direct,
                                         curr_max_nr_nontrivial))
          ++result;
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are too many non-trivial defenses
 * respective to user input
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender has too many non-trivial defenses
 */
static boolean too_many_nontrivial_defenses(slice_index si,
                                            stip_length_type n,
                                            int curr_max_nr_nontrivial)
{
  boolean result;
  int nontrivial_count;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nontrivial_count = count_enough_nontrivial_defenses(si,
                                                      curr_max_nr_nontrivial);
  if (curr_max_nr_nontrivial<nontrivial_count)
    result = true;
  else
    result = (has_defender_refutation(si,
                                      n,
                                      curr_max_nr_nontrivial-nontrivial_count)
              !=defender_has_no_refutation);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the threat after the attacker's move just played
 * is too long respective to user input.
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff threat is too long
 */
static boolean is_threat_too_long(slice_index si,
                                  stip_length_type n,
                                  int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==0);

  TraceValue("%u\n",2*max_len_threat);
  if (n>=2*max_len_threat+slack_length_direct
      && !echecc(nbply,defender))
    result = branch_d_defender_is_refuted(si,
                                          2*max_len_threat,
                                          curr_max_nr_nontrivial);
  else
    /* remainder of play is too short for max_len_threat to apply */
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff defender wins
 */
boolean branch_d_defender_does_defender_win(slice_index si,
                                            stip_length_type n,
                                            int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==1);

  TraceValue("%u\n",slices[si].u.pipe.u.branch_d_defender.min_length);

  if ((OptFlag[solflights]
       && n-2>slack_length_direct
       && has_too_many_flights(defender))
      || is_threat_too_long(si,n-1,curr_max_nr_nontrivial))
    result = true;
  else if (n>2*min_length_nontrivial+slack_length_direct)
    result = too_many_nontrivial_defenses(si,n,curr_max_nr_nontrivial);
  else
    result = (has_defender_refutation(si,n,curr_max_nr_nontrivial)
              !=defender_has_no_refutation);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice index
 * @param n (odd) number of moves until goal (after the move just
 *          played)
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff attacker has won
 */
static boolean has_starter_won_in_n(slice_index si,
                                    stip_length_type n,
                                    int curr_max_nr_nontrivial)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;

  if (slice_has_starter_apriori_lost(next))
    result = true;
  else if (slices[si].u.pipe.u.branch_d_defender.length-n
           >slices[si].u.pipe.u.branch_d_defender.min_length
           && slice_has_starter_reached_goal(next))
    result = false;
  else
    result = !branch_d_defender_does_defender_win(si,n,curr_max_nr_nontrivial);

  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_d_defender_has_starter_won(slice_index si)
{
  return has_starter_won_in_n(si,
                              slices[si].u.pipe.u.branch_d_defender.length,
                              max_nr_nontrivial);
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_d_defender_has_starter_reached_goal(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_reached_goal(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_d_defender_has_non_starter_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(slices[si].u.pipe.u.branch_d_defender.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n (even) number of moves until goal (after the defense)
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
static boolean defends_against_threats(table threats,
                                       slice_index si,
                                       stip_length_type n,
                                       int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==0);

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;

    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats)
          && !echecc(nbply,attacker))
      {
        if (n==slack_length_direct)
        {
          slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
          defense_found = !slice_has_starter_won(next);
        }
        else
          defense_found = !has_starter_won_in_n(si,n-1,curr_max_nr_nontrivial);

        if (defense_found)
        {
          coupfort();
        }
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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played is relevant
 * @param len_threat length of threat
 * @param table containing threats
 * @param si slice index
 * @param n (even) number of half moves until goal after the defense
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 */
static boolean is_defense_relevant(int len_threat,
                                   table threats,
                                   slice_index si,
                                   stip_length_type n,
                                   int curr_max_nr_nontrivial)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==0);

  if (n>slack_length_direct && OptFlag[noshort]
      && !branch_d_defender_is_refuted(si,n-2,curr_max_nr_nontrivial))
    /* variation shorter than stip */
    result = false;
  else if (len_threat>slack_length_direct
           && !branch_d_defender_is_refuted(si,
                                            len_threat-2,
                                            curr_max_nr_nontrivial))
    /* variation shorter than threat */
    /* TODO avoid double calculation if lenthreat==n*/
    result = false;
  else if (slice_has_non_starter_solved(slices[si].u.pipe.u.branch_d_defender.towards_goal))
    /* "defense" has reached goal in self/reflex play */
    result = false;
  else if (!defends_against_threats(threats,
                                    si,
                                    len_threat,
                                    curr_max_nr_nontrivial))
    /* defense doesn't defend against threat */
    result = false;
  else
    result = true;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a variation. The defense that starts the variation has
 * already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index
 * @param n (odd) number of half moves until end state is to be reached
 * @return true iff variation is solvable
 */
static boolean write_variation(slice_index si, stip_length_type n)
{
  boolean is_refutation = true; /* until we prove otherwise */
  stip_length_type i;
  slice_index const peer = slices[si].u.pipe.next;
  stip_length_type min_len;
  table const continuations = allocate_table();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==1);

  write_defense();

  output_start_continuation_level();

  TraceValue("%u\n",slices[si].u.pipe.u.branch_d_defender.min_length);
  if (slices[si].u.pipe.u.branch_d_defender.min_length+slack_length_direct<=n
      || n==slack_length_direct+1)
  {
    slice_solve_continuations(continuations,
                              slices[si].u.pipe.u.branch_d_defender.towards_goal);
    is_refutation = table_length(continuations)==0;
    min_len = slack_length_direct+3;
  }
  else
    min_len = n;

  for (i = min_len; i<=n && is_refutation; i += 2)
  {
    branch_d_solve_continuations_in_n(continuations,peer,i-1);
    is_refutation = table_length(continuations)==0;
  }

  output_end_continuation_level();

  free_table();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",!is_refutation);
  return !is_refutation;
}

/* Determine and write the variations after the move that has just
 * been played in the current ply.
 * We have already determined that this move doesn't have refutations
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff >=1 solution was found
 */
static boolean solve_variations_in_n(int len_threat,
                                     table threats,
                                     slice_index si,
                                     stip_length_type n,
                                     int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side defender = advers(attacker);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==1);

  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender)
        && is_defense_relevant(len_threat,
                               threats,
                               si,
                               n-1,
                               curr_max_nr_nontrivial))
      result = write_variation(si,n);

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the threats after the move that has just been
 * played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * @param threats table where to add threats
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return the length of the shortest threat(s)
 */
static int solve_threats(table threats, slice_index si, stip_length_type n)
{
  Side const defender = advers(slices[si].starter);
  slice_index const peer = slices[si].u.pipe.next;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==0);

  if (!(OptFlag[nothreat] || echecc(nbply,defender)))
  {
    output_start_threat_level();

    slice_solve_continuations(threats,slices[si].u.pipe.u.branch_d_defender.towards_goal);
    if (table_length(threats)>0)
      result = slack_length_direct;
    else
    {
      stip_length_type
          max_threat_length = (n>2*max_len_threat+slack_length_direct
                               ? 2*max_len_threat+slack_length_direct
                               : n);

      stip_length_type i;
      TraceValue("%u",2*max_len_threat+slack_length_direct);
      TraceValue("%u\n",max_threat_length);

      for (i = slack_length_direct+2; i<=max_threat_length; i += 2)
      {
        branch_d_solve_continuations_in_n(threats,peer,i);
        TraceValue("%u",i);
        TraceValue("%u\n",table_length(threats));
        if (table_length(threats)>0)
        {
          result = i;
          break;
        }
      }
    }

    output_end_threat_level();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_solve_postkey_in_n(slice_index si, stip_length_type n)
{
  table const threats = allocate_table();
  int len_threat;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==1);

  output_start_postkey_level();

  len_threat = solve_threats(threats,si,n-1);
  if (n>2*min_length_nontrivial+slack_length_direct)
  {
    int const nontrivial_count = count_all_nontrivial_defenses(si);
    result = solve_variations_in_n(len_threat,
                                   threats,
                                   si,
                                   n,
                                   max_nr_nontrivial-nontrivial_count);
  }
  else
    result = solve_variations_in_n(len_threat,threats,si,n,max_nr_nontrivial);

  output_end_postkey_level();

  free_table();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to finish the solution of the next slice starting with the key
 * move just played. 
 * @param si slice index
 * @return true iff finishing the solution was successful.
 */
boolean branch_d_defender_finish_solution_next(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.u.branch_d_defender.min_length<=slack_length_direct)
  {
    slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;

    if (slice_has_starter_won(next))
    {
      slice_root_write_key(next,attack_key);
      output_start_postkey_level();
      slice_solve_postkey(next);
      output_end_postkey_level();
      write_end_of_solution();
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find solutions in next slice
 * @param si slice index
 * @return true iff >=1 solution has been found
 */
boolean branch_d_defender_solve_next(slice_index si)
{
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
  stip_length_type const min_length = slices[si].u.pipe.u.branch_d_defender.min_length;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",min_length);
  if (min_length<slack_length_direct && slice_has_non_starter_solved(next))
  {
    slice_write_non_starter_has_solved(next);
    result = true;
  }
  else if (min_length<=slack_length_direct && slice_has_solution(next))
  {
    table const continuations = allocate_table();
    output_start_continuation_level();
    slice_solve_continuations(continuations,next);
    output_end_continuation_level();
    free_table();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write at root level the threat and variations after
 * the move that has just been played in the current ply
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param refutations table containing refutations after move just
 *                    played
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 */
static void root_solve_variations_in_n(int len_threat,
                                       table threats,
                                       table refutations,
                                       slice_index si,
                                       stip_length_type n,
                                       int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==1);

  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender)
        && !is_current_move_in_table(refutations)
        && is_defense_relevant(len_threat,
                               threats,
                               si,
                               n-1,
                               curr_max_nr_nontrivial))
      write_variation(si,n);

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 */
void branch_d_defender_root_solve_postkey(table refutations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_postkey_level();

  if (OptFlag[solvariantes])
  {
    stip_length_type const n = slices[si].u.pipe.u.branch_d_defender.length;
    table const threats = allocate_table();
    int const len_threat = solve_threats(threats,si,n-1);

    if (n>2*min_length_nontrivial+slack_length_direct)
    {
      int const nontrivial_count =
          count_enough_nontrivial_defenses(si,max_nr_nontrivial);
      root_solve_variations_in_n(len_threat,
                                 threats,
                                 refutations,
                                 si,
                                 n,
                                 max_nr_nontrivial-nontrivial_count);
    }
    else
      root_solve_variations_in_n(len_threat,
                                 threats,
                                 refutations,
                                 si,
                                 n,
                                 max_nr_nontrivial);

    free_table();
  }

  write_refutations(refutations);

  output_end_postkey_level();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_root_solve(slice_index si)
{
  stip_length_type const n = slices[si].u.pipe.u.branch_d_defender.length;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);
  result = branch_d_defender_solve_postkey_in_n(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Collect refutations at root level
 * @param t table where to add refutations
 * @param si slice index
 * @param (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff defender is immobile
 */
static boolean root_collect_refutations(table refutations,
                                        slice_index si,
                                        stip_length_type n,
                                        int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  boolean is_defender_immobile = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==1);

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
      if (branch_d_defender_is_refuted(si,n-1,curr_max_nr_nontrivial))
      {
        append_to_top_table();
        coupfort();
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",is_defender_immobile);
  TraceFunctionResultEnd();
  return is_defender_immobile;
}

/* Collect non-trivial defenses at root level
 * @param nontrivial table where to add non-trivial defenses
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return max_nr_refutations+1 if defender is immobile or there are
 *                              too many non-trivial defenses respective
 *                              to user input
 *         number of non-trivial defenses otherwise
 */
static unsigned int root_collect_nontrivial(table nontrivial,
                                            slice_index si,
                                            stip_length_type n)
{
  unsigned int result;
  int nontrivial_count;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nontrivial_count = count_enough_nontrivial_defenses(si,max_nr_nontrivial);
  if (max_nr_nontrivial<nontrivial_count)
    result = max_nr_refutations+1;
  else
    result = (root_collect_refutations(nontrivial,
                                       si,
                                       n,
                                       max_nr_nontrivial-nontrivial_count)
              ? max_nr_refutations+1
              : table_length(nontrivial));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
unsigned int branch_d_defender_find_refutations(table refutations,
                                                slice_index si)
{
  Side const defender = advers(slices[si].starter);
  unsigned int result;
  stip_length_type const n = slices[si].u.pipe.u.branch_d_defender.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);

  TraceValue("%u",n);
  TraceValue("%u\n",min_length_nontrivial);

  if (is_threat_too_long(si,n-1,max_nr_nontrivial))
    result = max_nr_refutations+1;
  else if (n-1>slack_length_direct+2
           && OptFlag[solflights] && has_too_many_flights(defender))
    result = max_nr_refutations+1;
  else if (n>2*min_length_nontrivial+slack_length_direct)
    result = root_collect_nontrivial(refutations,si,n);
  else
    result = (root_collect_refutations(refutations,si,n,max_nr_nontrivial)
              ? max_nr_refutations+1
              : table_length(refutations));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at non-root-level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_d_defender_make_setplay_slice(slice_index si)
{
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
  slice_index next_in_setplay;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.u.branch_d_defender.length==slack_length_direct+1)
    next_in_setplay = next;
  else
  {
    slice_index const peer = slices[si].u.pipe.next;

    slice_index const next_in_setplay_peer = copy_slice(si);
    slices[next_in_setplay_peer].u.pipe.u.branch_d_defender.length -= 2;
    if (slices[next_in_setplay_peer].u.pipe.u.branch_d_defender.min_length==0)
      slices[next_in_setplay_peer].u.pipe.u.branch_d_defender.min_length = 1;
    else
      --slices[next_in_setplay_peer].u.pipe.u.branch_d_defender.min_length;

    next_in_setplay = copy_slice(peer);
    slices[next_in_setplay].u.pipe.u.branch_d_defender.length -= 2;
    slices[next_in_setplay].u.pipe.u.branch_d_defender.min_length -= 2;
    hash_slice_is_derived_from(next_in_setplay,peer);

    slices[next_in_setplay].u.pipe.next = next_in_setplay_peer;
    slices[next_in_setplay_peer].u.pipe.next = next_in_setplay;
  }

  result = alloc_branch_h_slice(slack_length_help+1,
                                slack_length_help+1,
                                next_in_setplay);
  slices[result].starter
      = advers(slices[si].starter);

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
who_decides_on_starter
branch_d_defender_detect_starter(slice_index si, boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;
  slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
  slice_index next_relevant = next;
  Side * const starter = &slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();
  
  if (slices[next].type==STMoveInverter)
    next_relevant = slices[next].u.pipe.u.branch_d_defender.towards_goal;

  TraceValue("%u\n",next_relevant);

  result = slice_detect_starter(next,same_side_as_root);
  if (slice_get_starter(next)==no_side)
    /* next can't tell - let's tell him */
    switch (slices[next_relevant].type)
    {
      case STLeafDirect:
        *starter =  White;
        TraceValue("%u\n",*starter);
        slice_impose_starter(next,*starter);
        break;

      case STLeafSelf:
        *starter = White;
        TraceValue("%u\n",*starter);
        slice_impose_starter(next,*starter);
        break;

      case STLeafHelp:
        *starter = White;
        TraceValue("%u\n",*starter);
        slice_impose_starter(next,advers(*starter));
        break;

      default:
        *starter = no_side;
        break;
    }
  else
    *starter = slice_get_starter(next);

  TraceValue("%u\n",*starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_d_defender_impose_starter(slice_index si, Side s)
{
  slices[si].starter = s;
  slice_impose_starter(slices[si].u.pipe.u.branch_d_defender.towards_goal,s);
}
