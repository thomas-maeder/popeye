#include "pybrad.h"
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
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
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
 * @return information about defender's possibilities
 */
static
defender_has_refutation_type has_defender_refutation(slice_index si,
                                                     stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  Side const defender = advers(attacker);
  defender_has_refutation_type result = defender_is_immobile;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==1);

  move_generation_mode =
      n-1>slack_length_direct+2
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (result!=defender_has_refutation && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender))
    {
      result = defender_has_no_refutation;
      (*encode)();
      if (branch_d_has_solution_in_n(si,n-1)==branch_d_no_solution)
      {
        result = defender_has_refutation;
        coupfort();
      }
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there are too many non-trivial defenses
 * respective to user input
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return true iff the defender has too many non-trivial defenses
 */
static boolean too_many_non_trivial_defenses(slice_index si,
                                             stip_length_type n)
{
  boolean result;
  int non_trivial_count;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  non_trivial_count = count_non_trivial_defenses(si);
  if (max_nr_nontrivial<non_trivial_count)
    result = true;
  else
  {
    max_nr_nontrivial -= non_trivial_count;
    result = has_defender_refutation(si,n)!=defender_has_no_refutation;
    max_nr_nontrivial += non_trivial_count;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


/* The enumerators in the following enumeration type are sorted in
 * descending possibilities of the defender.
 */
typedef enum
{
  already_won,
  win,
  loss,
  already_lost
} d_defender_win_type;

/* Determine whether the defender wins after a move by the attacker
 * @param defender defending side (at move)
 * @param n (odd) number of half moves until goal
 * @return whether the defender wins or loses, and how fast
 */
static d_defender_win_type does_defender_win(slice_index si,
                                             stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  Side const defender = advers(attacker);
  d_defender_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==1);

  TraceValue("%u\n",slices[si].u.branch.min_length);

  if (slice_has_starter_apriori_lost(slices[si].u.branch.next))
    result = already_won;
  else if (slices[si].u.branch.length-n>slices[si].u.branch.min_length
           && slice_has_starter_reached_goal(slices[si].u.branch.next))
    result = already_lost;
  else if (is_threat_too_long(si,n-1)
           || (OptFlag[solflights] && has_too_many_flights(defender)))
    result = win;
  else if (n-1>2*min_length_nontrivial+slack_length_direct)
    result = too_many_non_trivial_defenses(si,n) ? win : loss;
  else
    result = (has_defender_refutation(si,n)!=defender_has_no_refutation
              ? win
              : loss);

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
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && does_defender_win(si,n-1)>=loss)
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
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender))
    {
      is_defender_immobile = false;
      (*encode)();
      if (branch_d_has_solution_in_n(si,n-1)==branch_d_no_solution)
        table_append(refutations);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",is_defender_immobile);
  return is_defender_immobile;
}

/* Collect non-trivial defenses at root level
 * @param t table where to add non-trivial defenses
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return max_nr_refutations+1 if defender is immobile or there are
 *                              too many non-trivial defenses respective
 *                              to user input
 *         number of non-trivial defenses otherwise
 */
