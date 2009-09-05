#include "pyselfcg.h"
#include "pypipe.h"
#include "pydirect.h"
#include "pyhelp.h"
#include "pyseries.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Initialise a STSelfCheckGuard slice into an allocated and wired
 * pipe slice 
 * @param si identifies slice
 */
static void init_selfcheck_guard_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].type = STSelfCheckGuard;
  slices[si].starter = slices[slices[si].u.pipe.next].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STSelfCheckGuard slice
 * @param next identifies next slice in branch
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_slice(slice_index next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  result = alloc_slice_index();
  slices[result].starter = slices[next].starter;
  slices[result].type = STSelfCheckGuard;
  slices[result].u.pipe.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Direct ***************
 */

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
boolean selfcheck_guard_are_threats_refuted_in_n(table threats,
                                                 stip_length_type len_threat,
                                                 slice_index si,
                                                 stip_length_type n,
                                                 unsigned int curr_max_nr_nontrivial)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = false;
  else if (n>=slack_length_direct)
    result = direct_are_threats_refuted_in_n(threats,
                                             len_threat,
                                             slices[si].u.pipe.next,
                                             n,
                                             curr_max_nr_nontrivial);
  else
    result = true;

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
void selfcheck_guard_direct_solve_continuations_in_n(table continuations,
                                                     slice_index si,
                                                     stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

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
stip_length_type selfcheck_guard_direct_solve_threats_in_n(table threats,
                                                           slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = (n-slack_length_direct)%2;
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = direct_solve_threats_in_n(threats,next,n);
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
selfcheck_guard_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         unsigned int curr_max_nr_nontrivial)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = defender_self_check;
  else
    result = direct_has_solution_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return success of key move
 */
attack_result_type selfcheck_guard_root_defend(table refutations,
                                               slice_index si)
{
  attack_result_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = attack_has_reached_deadend;
  else
    result = direct_defender_root_defend(refutations,next);

  TraceFunctionExit(__func__);
  TraceEnumerator(attack_result_type,result,"");
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
stip_length_type selfcheck_guard_solve_threats(table threats,
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
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void selfcheck_guard_root_solve_variations(table threats,
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

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return success of key move
 */
attack_result_type
selfcheck_guard_defend_in_n(slice_index si,
                            stip_length_type n,
                            unsigned int curr_max_nr_nontrivial)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = attack_has_reached_deadend;
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
boolean selfcheck_guard_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int curr_max_nr_nontrivial)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = true;
  else
    result = direct_defender_can_defend_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Help ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean selfcheck_guard_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (!echecc(nbply,advers(slices[si].starter))
            && help_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean selfcheck_guard_help_has_solution_in_n(slice_index si,
                                               stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  result = (!echecc(nbply,advers(slices[si].starter))
            && help_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void selfcheck_guard_help_solve_continuations_in_n(table continuations,
                                                   slice_index si,
                                                   stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (!echecc(nbply,advers(slices[si].starter)))
    help_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean selfcheck_guard_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  result = (!echecc(nbply,advers(slices[si].starter))
            && series_solve_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean selfcheck_guard_series_has_solution_in_n(slice_index si,
                                                 stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  result = (!echecc(nbply,advers(slices[si].starter))
            && series_has_solution_in_n(slices[si].u.pipe.next,n));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void selfcheck_guard_series_solve_continuations_in_n(table continuations,
                                                     slice_index si,
                                                     stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (!echecc(nbply,advers(slices[si].starter)))
    series_solve_continuations_in_n(continuations,slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Slice ***************
 */

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean selfcheck_guard_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
  {
    ErrorMsg(KingCapture);
    result = false;
  }
  else
    result = slice_root_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at non-root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean selfcheck_guard_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!echecc(nbply,advers(slices[si].starter))
            && slice_solve(slices[si].u.pipe.next));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type selfcheck_guard_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = defender_self_check;
  else
    result = slice_has_solution(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean selfcheck_guard_are_threats_refuted(table threats, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = false;
  else
    result = slice_are_threats_refuted(threats,slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void selfcheck_guard_solve_continuations(table continuations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_solve_continuations(continuations,slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STSelfCheckGuard slice after a STBranch{Help,Series} slice
 */
static boolean selfcheck_guards_inserter_branch(slice_index si,
                                                slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* prevent double insertion if .next has more than one predecessor
   */
  assert(slices[slices[si].u.pipe.next].type!=STSelfCheckGuard);
  pipe_insert_after(si);
  init_selfcheck_guard_slice(slices[si].u.pipe.next);
  slice_traverse_children(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfCheckGuard slice after a STBranchDirect or
 * STDirectRoot slice
 */
static boolean selfcheck_guards_inserter_branch_direct(slice_index si,
                                                       slice_traversal *st)
{
  boolean const result = true;
  slice_index guard_pos = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",guard_pos);

  if (slices[guard_pos].type==STDirectAttack)
    /* in direct stipulations, the last attacker's move may be allowed
     * to expose its own king (e.g. in ##!) */
    guard_pos = slices[guard_pos].u.pipe.next;

  TraceValue("->%u\n",guard_pos);

  assert(guard_pos!=no_slice);
  if (guard_pos!=no_slice
      && slices[guard_pos].type!=STSelfCheckGuard)
  {
    pipe_insert_before(guard_pos);
    init_selfcheck_guard_slice(guard_pos);
    slice_traverse_children(guard_pos,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfCheckGuard slice after a STBranchDirectDefender or
 * STDirectDefenderRoot slice
 */
static
boolean selfcheck_guards_inserter_branch_direct_defender(slice_index si,
                                                         slice_traversal *st)
{
  boolean const result = true;
  slice_index guard_pos = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",guard_pos);

  if (slices[guard_pos].type==STSelfDefense)
    /* in self stipulations, the last defender's move may be allowed
     * to expose its own king (e.g. in s##!) */
    guard_pos = slices[guard_pos].u.pipe.next;

  TraceValue("->%u\n",guard_pos);

  if (slices[guard_pos].type!=STSelfCheckGuard)
  {
    pipe_insert_before(guard_pos);
    init_selfcheck_guard_slice(guard_pos);
    slice_traverse_children(guard_pos,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfCheckGuard slice after a STMoveInverter slice
 */
static boolean selfcheck_guards_inserter_move_inverter(slice_index si,
                                                       slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* prevent double insertion if .next has more than one predecessor
   */
  assert(slices[slices[si].u.pipe.next].type!=STSelfCheckGuard);
  pipe_insert_after(si);
  init_selfcheck_guard_slice(slices[si].u.pipe.next);
  slice_traverse_children(slices[si].u.pipe.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const selfcheck_guards_inserters[] =
{
  &selfcheck_guards_inserter_branch_direct, /* STBranchDirect */
  &selfcheck_guards_inserter_branch_direct_defender, /* STBranchDirectDefender */
  &selfcheck_guards_inserter_branch,        /* STBranchHelp */
  &selfcheck_guards_inserter_branch,        /* STBranchSeries */
  &slice_traverse_children,                 /* STBranchFork */
  &slice_operation_noop,                    /* STLeafDirect */
  &slice_operation_noop,                    /* STLeafHelp */
  &slice_operation_noop,                    /* STLeafForced */
  &slice_traverse_children,                 /* STReciprocal */
  &slice_traverse_children,                 /* STQuodlibet */
  &slice_traverse_children,                 /* STNot */
  &selfcheck_guards_inserter_move_inverter, /* STMoveInverter */
  &selfcheck_guards_inserter_branch_direct, /* STDirectRoot */
  &selfcheck_guards_inserter_branch_direct_defender, /* STDirectDefenderRoot */
  &slice_traverse_children,                 /* STDirectHashed */
  &selfcheck_guards_inserter_branch,        /* STHelpRoot */
  &slice_traverse_children,                 /* STHelpAdapter */
  &slice_traverse_children,                 /* STHelpHashed */
  &selfcheck_guards_inserter_branch,        /* STSeriesRoot */
  &slice_traverse_children,                 /* STSeriesAdapter */
  &slice_traverse_children,                 /* STSeriesHashed */
  &slice_operation_noop,                    /* STSelfCheckGuard */
  &slice_traverse_children,                 /* STDirectAttack */
  &slice_traverse_children,                 /* STDirectDefense */
  &slice_traverse_children,                 /* STReflexGuard */
  &slice_traverse_children,                 /* STSelfAttack */
  &slice_traverse_children,                 /* STSelfDefense */
  0,                                        /* STRestartGuard */
  &slice_traverse_children,                 /* STGoalReachableGuard */
  0,                                        /* STKeepMatingGuard */
  0,                                        /* STMaxFlightsquares */
  0,                                        /* STMaxNrNonTrivial */
  0                                         /* STMaxThreatLength */
};
/* element STSelfCheckGuard is not 0 because we may reach a
 * STSelfCheckGuard slice inserted early later on a different path
 */


/* Insert a STSelfCheckGuard at the beginning of a toplevel "leaf slice"
 */
static boolean selfcheck_guards_inserter_toplevel_leaf(slice_index si,
                                                       slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_index const new_leaf = copy_slice(si);
  slices[si].type = STSelfCheckGuard;
  slices[si].starter = slices[new_leaf].starter;
  slices[si].u.pipe.next = new_leaf;
  slices[si].u.pipe.u.branch.length = slack_length_direct;
  slices[si].u.pipe.u.branch.min_length = slack_length_direct;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STSelfCheckGuard at the beginning of a toplevel branch
 */
static boolean selfcheck_guards_inserter_toplevel_root(slice_index si,
                                                       slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_insert_before(si);
  init_selfcheck_guard_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
boolean selfcheck_guards_inserter_toplevel_reflex_guard(slice_index si,
                                                        slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_insert_before(si);
  init_selfcheck_guard_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const selfcheck_guards_toplevel_inserters[] =
{
  0,                                             /* STBranchDirect */
  0,                                             /* STBranchDirectDefender */
  0,                                             /* STBranchHelp */
  0,                                             /* STBranchSeries */
  0,                                             /* STBranchFork */
  &selfcheck_guards_inserter_toplevel_leaf,      /* STLeafDirect */
  &selfcheck_guards_inserter_toplevel_leaf,      /* STLeafHelp */
  &selfcheck_guards_inserter_toplevel_leaf,      /* STLeafForced */
  &slice_traverse_children,                      /* STReciprocal */
  &slice_traverse_children,                      /* STQuodlibet */
  &slice_traverse_children,                      /* STNot */
  &slice_operation_noop,                         /* STMoveInverter */
  &selfcheck_guards_inserter_toplevel_root,      /* STDirectRoot */
  0,                                             /* STDirectDefenderRoot */
  0,                                             /* STDirectHashed */
  &selfcheck_guards_inserter_toplevel_root,      /* STHelpRoot */
  0,                                             /* STHelpAdapter */
  0,                                             /* STHelpHashed */
  &selfcheck_guards_inserter_toplevel_root,      /* STSeriesRoot */
  0,                                             /* STSeriesAdapter */
  0,                                             /* STSeriesHashed */
  0,                                             /* STSelfCheckGuard */
  0,                                             /* STDirectAttack */
  0,                                             /* STDirectDefense */
  &selfcheck_guards_inserter_toplevel_reflex_guard, /* STReflexGuard */
  0,                                             /* STSelfAttack */
  0,                                             /* STSelfDefense */
  0,                                             /* STRestartGuard */
  0,                                             /* STGoalReachableGuard */
  0,                                             /* STKeepMatingGuard */
  0,                                             /* STMaxFlightsquares */
  0,                                             /* STMaxNrNonTrivial */
  0                                              /* STMaxThreatLength */
};

/* Instrument stipulation with STSelfCheckGuard slices
 */
void stip_insert_selfcheck_guards(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation();

  slice_traversal_init(&st,&selfcheck_guards_inserters,0);
  traverse_slices(root_slice,&st);

  TraceStipulation();

  slice_traversal_init(&st,&selfcheck_guards_toplevel_inserters,0);
  traverse_slices(root_slice,&st);

  TraceStipulation();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
