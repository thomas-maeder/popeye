#include "pybradd.h"
#include "pybrad.h"
#include "pydata.h"
#include "pyslice.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_d_defender_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_must_starter_resign(slices[si].u.branch.next);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",si);

  result = slice_has_starter_apriori_lost(slices[si].u.branch.next);

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

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return whether the defender wins or loses, and how fast
 */
d_defender_win_type branch_d_defender_does_defender_win(slice_index si,
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

/* Determine whether the attacker has won with his move just played
 * @param si slice index
 * @param n (odd) number of moves until goal (after the move just
 *          played)
 * @return true iff attacker has won
 */
boolean branch_d_defender_has_starter_won_in_n(slice_index si,
                                               stip_length_type n)
{
  return branch_d_defender_does_defender_win(si,n)>win;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_d_defender_has_starter_won(slice_index si)
{
  stip_length_type const n = slices[si].u.branch.length;
  return branch_d_defender_has_starter_won_in_n(si,n-1);
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_d_defender_has_starter_reached_goal(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_starter_reached_goal(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",si);

  result = slice_has_non_starter_solved(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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
          defense_found = !branch_d_defender_has_starter_won_in_n(si,n-1);

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
  else if (branch_d_defender_has_non_starter_solved(si))
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

/* Determine and write and variations after the move that has just
 * been played in the current ply.
 * We have already determined that this move doesn't have refutations
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
void branch_d_defender_solve_variations_in_n(int len_threat,
                                             table threats,
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

/* Determine and write the threats after the move that has just been
 * played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * @param threats table where to add threats
 * @param si slice index
 * @param n (even) number of half moves until goal
 * @return the length of the shortest threat(s)
 */
int branch_d_defender_solve_threats(table threats,
                                    slice_index si,
                                    stip_length_type n)
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

/* Solve postkey play in a certain number of moves
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
void branch_d_defender_solve_postkey_in_n(slice_index si, stip_length_type n)
{
  table const threats = allocate_table();
  int len_threat;

  assert(n%2==1);

  output_start_postkey_level();

  len_threat = branch_d_defender_solve_threats(threats,si,n-1);
  if (n>2*min_length_nontrivial+slack_length_direct)
  {
    int const non_trivial_count = count_non_trivial_defenses(si);
    max_nr_nontrivial -= non_trivial_count;
    branch_d_defender_solve_variations_in_n(len_threat,threats,si,n);
    max_nr_nontrivial += non_trivial_count;
  }
  else
    branch_d_defender_solve_variations_in_n(len_threat,threats,si,n);

  output_end_postkey_level();

  free_table();
}

/* Solve postkey play
 * @param si slice index
 * @param n (odd) number of half moves until goal
 */
void branch_d_defender_solve_postkey(slice_index si)
{
  branch_d_defender_solve_postkey_in_n(si,slices[si].u.branch.length-1);
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
                                       table refutations,
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

/* Continue solving after the key just played to find and write the
 * post key play (threats, variations)
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 */
void branch_d_defender_root_solve_postkey(table refutations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_postkey_level();

  if (OptFlag[solvariantes])
  {
    stip_length_type const n = slices[si].u.branch.length-1;
    table const threats = allocate_table();
    int const len_threat = branch_d_defender_solve_threats(threats,si,n-1);

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
