#include "pyselfgd.h"
#include "pybrafrk.h"
#include "pybrah.h"
#include "pyhelp.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Initialise a STSelfAttack slice
 * @param si identifies slice to be initialised
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 */
static void init_self_attack_slice(slice_index si,
                                   stip_length_type length,
                                   stip_length_type min_length,
                                   slice_index next,
                                   slice_index towards_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slices[si].type = STSelfAttack; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.branch.length = length;
  slices[si].u.pipe.u.branch.min_length = min_length;
  slices[si].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a STSelfDefense slice
 * @param si identifies slice to be initialised
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 */
static void init_self_defense_slice(slice_index si,
                                    stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index towards_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slices[si].type = STSelfDefense; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.branch.length = length;
  slices[si].u.pipe.u.branch.min_length = min_length;
  slices[si].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate and initialise a STSelfAttack slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_self_attack_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index next,
                                    slice_index towards_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  result = alloc_slice_index();
  init_self_attack_slice(result,length,min_length,next,towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
self_defense_direct_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      unsigned int curr_max_nr_nontrivial)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;
  stip_length_type const
      max_n_for_goal = (slices[si].u.pipe.u.branch.length
                        -slices[si].u.pipe.u.branch.min_length
                        +slack_length_direct);
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",max_n_for_goal);
  
  if (n<max_n_for_goal && slice_has_non_starter_solved(towards_goal))
    result = has_solution;
  else if (n>slack_length_direct)
    result = direct_has_solution_in_n(next,n,curr_max_nr_nontrivial);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean self_defense_are_threats_refuted_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n,
                                              unsigned int curr_max_nr_nontrivial)
{
  boolean result = true;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type const max_n_for_goal = length-min_length+slack_length_direct;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",max_n_for_goal);

  if (n<max_n_for_goal && slice_has_non_starter_solved(towards_goal))
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

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 */
void self_defense_direct_solve_continuations_in_n(table continuations,
                                                  slice_index si,
                                                  stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(continuations,next,n);

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
stip_length_type self_defense_direct_solve_threats(table threats,
                                                   slice_index si,
                                                   stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = direct_solve_threats(threats,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return success of key move
 */
attack_result_type
self_attack_defend_in_n(slice_index si,
                        stip_length_type n,
                        unsigned int curr_max_nr_nontrivial)
{
  boolean result = attack_has_reached_deadend;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type const n_max_for_goal
      = length-min_length+slack_length_direct;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (n<=n_max_for_goal)
    switch (slice_has_starter_won(to_goal))
    {
      case starter_has_not_won:
        if (n>slack_length_direct)
          result = direct_defender_defend_in_n(next,n,curr_max_nr_nontrivial);
        break;

      case starter_has_won:
        result = attack_has_solved_next_branch;
        write_attack(attack_regular);
        slice_solve_postkey(to_goal);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_defend_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a defense after an attempted key move at
 * non-root level 
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean self_attack_can_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    unsigned int curr_max_nr_nontrivial)
{
  boolean result = true;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type const n_max_for_goal
      = length-min_length+slack_length_direct;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (n<=n_max_for_goal)
    switch (slice_has_starter_won(to_goal))
    {
      case starter_has_not_won:
        if (n>slack_length_direct)
          result = direct_defender_can_defend_in_n(next,
                                                   n,
                                                   curr_max_nr_nontrivial);
        break;

      case starter_has_won:
        result = false;
        break;

      default:
        assert(0);
        break;
    }
  else
    result = direct_defender_can_defend_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type self_attack_solve_threats(table threats,
                                           slice_index si,
                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = direct_defender_solve_threats(threats,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean self_attack_solve_variations_in_n(table threats,
                                          stip_length_type len_threat,
                                          slice_index si,
                                          stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = direct_defender_solve_variations_in_n(threats,len_threat,next,n);

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
void self_attack_root_solve_variations(table threats,
                                       stip_length_type len_threat,
                                       table refutations,
                                       slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  direct_defender_root_solve_variations(threats,len_threat,
                                        refutations,
                                        slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index self_attack_root_reduce_to_postkey_play(slice_index si)
{
  slice_index result;
  slice_index const length = slices[si].u.pipe.u.branch.length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length==slack_length_direct)
  {
    /* we are reducing from s#1 to s#0.5 */
    result = towards_goal;
    dealloc_slice_index(si);
  }
  else
  {
    result = slice_root_reduce_to_postkey_play(next);

    if (result!=no_slice)
      dealloc_slice_index(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean self_defense_solve(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const min_length = slices[si].u.pipe.u.branch.min_length;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* We arrive here e.g. when solving the set play of a sXN, after the
   * initial defender's "help move" has been played
   */
  if (min_length<slack_length_direct
      && slice_has_non_starter_solved(towards_goal))
  {
    slice_write_non_starter_has_solved(towards_goal);
    result = true;
  }
  else
    result = slice_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean self_guard_root_solve(slice_index si)
{
  boolean result = false;
  table refutations = allocate_table();
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* We only arrive here when solving a sX0.5
   */
  init_output(si);
  if (slice_root_solve_postkey(refutations,towards_goal))
  {
    write_end_of_solution();
    result = true;
  }

  free_table();

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
attack_result_type self_attack_root_defend(table refutations, slice_index si)
{
  attack_result_type result = attack_has_reached_deadend;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length==slack_length_direct)
    switch (slice_root_find_refutations(refutations,to_goal))
    {
      case found_no_refutation:
        result = attack_has_solved_next_branch;
        write_attack(attack_key);
        slice_root_solve_postkey(refutations,to_goal);
        write_end_of_solution();
        break;

      case found_refutations:
        result = attack_has_solved_next_branch;
        if (table_length(refutations)<=max_nr_refutations)
        {
          write_attack(attack_try);
          slice_root_solve_postkey(refutations,to_goal);
          write_refutations(refutations);
          write_end_of_solution();
        }
        break;

      default:
        break;
    }
  else if (min_length==slack_length_direct && slice_has_starter_won(to_goal))
  {
    result = attack_has_solved_next_branch;
    write_attack(attack_key);
    slice_solve_postkey(to_goal);
    write_end_of_solution();
  }
  else
    result = direct_defender_root_defend(refutations,next);
        
  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean self_attack_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean self_defense_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  traverse_slices(slices[si].u.pipe.next,st);

  *starter = advers(*starter);
  traverse_slices(slices[si].u.pipe.u.branch.towards_goal,st);
  *starter = advers(*starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STSelfDefense after each STDirectDefenderRoot slice if a
 * defender's move played in the STDirectDefenderRoot slice is allowed
 * to solve the following branch (as typical in a non-exact stipulation).
 */
static
boolean self_guards_inserter_branch_direct_defender(slice_index si,
                                                    slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_insert_after(si);
  init_self_defense_slice(slices[si].u.pipe.next,
                          length-1,min_length-1,
                          *towards_goal);

  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfAttack after each STBranchDirect slice if play is
 * allowed to continue in the following branch after moves played in
 * the STBranchDirect slice
 */
static boolean self_guards_inserter_branch_direct(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_insert_after(si);
  init_self_attack_slice(slices[si].u.pipe.next,
                         length-1,min_length-1,
                         slices[slices[si].u.pipe.next].u.pipe.next,
                         *towards_goal);

  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const self_guards_inserters[] =
{
  &self_guards_inserter_branch_direct,          /* STBranchDirect */
  &self_guards_inserter_branch_direct_defender, /* STBranchDirectDefender */
  &slice_traverse_children,                     /* STBranchHelp */
  &slice_traverse_children,                     /* STBranchSeries */
  0,                                            /* STBranchFork */
  &slice_operation_noop,                        /* STLeafDirect */
  &slice_operation_noop,                        /* STLeafHelp */
  &slice_traverse_children,                     /* STLeafForced */
  &slice_traverse_children,                     /* STReciprocal */
  &slice_traverse_children,                     /* STQuodlibet */
  &slice_traverse_children,                     /* STNot */
  &slice_traverse_children,                     /* STMoveInverter */
  &self_guards_inserter_branch_direct,          /* STDirectRoot */
  &self_guards_inserter_branch_direct_defender, /* STDirectDefenderRoot */
  &slice_traverse_children,                     /* STDirectHashed */
  &slice_traverse_children,                     /* STHelpRoot */
  &slice_traverse_children,                     /* STHelpAdapter */
  &slice_traverse_children,                     /* STHelpHashed */
  &slice_traverse_children,                     /* STSeriesRoot */
  &slice_traverse_children,                     /* STSeriesAdapter */
  &slice_traverse_children,                     /* STSeriesHashed */
  0,                                            /* STSelfCheckGuard */
  0,                                            /* STDirectAttack */
  0,                                            /* STDirectDefense */
  0,                                            /* STReflexGuard */
  &slice_traverse_children,                     /* STSelfAttack */
  &slice_traverse_children,                     /* STSelfDefense */
  0,                                            /* STRestartGuard */
  0,                                            /* STGoalReachableGuard */
  0,                                            /* STKeepMatingGuard */
  0,                                            /* STMaxFlightsquares */
  0,                                            /* STMaxNrNonTrivial */
  0                                             /* STMaxThreatLength */
};

/* Instrument a branch with STSelfAttack and STSelfDefense slices
 * @param si root of branch to be instrumented
 * @param towards_goal identifies slice leading towards goal
 */
void slice_insert_self_guards(slice_index si, slice_index towards_goal)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  TraceStipulation();

  slice_traversal_init(&st,&self_guards_inserters,&towards_goal);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
