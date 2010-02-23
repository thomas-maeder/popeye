#include "stipulation/battle_play/defense_root.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/fork.h"
#include "pyoutput.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseRoot defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_root_slice(stip_length_type length,
                                     stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_battle)
    min_length += 2;
  assert(min_length>=slack_length_battle);
  result = alloc_branch(STDefenseRoot,length,min_length);

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

  if (n+min_length>slack_length_battle+length)
    n_min = n-(length-min_length);
  else
    n_min = slack_length_battle-parity;

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

  if (n>slack_length_battle && OptFlag[noshort] && has_short_solution(si,n))
    /* variation shorter than stip */
    result = false;
  else if (len_threat>slack_length_battle+1
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
  stip_length_type const parity = (n_next-slack_length_battle)%2;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_min = slack_length_battle-parity;

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

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
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
  stip_length_type const parity = (n-slack_length_battle)%2;
  stip_length_type n_min = slack_length_battle-parity;
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

/* Write a variation. The defense that starts the variation has
 * already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index
 * @param n maximum number of half moves until end state is to be reached
 * @return true iff variation is solvable
 */
static boolean write_possible_variation(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n_next = n-1;
  stip_length_type const parity = (n_next-slack_length_battle)%2;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_min = slack_length_battle-parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  write_defense();

  if (n_next+min_length>n_min+length)
    n_min = n_next-(length-min_length);

  result = attack_solve_in_n(next,n_next,n_min)<=n_next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
static void root_solve_variations(table threats,
                                  stip_length_type len_threat,
                                  table refutations,
                                  slice_index si)
{
  stip_length_type const length = slices[si].u.branch.length;
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
      write_existing_variation(si,length);

    repcoup();
  }

  finply();

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
static
boolean root_solve_postkey_mode_variations_in_n(table threats,
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

  assert(n%2==slices[si].u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && is_defense_relevant(threats,len_threat,si,n-1))
    {
      if (write_possible_variation(si,n))
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

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 solution was found
 */
static boolean root_solve_postkey_mode_in_n(slice_index si, stip_length_type n)
{
  table const threats = allocate_table();
  stip_length_type len_threat;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  output_start_postkey_level();

  if (OptFlag[nothreat])
    len_threat = n+1;
  else
  {
    Message(NewLine);
    len_threat = solve_threats_in_n(threats,si,n-1);
  }

  result = root_solve_postkey_mode_variations_in_n(threats,len_threat,si,n);

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
boolean defense_root_root_solve(slice_index si)
{
  stip_length_type const length = slices[si].u.branch.length;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  if (root_solve_postkey_mode_in_n(si,length))
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
 * @param max_number_refutations maximum number of refutations to deliver
 * @return true if defender is immobile or has too many refutations
 */
static boolean root_collect_refutations(table refutations,
                                        slice_index si,
                                        stip_length_type n,
                                        unsigned int max_number_refutations)
{
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = true;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_min_next;
  stip_length_type const parity = (n-1)%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_number_refutations);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  if (n-1+min_length>slack_length_battle+length)
    n_min_next = n-1-(length-min_length);
  else
    n_min_next = slack_length_battle-parity;

  active_slice[nbply+1] = si;
  if (n-1>slack_length_battle+2)
    move_generation_mode= move_generation_mode_opti_per_side[defender];
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const length_sol = attack_has_solution_in_n(next,
                                                                   n-1,
                                                                   n_min_next);
      if (length_sol>n-1)
      {
        result = false;
        append_to_top_table();
        coupfort();
      }
      else if (length_sol>=n_min_next)
        result = false;
      else
      {
        /* self check */
      }
    }

    repcoup();

    if (table_length(refutations)>max_number_refutations)
    {
      clear_top_table();
      result = true;
      break;
    }
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write the postkey play at root level (if the user hasn't opted it
 * out)
 * @param si slice index
 * @param refutations table containing refutations
 */
static void root_write_postkey(slice_index si, table refutations)
{
  output_start_postkey_level();

  if (OptFlag[solvariantes])
  {
    stip_length_type const length = slices[si].u.branch.length;
    table const threats = allocate_table();
    stip_length_type const len_threat = (OptFlag[nothreat]
                                         ? length
                                         : solve_threats_in_n(threats,
                                                              si,
                                                              length-1));
    root_solve_variations(threats,len_threat,refutations,si);
    free_table();
  }

  output_end_postkey_level();
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean defense_root_root_defend(slice_index si)
{
  table const refutations = allocate_table();
  stip_length_type const length = slices[si].u.branch.length;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (root_collect_refutations(refutations,si,length,max_nr_refutations))
    result = true;
  else if (table_length(refutations)>0)
  {
    result = true;
    write_attack(attack_try);
    root_write_postkey(si,refutations);
    write_refutations(refutations);
    write_end_of_solution();
  }
  else
  {
    result = false;
    write_attack(attack_key);
    root_write_postkey(si,refutations);
    write_end_of_solution();
  }

  free_table();

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean defense_root_detect_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();
  
  if (slices[si].starter==no_side)
    slices[si].starter = Black;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean defense_root_make_setplay_slice(slice_index si,
                                        struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (prod->sibling!=no_slice)
  {
    slice_index const branch = alloc_help_move_slice(slack_length_help+1,
                                                    slack_length_help+1);
    slice_index const fork = alloc_help_fork_slice(slack_length_help,
                                                   slack_length_help,
                                                   slices[si].u.pipe.next);
    pipe_link(branch,fork);
    prod->setplay_slice = alloc_help_root_slice(slack_length_help+1,
                                                slack_length_help+1,
                                                branch,
                                                no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean defense_root_reduce_to_postkey_play(slice_index si,
                                            struct slice_traversal *st)
{
  boolean const result = true;
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
