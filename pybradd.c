#include "pybradd.h"
#include "pydirect.h"
#include "pydata.h"
#include "pyslice.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_root.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/fork.h"
#include "pyoutput.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STBranchDirectDefender defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_branch_d_defender_slice(stip_length_type length,
                                          stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_direct)
    min_length += 2;
  assert(min_length>=slack_length_direct);
  result = alloc_branch(STBranchDirectDefender,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_d_defender_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const next = slices[si].u.pipe.next;
    *root = alloc_defense_root_slice(length,min_length);
    if (length==slack_length_direct+1)
    {
      pipe_link(*root,next);
      dealloc_slice(si);
    }
    else
    {
      pipe_set_successor(*root,next);

      slices[si].u.branch.length -= 2;
      if (min_length>=slack_length_direct+2)
        slices[si].u.branch.min_length -= 2;
    }
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a short solution after the defense played
 * in a slice
 * @param si identifies slice that just played the defense
 * @param n maximum number of half moves until end of branch
 */
static boolean has_short_solution(slice_index si, stip_length_type n)
{
  boolean result;
  stip_length_type n_min;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const parity = n%2;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  n -= 2;

  if (n+min_length>slack_length_direct+length)
    n_min = n-(length-min_length);
  else
    n_min = slack_length_direct-parity;

  result = attack_has_solution_in_n(next,n,n_min)<=n;

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
    result = attack_are_threats_refuted_in_n(threats,len_threat,next,n);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a variation. The defense that starts the variation has
 * already been played in the current ply.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index
 * @param n maximum number of half moves until end state is to be reached
 */
static void write_existing_variation(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n_next = n-1;
  stip_length_type const parity = (n_next-slack_length_direct)%2;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_min = slack_length_direct-parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  write_defense();

  if (n_next+min_length>n_min+length)
    n_min = n_next-(length-min_length);

  attack_solve_continuations_in_n(next,n_next,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve variations after an attacker's move
 * @param threats table containing the threats after the attacker's move
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >=1 variation was found
 */
static void solve_variations_in_n(table threats,
                                  stip_length_type len_threat,
                                  slice_index si,
                                  stip_length_type n)
{
  Side const defender = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && is_defense_relevant(threats,len_threat,si,n-1))
      write_existing_variation(si,n);

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
static stip_length_type solve_threats_in_n(table threats,
                                           slice_index si,
                                           stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const parity = (n-slack_length_direct)%2;
  stip_length_type n_min = slack_length_direct-parity;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n+min_length>n_min+length)
    n_min = n-(length-min_length);

  output_start_threat_level();
  result = attack_solve_threats_in_n(threats,next,n,n_min);
  output_end_threat_level();

  if (result==n+2)
    Message(Zugzwang);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
static void solve_postkey_in_n(slice_index si, stip_length_type n)
{
  table const threats = allocate_table();
  stip_length_type len_threat;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  output_start_postkey_level();

  len_threat = OptFlag[nothreat] ? n+1 : solve_threats_in_n(threats,si,n-1);
  solve_variations_in_n(threats,len_threat,si,n);

  output_end_postkey_level();

  free_table();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean branch_d_defender_defend_in_n(slice_index si, stip_length_type n)
{
  Side const defender = slices[si].starter;
  boolean defender_is_immobile = true;
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n-1+min_length>slack_length_direct+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_direct-parity;

  TraceValue("%u\n",n_min_next);

  active_slice[nbply+1] = si;
  move_generation_mode =
      n-1>slack_length_direct
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const length_sol = attack_has_solution_in_n(next,
                                                                   n-1,
                                                                   n_min_next);
      if (length_sol<n_min_next)
        ; /* defense is illegal, e.g. self check */
      else
      {
        defender_is_immobile = false;
        if (length_sol>=n)
        {
          result = true;
          coupfort();
        }
      }
    }

    repcoup();
  }

  finply();

  TraceValue("%u\n",defender_is_immobile);
  if (defender_is_immobile)
    result = true;
  else if (!result)
  {
    write_attack(attack_regular);
    solve_postkey_in_n(si,n);
    coupfort();
  }
  
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
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

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
    {
      stip_length_type const length_sol = attack_has_solution_in_n(next,
                                                                   n-1,
                                                                   n_min_next);
      if (length_sol>n-1)
      {
        ++result;
        coupfort();
        isDefenderImmobile = false;
      }
      else if (length_sol>=n_min_next)
        isDefenderImmobile = false;
      else
      {
        /* self check */
      }
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