static unsigned int root_collect_non_trivial(int t,
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
    result = (root_collect_refutations(t,si,n)
              ? max_nr_refutations+1
              : table_length(t));
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

/* Determine whether the attacker has won with his move just played
 * @param si slice index
 * @param n (odd) number of moves until goal (after the move just
 *          played)
 * @return true iff attacker has won
 */
static boolean has_starter_won_in_n(slice_index si, stip_length_type n)
{
  return does_defender_win(si,n)>win;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice index
 * @return true iff attacker has won
 */
boolean branch_d_has_starter_won(slice_index si)
{
  return has_starter_won_in_n(si,slices[si].u.branch.length-1);
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n (even) number of moves until goal (after the defense)
 * @return true iff the defense defends against at least one of the
 *         threats
 */
static boolean defends_against_threats(table threats,
                                       slice_index si,
                                       stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;

    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && is_current_move_in_table(threats)
          && !echecc(nbply,attacker))
      {
        if (n==slack_length_direct)
          defense_found = !slice_has_starter_won(slices[si].u.branch.next);
        else
          defense_found = !has_starter_won_in_n(si,n-1);

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
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a variation. The defense that starts the variation has
 * already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index
 * @param n (odd) number of half moves until end state is to be reached
 */
static void write_variation(slice_index si, stip_length_type n)
{
  boolean is_refutation = true; /* until we prove otherwise */
  stip_length_type i;
  stip_length_type const
      min_len = (slices[si].u.branch.min_length+slack_length_direct>n
                 ? n
                 : slack_length_direct+1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==1);

  write_defense();

  output_start_continuation_level();
  
  for (i = min_len; i<=n && is_refutation; i += 2)
  {
    table const continuations = allocate_table();
    branch_d_solve_continuations_in_n(continuations,si,i-1);
    is_refutation = table_length(continuations)==0;
    free_table();
  }

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceText("\n");
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
  Side const defender = advers(slices[si].u.branch.starter);
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  if (!(OptFlag[nothreat] || echecc(nbply,defender)))
  {
    stip_length_type
        max_threat_length = (n>2*max_len_threat+slack_length_direct
                             ? 2*max_len_threat+slack_length_direct
                             : n);
    stip_length_type i;

    TraceValue("%u",2*max_len_threat+slack_length_direct);
    TraceValue("%u\n",max_threat_length);
    output_start_threat_level();

    for (i = slack_length_direct; i<=max_threat_length; i += 2)
    {
      branch_d_solve_continuations_in_n(threats,si,i);
      TraceValue("%u",i);
      TraceValue("%u\n",table_length(threats));
      if (table_length(threats)>0)
      {
        result = i;
        break;
      }
    }

    output_end_threat_level();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the defense just played is relevant
 * @param len_threat length of threat
 * @param table containing threats
 * @param si slice index
 * @param n (even) number of half moves until goal after the defense
 */
static boolean is_defense_relevant(int len_threat,
                                   table threats,
                                   slice_index si,
                                   stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==0);

  (*encode)();
  if (n>slack_length_direct && OptFlag[noshort]
      && (branch_d_has_solution_in_n(si,n-2)<=branch_d_we_solve))
    /* variation shorter than stip */
    result = false;
  else if (len_threat>slack_length_direct
           && branch_d_has_solution_in_n(si,len_threat-2)<=branch_d_we_solve)
    /* variation shorter than threat */
    /* TODO avoid double calculation if lenthreat==n*/
    result = false;
  else if (slice_has_non_starter_solved(si))
    /* "defense" has reached goal in self/reflex play */
    result = false;
  else if (!defends_against_threats(threats,si,len_threat))
    /* defense doesn't defend against threat */
    result = false;
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
 */
static void root_solve_variations_in_n(int len_threat,
                                       table threats,
                                       int refutations,
                                       slice_index si,
                                       stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  Side defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==1);

  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender)
        && !is_current_move_in_table(refutations)
        && is_defense_relevant(len_threat,threats,si,n-1))
      write_variation(si,n);

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the threat and variations after the move that
 * has just been played in the current ply. 
 * We have already determined that this move doesn't have refutations
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
static void solve_variations_in_n(int len_threat,
                                  int threats,
                                  slice_index si,
                                  stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
  Side defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n%2==1);

  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender)
        && is_defense_relevant(len_threat,threats,si,n-1))
      write_variation(si,n);

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
}

/* Solve postkey play in a certain number of moves
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
static void solve_postkey_in_n(slice_index si, stip_length_type n)
{
  table const threats = allocate_table();
  int len_threat;

  assert(n%2==1);

  output_start_postkey_level();

  len_threat = solve_threats(threats,si,n-1);
  if (n>2*min_length_nontrivial+slack_length_direct)
  {
    int const non_trivial_count = count_non_trivial_defenses(si);
    max_nr_nontrivial -= non_trivial_count;
    solve_variations_in_n(len_threat,threats,si,n);
    max_nr_nontrivial += non_trivial_count;
  }
  else
    solve_variations_in_n(len_threat,threats,si,n);

  output_end_postkey_level();

  free_table();
}

/* Solve postkey play
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
void branch_d_solve_postkey(slice_index si)
{
  solve_postkey_in_n(si,slices[si].u.branch.length-1);
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
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !echecc(nbply,attacker))
      {
        d_defender_win_type const
            defender_success = does_defender_win(si,n-1);
        TraceValue("%u\n",defender_success);
        if (defender_success>=loss)
        {
          write_attack(attack_regular);

          if (defender_success==already_lost)
            slice_solve_postkey(slices[si].u.branch.next);
          else
            solve_postkey_in_n(si,n-1);

          table_append(continuations);
        }
      }

      repcoup();
    }

    finply();
  }

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

/* Continue solving after the key just played to find and write the
 * post key play (threats, variations)
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 */
void branch_d_root_solve_postkey(table refutations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_postkey_level();

  if (OptFlag[solvariantes])
  {
    stip_length_type const n = slices[si].u.branch.length-1;
    table const threats = allocate_table();
    int const len_threat = solve_threats(threats,si,n-1);

    if (n>2*min_length_nontrivial+slack_length_direct)
    {
      int const non_trivial_count = count_non_trivial_defenses(si);
      max_nr_nontrivial -= non_trivial_count;
      root_solve_variations_in_n(len_threat,threats,refutations,si,n);
      max_nr_nontrivial += non_trivial_count;
    }
    else
      root_solve_variations_in_n(len_threat,threats,refutations,si,n);

    free_table();
  }

  write_refutations(refutations);

  output_end_postkey_level();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve the postkey play only at root level.
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
static void root_solve_postkeyonly(slice_index si, stip_length_type n)
{
  output_start_postkeyonly_level();

  assert(n%2==1);

  /* TODO does this all make sense?? */
  if (n==slices[si].u.branch.min_length+1)
    solve_postkey_in_n(si,n);
  else if (slice_has_starter_reached_goal(slices[si].u.branch.next))
    slice_solve_postkey(slices[si].u.branch.next);
  else
    solve_postkey_in_n(si,n);

  output_end_postkeyonly_level();
}

/* Solve the solutions and tries
 * @param si slice index
 */
static void root_solve_real_play(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_continuation_level();

  if (slice_must_starter_resign(si))
    slice_write_unsolvability(slices[si].u.branch.next);
  else if (slices[si].u.branch.length==slack_length_direct)
    slice_root_solve(slices[si].u.branch.next);
  else
  {
    Side const attacker = slices[si].u.branch.starter;
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
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
            attack_type const type = (table_length(refutations)>=1
                                      ? attack_try
                                      : attack_key);
            branch_d_root_write_key(si,type);
            branch_d_root_solve_postkey(refutations,si);
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

    finply();
  }

  output_end_continuation_level();

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

    write_defense();

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
      root_solve_postkeyonly(si,n);
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
    slice_index const derived = copy_slice(si);
    slices[derived].u.branch.length -= 2;
    slices[derived].u.branch.min_length -= 2;
    slices[derived].u.branch.starter = slices[si].u.branch.starter;
    result = alloc_branch_slice(STBranchHelp,
                                slack_length_help+1,
                                slack_length_help+1,
                                derived);
    slices[result].u.branch.starter = advers(slices[si].u.branch.starter);
    hash_slice_is_derived_from(derived,si);
  }

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
  slice_impose_starter(slices[si].u.branch.next,s);
}
