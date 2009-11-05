#include "pybrad.h"
#include "pydirect.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybrafrk.h"
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
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_d_are_threats_refuted_in_n(table threats,
                                          stip_length_type len_threat,
                                          slice_index si,
                                          stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int const nr_refutations_allowed = 0;
  boolean result = true;
  unsigned int nr_successful_threats = 0;
  boolean defense_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (encore() && !defense_found)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && is_current_move_in_table(threats))
    {
      if (direct_defender_can_defend_in_n(next,
                                          len_threat-1,
                                          nr_refutations_allowed)
          >nr_refutations_allowed)
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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n maximum number of half moves until goal
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si, stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean solution_found = false;
  unsigned int const nr_refutations_allowed = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  active_slice[nbply+1] = si;
  genmove(attacker);

  while (!solution_found && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && (direct_defender_can_defend_in_n(next,n-1,nr_refutations_allowed)
            <=nr_refutations_allowed))
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

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type branch_d_has_solution_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();
  
  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (n_min<=slack_length_direct)
    n_min += 2;

  for (result = n_min; result<=n; result += 2)
    if (have_we_solution_in_n(si,result))
      break;
    else if (periods_counter>=nr_periods)
    {
      result = n+2;
      break;
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
 */
static void solve_postkey_in_n(slice_index si, stip_length_type n)
{
  table const threats = allocate_table();
  stip_length_type len_threat;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  output_start_postkey_level();

  if (OptFlag[nothreat])
    len_threat = n;
  else
  {
    output_start_threat_level();
    len_threat = direct_defender_solve_threats_in_n(threats,next,n-2);
    output_end_threat_level();
    if (len_threat==n)
      Message(Zugzwang);
  }

  direct_defender_solve_variations_in_n(threats,len_threat,next,n-1);

  output_end_postkey_level();

  free_table();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void branch_d_solve_continuations_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_min)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = branch_d_solve_in_n(si,n,n_min);
  assert(result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type branch_d_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_min)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n_min;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (result<=slack_length_direct)
    result += 2;

  active_slice[nbply+1] = si;

  while (result<=n)
  {
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
        switch (direct_defender_defend_in_n(next,result-1))
        {
          case attack_has_solved_next_branch:
            append_to_top_table();
            coupfort();
            break;

          case attack_solves_full_length:
            write_attack(attack_regular);
            solve_postkey_in_n(si,result);
            append_to_top_table();
            coupfort();
            break;

          default:
            break;
        }

      repcoup();
    }

    finply();

    if (table_length(threats)>0)
      break;
    else
      result += 2;
  }

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

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void branch_d_solve_threats(table threats, slice_index si)
{
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const parity = (length-slack_length_direct)%2;
  stip_length_type const n_min = slack_length_direct+2-parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  branch_d_solve_threats_in_n(threats,si,length,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type branch_d_solve_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n_min;
  boolean continuation_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  output_start_continuation_level();

  active_slice[nbply+1] = si;

  if (result<=slack_length_direct)
    result += 2;

  while (result<=n)
  {
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
        switch (direct_defender_defend_in_n(next,result-1))
        {
          case attack_has_solved_next_branch:
            /* We used to have
             * assert(result==slack_length_direct+1);
             * here, but in certain situations (e.g. if option
             * nontrivial is used), this condition doesn't necessrily
             * hold because we don't for each n from
             * slack_length_direct to full length.
             */
            continuation_found = true;
            coupfort();
            break;

          case attack_solves_full_length:
            continuation_found = true;
            write_attack(attack_regular);
            solve_postkey_in_n(si,result);
            coupfort();
            break;

          default:
            break;
        }

      repcoup();
    }

    finply();

    if (continuation_found)
      break;
    else
      result += 2;
  }

  output_end_continuation_level();

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
    slice_index const next = slices[si].u.pipe.next;
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    table const threats = allocate_table();
    stip_length_type len_threat;

    if (OptFlag[nothreat])
      len_threat = length;
    else
    {
      output_start_threat_level();
      len_threat = direct_defender_solve_threats_in_n(threats,next,length-2);
      output_end_threat_level();

      if (len_threat==length)
        Message(Zugzwang);
    }

    direct_defender_root_solve_variations(threats,len_threat,
                                          refutations,
                                          next);
    free_table();
  }

  output_end_postkey_level();
}

/* Solve at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_root_root_solve(slice_index si)
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
        case attack_refuted_full_length:
          if (table_length(refutations)<=max_nr_refutations)
          {
            write_attack(attack_try);
            root_write_postkey(si,refutations);
            write_refutations(refutations);
            write_end_of_solution();
          }
          break;

        case attack_solves_full_length:
          result = true;
          write_attack(attack_key);
          root_write_postkey(si,refutations);
          write_end_of_solution();
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
