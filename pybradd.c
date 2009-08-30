#include "pybradd.h"
#include "pydirect.h"
#include "pybrad.h"
#include "pydata.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBranchDirectDefender defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal index of slice leading to goal
 * @return index of allocated slice
 */
slice_index alloc_branch_d_defender_slice(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index next,
                                          slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STBranchDirectDefender; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.u.branch.towards_goal =towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STDirectDefenderRoot defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param fork identifies fork slice
 * @return index of allocated slice
 */
slice_index alloc_branch_d_defender_root_slice(stip_length_type length,
                                               stip_length_type min_length,
                                               slice_index next,
                                               slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STDirectDefenderRoot; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = next;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

  result = slice_is_goal_reached(just_moved,
                                 slices[si].u.pipe.u.branch.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* TODO is this equivalent to has_solution_type? */
typedef enum
{
  defender_is_immobile,
  defender_has_refutation,
  defender_has_no_refutation
} defender_has_refutation_type;

/* Determine whether the defender has a refutation (including being
 * immobile) 
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return information about defender's possibilities
 */
static defender_has_refutation_type
has_defender_refutation(slice_index si,
                        stip_length_type n,
                        int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;
  defender_has_refutation_type result = defender_is_immobile;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  move_generation_mode =
      n-1>slack_length_direct
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (result!=defender_has_refutation && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      switch (direct_has_solution_in_n(next,n-1,curr_max_nr_nontrivial))
      {
        case defender_self_check:
          /* nothing */ ;
          break;

        case has_no_solution:
          result = defender_has_refutation;
          coupfort();
          break;

        case has_solution:
          result = defender_has_no_refutation;
          break;

        default:
          assert(0);
          break;
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
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = slices[si].u.pipe.u.branch.length%2;
  int result = -1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(defender);

  TraceValue("%u",nbcou);
  TraceValue("%u",nbply);
  TraceValue("%u",repere[nbply]);
  TraceValue("%u",min_length_nontrivial);
  TraceValue("%d\n",result);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender) /* TODO rearrange slices */
        && (min_length_nontrivial==0
            || (direct_has_solution_in_n(next,
                                         2*min_length_nontrivial
                                         +slack_length_direct
                                         +parity-1,
                                         max_nr_nontrivial)
                ==has_no_solution)))
      ++result;

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
  Side const defender = slices[si].starter;
  int result = -1;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = slices[si].u.pipe.u.branch.length%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(defender);

  TraceValue("%u",max_nr_nontrivial);
  TraceValue("%u\n",min_length_nontrivial);

  while (encore() && curr_max_nr_nontrivial>=result)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,defender) /* TODO rearrange slices */
        && (min_length_nontrivial==0
            || (direct_has_solution_in_n(next,
                                         2*min_length_nontrivial
                                         +slack_length_direct
                                         +parity-1,
                                         curr_max_nr_nontrivial)
                ==has_no_solution)))
      ++result;

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
 * @param n maximum number of half moves until goal
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

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

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
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff threat is too long
 */
static boolean is_threat_too_long(slice_index si,
                                  stip_length_type n,
                                  int curr_max_nr_nontrivial)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2!=slices[si].u.pipe.u.branch.length%2);

  TraceValue("%u\n",2*max_len_threat);
  if (n>=2*max_len_threat+slack_length_direct)
    result = (direct_has_solution_in_n(next,
                                       2*max_len_threat,
                                       curr_max_nr_nontrivial)
              ==has_no_solution);
  else
    /* remainder of play is too short for max_len_threat to apply */
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find refutations after a move of the attacking side at a nested level.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return attacker_has_reached_deadend if we are in a situation where
 *              the position after the attacking move is to be
 *              considered hopeless for the attacker
 *         attacker_has_solved_next_slice if the attacking move has
 *              solved the branch
 *         found_refutations if there is a refutation
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
branch_d_defender_find_refutations_in_n(slice_index si,
                                        stip_length_type n,
                                        int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;
  quantity_of_refutations_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if ((OptFlag[solflights]
       && n-2>slack_length_direct
       && has_too_many_flights(defender))
      || is_threat_too_long(si,n-1,curr_max_nr_nontrivial))
    result = attacker_has_reached_deadend;
  else if (n>2*min_length_nontrivial+slack_length_direct)
    result = (too_many_nontrivial_defenses(si,n,curr_max_nr_nontrivial)
              ? found_refutations
              : found_no_refutation);
  /* TODO shouldn't we continue with the next condition if we have
   * failed to detect too many nontrivial defenses? */
  else
    result = (has_defender_refutation(si,n,curr_max_nr_nontrivial)
              ==defender_has_no_refutation
              ? found_no_refutation
              : found_refutations);

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played is relevant
 * @param table containing threats
 * @param len_threat length of threat
 * @param si slice index
 * @param n maximum number of half moves until goal after the defense
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 */
static boolean is_defense_relevant(table threats,
                                   stip_length_type len_threat,
                                   slice_index si,
                                   stip_length_type n,
                                   int curr_max_nr_nontrivial)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2!=slices[si].u.pipe.u.branch.length%2);

  if (n>slack_length_direct && OptFlag[noshort]
      && (direct_has_solution_in_n(next,n-2,curr_max_nr_nontrivial)
          !=has_no_solution))
    /* variation shorter than stip */
    result = false;
  else if (len_threat>slack_length_direct+1
           && len_threat<=n
           && (direct_has_solution_in_n(next,
                                       len_threat-2,
                                       curr_max_nr_nontrivial)
               !=has_no_solution))
    /* there are threats and the variation is shorter than them */
    /* TODO avoid double calculation if lenthreat==n*/
    result = false;
  else
    result = direct_are_threats_refuted_in_n(threats,
                                             len_threat,
                                             next,
                                             n,
                                             curr_max_nr_nontrivial);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a variation. The defense that starts the variation has
 * already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index
 * @param n maximum number of half moves until end state is to be reached
 * @return true iff variation is solvable
 */
static boolean write_variation(slice_index si, stip_length_type n)
{
  boolean is_refutation = true; /* until we prove otherwise */
  stip_length_type i;
  slice_index const next = slices[si].u.pipe.next;
  table const continuations = allocate_table();

  stip_length_type const length_span = (slices[si].u.pipe.u.branch.length
                                        -slices[si].u.pipe.u.branch.min_length);

  stip_length_type const i_min = (n<=slack_length_direct+length_span
                                  ? (slack_length_direct+2
                                     -(n-slack_length_direct)%2)
                                  : n-length_span);
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  write_defense();

  output_start_continuation_level();

  for (i = i_min; i<=n && is_refutation; i += 2)
  {
    direct_solve_continuations_in_n(continuations,next,i-1);
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
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff >=1 solution was found
 */
static boolean solve_variations_in_n(stip_length_type len_threat,
                                     table threats,
                                     slice_index si,
                                     stip_length_type n,
                                     int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && is_defense_relevant(threats,
                               len_threat,
                               si,
                               n-1,
                               curr_max_nr_nontrivial))
    {
      if (write_variation(si,n))
        result = true;
      else
        write_refutation_mark();
    }

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
 * @param n maximum number of half moves until goal
 * @return (n-slack_length_direct)%2 if the defender is in check
 *         a value >max_threat_length if there is no threat nor check
 *         the length of the shortest threat(s) otherwise
 */
static stip_length_type solve_threats(table threats,
                                      slice_index si,
                                      stip_length_type n)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;
  unsigned int const parity = (n-slack_length_direct)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2!=slices[si].u.pipe.u.branch.length%2);

  stip_length_type const
      max_threat_length = (n>2*max_len_threat+slack_length_direct
                           ? 2*max_len_threat+slack_length_direct+parity
                           : n);
  TraceValue("%u\n",max_threat_length);

  if (OptFlag[nothreat])
    result = max_threat_length+2;
  else if (echecc(nbply,defender))
    result = (n-slack_length_direct)%2;
  else
  {
    output_start_threat_level();

    result = slack_length_direct+parity;

    while (result<=max_threat_length)
    {
      direct_solve_continuations_in_n(threats,next,result);
      TraceValue("%u",result);
      TraceValue("%u\n",table_length(threats));
      if (table_length(threats)>0)
        break;
      else
        result += 2;
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
 * @param n maximum number of half moves until goal
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_solve_postkey_in_n(slice_index si,
                                             stip_length_type n)
{
  table const threats = allocate_table();
  stip_length_type len_threat;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

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

/****************** root *******************/

/* Determine and write at root level the threat and variations after
 * the move that has just been played in the current ply
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param refutations table containing refutations after move just
 *                    played
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 */
static void root_solve_variations_in_n(stip_length_type len_threat,
                                       table threats,
                                       table refutations,
                                       slice_index si,
                                       stip_length_type n,
                                       int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !is_current_move_in_table(refutations)
        && is_defense_relevant(threats,
                               len_threat,
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
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_root_solve_postkey(table refutations, slice_index si)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_postkey_level();

  if (OptFlag[solvariantes])
  {
    stip_length_type const n = slices[si].u.pipe.u.branch.length;
    table const threats = allocate_table();
    stip_length_type const len_threat = solve_threats(threats,si,n-1);
    TraceValue("%u",n);
    TraceValue("%u\n",len_threat);

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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_defender_root_solve(slice_index si)
{
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);
  if (branch_d_defender_solve_postkey_in_n(si,n))
  {
    write_end_of_solution();
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Collect refutations at root level
 * @param t table where to add refutations
 * @param si slice index
 * @param maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
static
quantity_of_refutations_type root_collect_refutations(table refutations,
                                                      slice_index si,
                                                      stip_length_type n,
                                                      int curr_max_nr_nontrivial)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  quantity_of_refutations_type result = attacker_has_reached_deadend;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  if (n-1>slack_length_direct+2)
    move_generation_mode= move_generation_mode_opti_per_side[defender];
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore()
         && table_length(refutations)<=max_nr_refutations)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      switch (direct_has_solution_in_n(next,n-1,curr_max_nr_nontrivial))
      {
        case defender_self_check:
          /* nothing */ ;
          break;

        case has_solution:
          if (result==attacker_has_reached_deadend)
            result = found_no_refutation;
          break;

        case has_no_solution:
          append_to_top_table();
          coupfort();
          result = found_refutations;
          break;

        default:
          assert(0);
          break;
      }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Collect non-trivial defenses at root level
 * @param nontrivial table where to add non-trivial defenses
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
static quantity_of_refutations_type root_collect_nontrivial(table nontrivial,
                                                            slice_index si,
                                                            stip_length_type n)
{
  unsigned int result;
  int nontrivial_count;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  nontrivial_count = count_enough_nontrivial_defenses(si,max_nr_nontrivial);
  if (max_nr_nontrivial<nontrivial_count)
    result = attacker_has_reached_deadend;
  else
    result = root_collect_refutations(nontrivial,
                                      si,
                                      n,
                                      max_nr_nontrivial-nontrivial_count);

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Find refutations after a move of the attacking side at root level.
 * @param t table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
branch_d_defender_root_find_refutations(table refutations, slice_index si)
{
  Side const defender = slices[si].starter;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  quantity_of_refutations_type result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",table_length(refutations));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",n);
  TraceValue("%u\n",min_length_nontrivial);

  if (is_threat_too_long(si,n-1,max_nr_nontrivial))
    result = attacker_has_reached_deadend;
  else if (n-1>slack_length_direct+2
           && OptFlag[solflights] && has_too_many_flights(defender))
    result = attacker_has_reached_deadend;
  else if (n>2*min_length_nontrivial+slack_length_direct)
    result = root_collect_nontrivial(refutations,si,n);
  else
    result = root_collect_refutations(refutations,si,n,max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter
branch_d_defender_root_detect_starter(slice_index si, boolean same_side_as_root)
{
  who_decides_on_starter const result = leaf_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();
  
  if (slices[si].starter==no_side)
    slices[si].starter = Black;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index branch_d_defender_root_reduce_to_postkey_play(slice_index si)
{
  slice_index const result = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
