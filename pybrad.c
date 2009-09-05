#include "pybrad.h"
#include "pydirect.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybrafrk.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pynontrv.h"
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
 * @param towards_goal identifies first slice beyond branch
 * @return index of allocated slice
 */
static slice_index alloc_branch_d_slice(stip_length_type length,
                                        stip_length_type min_length,
                                        slice_index defender,
                                        slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",defender);
  TraceFunctionParamListEnd();

  assert(length>slack_length_direct);
  assert(min_length>=slack_length_direct-1);
  assert((length%2)==(min_length%2));

  slices[result].type = STBranchDirect; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = defender;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STBranchDirect slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies first slice beyond branch
 * @return index of allocated slice
 */
static slice_index alloc_direct_root_branch(stip_length_type length,
                                            stip_length_type min_length,
                                            slice_index defender,
                                            slice_index towards_goal)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",defender);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>=slack_length_direct+1);
  assert(min_length>=slack_length_direct);
  assert((length%2)==(min_length%2));

  slices[result].type = STDirectRoot; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.next = defender;
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_index alloc_toplevel_direct_branch(stip_length_type length,
                                                stip_length_type min_length,
                                                slice_index towards_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_direct);
  assert(min_length>=slack_length_direct);

  if (length%2==0)
  {
    if (length-slack_length_direct==2)
    {
      stip_length_type const defender_root_minlength
          = (min_length-slack_length_direct<2
             ? slack_length_direct+1
             : min_length-1);
      slice_index const defender_root
          = alloc_branch_d_defender_root_slice(length-1,
                                               defender_root_minlength,
                                               no_slice,
                                               towards_goal);
      result = alloc_direct_root_branch(length,min_length,
                                        defender_root,towards_goal);
    }
    else
    {
      stip_length_type defender_min_length
          = (min_length-slack_length_direct<4
             ? slack_length_direct+1
             : min_length-3);
      slice_index const defender
          = alloc_branch_d_defender_slice(length-3,defender_min_length,
                                          no_slice,towards_goal);
      stip_length_type attacker_min_length
          = (min_length-slack_length_direct<2
             ? slack_length_direct
             : min_length-2);
      slice_index const branch_d = alloc_branch_d_slice(length-2,
                                                        attacker_min_length,
                                                        defender,
                                                        towards_goal);
      slice_index const defender_root
          = alloc_branch_d_defender_root_slice(length-1,
                                               attacker_min_length+1,
                                               branch_d,
                                               towards_goal);
      slices[defender].u.pipe.next = branch_d;
      result = alloc_direct_root_branch(length,min_length,
                                        defender_root,towards_goal);
    }
  }
  else
  {
    if (length-slack_length_direct==1)
      result = alloc_direct_root_branch(length,min_length,
                                        no_slice,towards_goal);
    else if (length-slack_length_direct==3)
    {
      stip_length_type const branch_min_length
          = (min_length-slack_length_direct<3
             ? slack_length_direct+1
             : min_length-2);
      slice_index const branch_d = alloc_branch_d_slice(length-2,
                                                        branch_min_length,
                                                        no_slice,
                                                        towards_goal);
      slice_index const defender_root
          = alloc_branch_d_defender_root_slice(length-1,min_length-1,
                                               branch_d,towards_goal);

      result = alloc_direct_root_branch(length,min_length,
                                        defender_root,towards_goal);
    }
    else
    {
      stip_length_type const defender_min_length
          = (min_length-slack_length_direct<3
             ? slack_length_direct
             : min_length-3);
      slice_index const
          defender = alloc_branch_d_defender_slice(length-3,
                                                   defender_min_length,
                                                   no_slice,
                                                   towards_goal);
      stip_length_type const attacker_min_length
          = (min_length-slack_length_direct<3
             ? slack_length_direct+1
             : min_length-2);
      slice_index const branch_d
          = alloc_branch_d_slice(length-2,attacker_min_length,
                                 defender,towards_goal);

      slice_index const defender_root
          = alloc_branch_d_defender_root_slice(length-1,min_length-1,
                                               branch_d,towards_goal);
      slices[defender].u.pipe.next = branch_d;
      result = alloc_direct_root_branch(length,min_length,
                                        defender_root,towards_goal);
    }
  }

     
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a nested (i.e. non toplevel) direct branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 * @return identifier for entry slice of allocated branch
 */
slice_index alloc_nested_direct_branch(stip_length_type length,
                                       stip_length_type min_length,
                                       slice_index towards_goal)
{
  slice_index result;
  slice_index defender;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(min_length>=slack_length_direct);
  assert(min_length%2==length%2);

  /* TODO direct initialisation of nested branch */
  result = alloc_toplevel_direct_branch(length,min_length,towards_goal);
  slices[result].type = STBranchDirect;

  defender = branch_find_slice(STDirectDefenderRoot,result);
  if (defender!=no_slice)
    slices[defender].type = STBranchDirectDefender;

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
 * @param towards_goal identifies slice leading towards goal
 * @return index of adapter slice
 */
slice_index alloc_direct_branch(branch_level level,
                                stip_length_type length,
                                stip_length_type min_length,
                                slice_index towards_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",level);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  assert(length>slack_length_direct);

  if (level==toplevel_branch)
    result = alloc_toplevel_direct_branch(length,min_length,towards_goal);
  else
    result = alloc_nested_direct_branch(length,min_length,towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal (after the defense)
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_d_are_threats_refuted_in_n(table threats,
                                          stip_length_type len_threat,
                                          slice_index si,
                                          stip_length_type n,
                                          unsigned int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;

    active_slice[nbply+1] = si;
    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats))
      {
        if (direct_defender_can_defend_in_n(next,
                                            len_threat-1,
                                            curr_max_nr_nontrivial))
          defense_found = true;
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

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si,
                                     stip_length_type n,
                                     unsigned int curr_max_nr_nontrivial)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (!solution_found && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_can_defend_in_n(next,n-1,curr_max_nr_nontrivial))
    {
      solution_found = true;
      coupfort();
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
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff attacker can end in n half moves
 */
static boolean have_we_solution_in_n_short(slice_index si,
                                           stip_length_type n,
                                           unsigned int curr_max_nr_nontrivial)
{
  boolean result = false;

  stip_length_type i;
  stip_length_type n_max = n-2;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type n_min = min_length;
  stip_length_type const parity = n%2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(slices[si].u.pipe.u.branch.length%2 == parity);

  if (n_min<=slack_length_direct)
    n_min += 2;

  if (n_max>=min_length_nontrivial)
    n_max = min_length_nontrivial-parity;

  TraceValue("%u",min_length);
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
 * @param n maximum number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type branch_d_has_solution_in_n(slice_index si,
                                             stip_length_type n,
                                             unsigned int curr_max_nr_nontrivial)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (have_we_solution_in_n_short(si,n,curr_max_nr_nontrivial))
    result = has_solution;
  else if (periods_counter<nr_periods
           && have_we_solution_in_n(si,n,curr_max_nr_nontrivial))
    result = has_solution;
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type branch_d_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = branch_d_has_solution_in_n(si,
                                      slices[si].u.pipe.u.branch.length,
                                      max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param attacker attacking side
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
void branch_d_solve_continuations_in_n(table continuations,
                                       slice_index si,
                                       stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !direct_defender_defend_in_n(next,n-1,max_nr_nontrivial))
    {
      append_to_top_table();
      coupfort();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type branch_d_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n)
{
  unsigned int const parity = (n-slack_length_direct)%2;
  stip_length_type result = slack_length_direct+parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  output_start_threat_level();

  while (result<=n)
  {
    branch_d_solve_continuations_in_n(threats,si,result);
    if (table_length(threats)>0)
      break;
    else
      result += 2;
  }

  output_end_threat_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/**************** slice interface ***********************/

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void branch_d_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.pipe.u.branch.towards_goal);
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return whether the starter has won
 */
has_starter_won_result_type branch_d_has_starter_won(slice_index si)
{
  return slice_has_starter_won(slices[si].u.pipe.u.branch.towards_goal);
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

  result = slice_is_goal_reached(just_moved,
                                 slices[si].u.pipe.u.branch.towards_goal);

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
boolean branch_d_has_non_starter_solved(slice_index si)
{
  return slice_has_non_starter_solved(slices[si].u.pipe.u.branch.towards_goal);
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_d_has_starter_reached_goal(slice_index si)
{
  return slice_has_starter_reached_goal(slices[si].u.pipe.u.branch.towards_goal);
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
                                    slices[si].u.pipe.next,
                                    slices[si].u.pipe.u.branch.length);

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

  slice_root_write_key(slices[si].u.pipe.u.branch.towards_goal,type);

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
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  stip_length_type i;
  table const continuations = allocate_table();
  stip_length_type const min_len = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_continuation_level();
  
  for (i = min_len; i<=n && !result; i += 2)
  {
    direct_solve_continuations_in_n(continuations,si,i);
    result = table_length(continuations)>0;
  }

  output_end_continuation_level();

  free_table();

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

/****************** root ************************/

/* Solve at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_root_solve(slice_index si)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  solutions = 0;

  active_slice[nbply+1] = si;
  genmove(attacker);

  output_start_continuation_level();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      table const refutations = allocate_table();
      switch (direct_defender_root_defend(refutations,next))
      {
        case attack_has_full_length_play:
          if (table_length(refutations)<=max_nr_refutations)
          {
            attack_type const type = (table_length(refutations)==0
                                      ? attack_key
                                      : attack_try);
            write_attack(type);

            output_start_postkey_level();

            if (OptFlag[solvariantes])
              direct_defender_root_solve_postkey(refutations,next);

            output_end_postkey_level();

            if (table_length(refutations)==0)
              result = true;
            else
              write_refutations(refutations);

            write_end_of_solution();
          }
          break;

        case attack_has_solved_next_branch:
          result = true;
          break;

        default:
          break;
      }

      free_table();
    }

    repcoup();

    if (OptFlag[maxsols] && solutions>=maxsolutions)
    {
      /* signal maximal number of solutions reached to outer world */
      FlagMaxSolsReached = true;
      break;
    }

    if (periods_counter>=nr_periods)
      break;
  }

  output_end_continuation_level();

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index direct_root_make_setplay_slice(slice_index si)
{
  slice_index defender_root;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  defender_root = branch_find_slice(STDirectDefenderRoot,si);
  if (defender_root==no_slice)
  {
    /* we end up here when making the setplay slice of a r#1 or s#1 */
    slice_index const selfcheck_guard = branch_find_slice(STSelfCheckGuard,si);
    assert(STSelfCheckGuard!=no_slice);
    result = slices[selfcheck_guard].u.pipe.next;
  }
  else
  {
    result = alloc_help_branch(toplevel_branch,
                               slack_length_help+1,slack_length_help+1,
                               slices[defender_root].u.pipe.next);
    slices[result].starter = advers(slices[si].starter);
  }

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
slice_index direct_root_reduce_to_postkey_play(slice_index si)
{
  slice_index result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_root_reduce_to_postkey_play(next);

  if (result!=no_slice)
    dealloc_slice_index(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
