#include "pybradd.h"
#include "pydirect.h"
#include "pybrad.h"
#include "pydata.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pyoutput.h"
#include "pymsg.h"
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

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return success of key move
 */
attack_result_type branch_d_defender_defend_in_n(slice_index si,
                                                 stip_length_type n)
{
  Side const defender = slices[si].starter;
  attack_result_type result = attack_has_reached_deadend;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n-1+min_length>slack_length_direct+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_direct-parity;

  active_slice[nbply+1] = si;
  move_generation_mode =
      n-1>slack_length_direct
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (result!=attack_refuted_full_length && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      switch (direct_has_solution_in_n(next,n-1,n_min_next))
      {
        case defender_self_check:
          /* nothing */ ;
          break;

        case has_no_solution:
          result = attack_refuted_full_length;
          coupfort();
          break;

        case has_solution:
          result = attack_solves_full_length;
          break;

        default:
          assert(0);
          break;
      }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

static boolean has_short_solution(slice_index si, stip_length_type n)
{
  boolean result;
  stip_length_type n_min;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = n%2;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  n -= 2;

  if (n+min_length>slack_length_direct+length)
    n_min = n-(length-min_length);
  else
    n_min = slack_length_direct-parity;

  result = direct_has_solution_in_n(next,n,n_min)!=has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played is relevant
 * @param table containing threats
 * @param len_threat length of threat
 * @param si slice index
 * @param n maximum number of half moves until goal after the defense
 */
static boolean is_defense_relevant(table threats,
                                   stip_length_type len_threat,
                                   slice_index si,
                                   stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2!=slices[si].u.pipe.u.branch.length%2);

  if (n>slack_length_direct && OptFlag[noshort] && has_short_solution(si,n))
    /* variation shorter than stip */
    result = false;
  else if (len_threat>slack_length_direct+1
           && len_threat<=n
           && has_short_solution(si,len_threat))
    /* there are threats and the variation is shorter than them */
    /* TODO avoid double calculation if lenthreat==n*/
    result = false;
  else
    result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);
  
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
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  table const continuations = allocate_table();
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  write_defense();

  output_start_continuation_level();
  result = direct_solve_continuations_in_n(continuations,next,n-1)<=n-1;
  output_end_continuation_level();

  free_table();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve variations after an attacker's move
 * @param threats table containing the threats after the attacker's move
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >=1 variation was found
 */
boolean branch_d_defender_solve_variations_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n)
{
  Side const defender = slices[si].starter;
  boolean result = false;

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
        && is_defense_relevant(threats,len_threat,si,n-1))
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

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int branch_d_defender_can_defend_in_n(slice_index si,
                                               stip_length_type n,
                                               unsigned int max_result)
{
  Side const defender = slices[si].starter;
  unsigned int result = 0;
  slice_index const next = slices[si].u.pipe.next;
  boolean isDefenderImmobile = true;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n-1+min_length>slack_length_direct+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_direct-parity;

  active_slice[nbply+1] = si;
  move_generation_mode =
      n-1>slack_length_direct
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (result<=max_result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      switch (direct_has_solution_in_n(next,n-1,n_min_next))
      {
        case defender_self_check:
          /* nothing */ ;
          break;

        case has_no_solution:
          ++result;
          coupfort();
          isDefenderImmobile = false;
          break;

        case has_solution:
          isDefenderImmobile = false;
          break;

        default:
          assert(0);
          break;
      }

    repcoup();
  }

  finply();

  if (isDefenderImmobile)
    result = max_result+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/****************** root *******************/

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type branch_d_defender_solve_threats(table threats,
                                                 slice_index si,
                                                 stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2!=slices[si].u.pipe.u.branch.length%2);

  result = direct_solve_threats(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void branch_d_defender_root_solve_variations(table threats,
                                             stip_length_type len_threat,
                                             table refutations,
                                             slice_index si)
{
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  Side const defender = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !is_current_move_in_table(refutations)
        && is_defense_relevant(threats,len_threat,si,length-1))
      write_variation(si,length);

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 solution was found
 */
static boolean solve_postkey_in_n(slice_index si, stip_length_type n)
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

  if (OptFlag[nothreat])
    len_threat = n+1;
  else
  {
    Message(NewLine);
    len_threat = branch_d_defender_solve_threats(threats,si,n-1);
    if (len_threat==n+1)
      Message(Zugzwang);
  }

  result = branch_d_defender_solve_variations_in_n(threats,len_threat,si,n);

  output_end_postkey_level();

  free_table();

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
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  if (solve_postkey_in_n(si,length))
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
 * @return true if defender is immobile
 */
static boolean root_collect_refutations(table refutations,
                                        slice_index si,
                                        stip_length_type n)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = true;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n-1+min_length>slack_length_direct+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_direct-parity;

  active_slice[nbply+1] = si;
  if (n-1>slack_length_direct+2)
    move_generation_mode= move_generation_mode_opti_per_side[defender];
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore()
         && table_length(refutations)<=max_nr_refutations)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      switch (direct_has_solution_in_n(next,n-1,n_min_next))
      {
        case defender_self_check:
          /* nothing */ ;
          break;

        case has_solution:
          result = false;
          break;

        case has_no_solution:
          result = false;
          append_to_top_table();
          coupfort();
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

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return success of key move
 */
attack_result_type branch_d_defender_root_defend(table refutations,
                                                 slice_index si)
{
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  attack_result_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (root_collect_refutations(refutations,si,length))
    result = attack_has_reached_deadend;
  else if (table_length(refutations)==0)
    result = attack_solves_full_length;
  else
    result = attack_refuted_full_length;

  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter
branch_d_defender_root_detect_starter(slice_index si,
                                      boolean same_side_as_root)
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
