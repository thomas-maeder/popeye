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
stip_length_type self_defense_direct_has_solution_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n_min>=slack_length_direct-1);

  if (n_min==slack_length_direct-1
      && slice_has_non_starter_solved(towards_goal))
    result = n_min;
  else if (n>slack_length_direct)
    result = direct_has_solution_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean self_defense_are_threats_refuted_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n)
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
  TraceFunctionParamListEnd();

  TraceValue("%u\n",max_n_for_goal);

  if (n<max_n_for_goal && slice_has_non_starter_solved(towards_goal))
    result = false;
  else
    result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void self_defense_direct_solve_continuations_in_n(slice_index si,
                                                  stip_length_type n,
                                                  stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  direct_solve_continuations_in_n(next,n,n_min);

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
stip_length_type self_defense_direct_solve_threats_in_n(table threats,
                                                        slice_index si,
                                                        stip_length_type n,
                                                        stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = direct_solve_threats_in_n(threats,next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean self_attack_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type const n_max_for_goal
      = length-min_length+slack_length_direct;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n<=n_max_for_goal && !slice_defend(to_goal))
    result = false;
  else if (next!=no_slice)
    result = direct_defender_defend_in_n(next,n);
  else
    result = true;
  
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
unsigned int self_attack_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         unsigned int max_result)
{
  unsigned int result = max_result+1;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==slack_length_direct)
    result = slice_count_refutations(to_goal,max_result);
  else
    result = direct_defender_can_defend_in_n(next,n,max_result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Slice ***************
 */

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index self_attack_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index const length = slices[si].u.pipe.u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length==slack_length_help)
    result = slices[si].u.pipe.u.branch.towards_goal;
  else
    result = slice_root_make_setplay_slice(slices[si].u.pipe.next);

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
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type self_defense_solve_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  /* We arrive here e.g. when solving the set play of a sXN, after the
   * initial defender's "help move" has been played
   */
  if (n_min<slack_length_direct
      && slice_solved(towards_goal))
    result = n_min;
  else
    result = direct_solve_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean self_attack_root_defend(slice_index si)
{
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length==slack_length_direct)
  {
    if (slice_root_find_refutations(to_goal,max_nr_refutations)
        <=slack_length_direct+2)
      result = false;
    else
      result = true;
  }
  else
  {
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
    if (min_length==slack_length_direct
        && slice_root_find_refutations(to_goal,0)==slack_length_direct)
      result = false;
    else
      result = direct_defender_root_defend(slices[si].u.pipe.next);
  }
        
  TraceFunctionExit(__func__);
  TraceValue("%u",result);
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

/* Insert a STSelfAttack before anda STSelfDefense after each
 * STDirectRoot
 */
static boolean self_guards_inserter_direct_root(slice_index si,
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
  &self_guards_inserter_direct_root,            /* STDirectRoot */
  &self_guards_inserter_branch_direct_defender, /* STDirectDefenderRoot */
  &slice_traverse_children,                     /* STDirectHashed */
  &slice_traverse_children,                     /* STHelpRoot */
  &slice_traverse_children,                     /* STHelpHashed */
  &slice_traverse_children,                     /* STSeriesRoot */
  &slice_traverse_children,                     /* STSeriesHashed */
  0,                                            /* STSelfCheckGuard */
  0,                                            /* STDirectDefense */
  0,                                            /* STReflexGuard */
  &slice_traverse_children,                     /* STSelfAttack */
  &slice_traverse_children,                     /* STSelfDefense */
  0,                                            /* STRestartGuard */
  0,                                            /* STGoalReachableGuard */
  0,                                            /* STKeepMatingGuard */
  0,                                            /* STMaxFlightsquares */
  0,                                            /* STDegenerateTree */
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
