#include "pydirctg.h"
#include "pydirect.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pybrah.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Initialise a STDirectDefense slice
 * @param si identifies slice to be initialised
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 */
static void init_direct_defense_slice(slice_index si,
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

  slices[si].type = STDirectDefense; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.branch.length = length;
  slices[si].u.pipe.u.branch.min_length = min_length;
  slices[si].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type
direct_defense_direct_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min)
{
  stip_length_type result;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (n_min<=slack_length_direct && slice_has_solution(togoal)==has_solution)
    result = n_min;
  else if (n>slack_length_direct)
    result = direct_has_solution_in_n(next,n,n_min);
  else
    result = n+2;

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
boolean direct_defense_are_threats_refuted_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(len_threat>=slack_length_direct);

  if (len_threat==slack_length_direct)
    result = slice_are_threats_refuted(threats,togoal);
  else
    result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no continuation was found
 */
stip_length_type
direct_defense_direct_solve_continuations_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_min)
{
  stip_length_type result;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_direct);

  if (n_min<=slack_length_direct && slice_solve(togoal))
    result = slack_length_direct;
  else if (n>slack_length_direct)
    result = direct_solve_continuations_in_n(next,n,n_min);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type
direct_defense_direct_solve_threats_in_n(table threats,
                                         slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_direct);
  
  if (n_min<=slack_length_direct)
  {
    slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
    slice_solve_threats(threats,togoal);
    if (table_length(threats)>0)
      result = slack_length_direct;
    else if (n>slack_length_direct)
      result = direct_solve_threats_in_n(threats,next,n,n_min);
  }
  else
    result = direct_solve_threats_in_n(threats,next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Slice **********
 */

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_defense_root_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length<=slack_length_direct && slice_root_solve(to_goal))
    result = true;

  if (next!=no_slice)
    /* always evaluate slice_root_solve(next), even if we have found a
     * short solution */
    result = direct_root_solve_in_n(next) || result;

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
stip_length_type direct_defense_solve_in_n(slice_index si,
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

  if (n_min<=slack_length_direct && slice_solve(towards_goal))
    result = n_min;
  else if (n>slack_length_direct)
    result = direct_solve_in_n(next,n,n_min);
  else
    result = n+2;

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
slice_index direct_defense_root_reduce_to_postkey_play(slice_index si)
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

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index direct_defense_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_root_make_setplay_slice(next);

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
boolean direct_defense_impose_starter(slice_index si, slice_traversal *st)
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


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STDirectDefense before each STDirectRoot slice
 */
static boolean direct_guards_inserter_branch_direct_root(slice_index si,
                                                         slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(si);
  init_direct_defense_slice(si,length,min_length,*towards_goal);

  si = slices[si].u.pipe.next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STDirectDefense after each STDirectDefenderRoot slice if
 * play is allowed to continue in the following branch after moves
 * played in the STDirectDefenderRoot slice (as typical in a non-exact
 * stipulation).
 */
static
boolean direct_guards_inserter_branch_direct_defender(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_after(si);
  init_direct_defense_slice(slices[si].u.pipe.next,
                            length-1,min_length-1,
                            *towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const direct_guards_inserters[] =
{
  &slice_traverse_children,                       /* STBranchDirect */
  &direct_guards_inserter_branch_direct_defender, /* STBranchDirectDefender */
  &slice_traverse_children,                       /* STBranchHelp */
  &slice_traverse_children,                       /* STBranchSeries */
  0,                                              /* STBranchFork */
  &slice_operation_noop,                          /* STLeafDirect */
  &slice_operation_noop,                          /* STLeafHelp */
  &slice_operation_noop,                          /* STLeafForced */
  &slice_traverse_children,                       /* STReciprocal */
  &slice_traverse_children,                       /* STQuodlibet */
  &slice_traverse_children,                       /* STNot */
  &slice_traverse_children,                       /* STMoveInverter */
  &direct_guards_inserter_branch_direct_root,     /* STDirectRoot */
  &direct_guards_inserter_branch_direct_defender, /* STDirectDefenderRoot */
  &slice_traverse_children,                       /* STDirectHashed */
  &slice_traverse_children,                       /* STHelpRoot */
  &slice_traverse_children,                       /* STHelpAdapter */
  &slice_traverse_children,                       /* STHelpHashed */
  &slice_traverse_children,                       /* STSeriesRoot */
  &slice_traverse_children,                       /* STSeriesAdapter */
  &slice_traverse_children,                       /* STSeriesHashed */
  0,                                              /* STSelfCheckGuard */
  0,                                              /* STDirectDefense */
  0,                                              /* STReflexGuard */
  0,                                              /* STSelfAttack */
  0,                                              /* STSelfDefense */
  0,                                              /* STRestartGuard */
  0,                                              /* STGoalReachableGuard */
  0,                                              /* STKeepMatingGuard */
  0,                                              /* STMaxFlightsquares */
  0,                                              /* STDegenerateTree */
  0,                                              /* STMaxNrNonTrivial */
  0                                               /* STMaxThreatLength */
};

/* Instrument a branch with STDirectDefense slices
 * @param si root of branch to be instrumented
 * @param towards_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index towards_goal)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&direct_guards_inserters,&towards_goal);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
