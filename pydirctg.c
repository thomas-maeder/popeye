#include "pydirctg.h"
#include "pydirect.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pybrah.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STDirectDefense slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_direct_defense(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STDirectDefense,length,min_length,proxy_to_goal);

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
boolean direct_defense_insert_root(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.pipe.next,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
    slice_index const direct_defense = alloc_direct_defense(length,min_length,
                                                            to_goal);
    pipe_link(direct_defense,*root);
    *root = direct_defense;

    slices[si].u.branch.length -= 2;
    if (min_length>=slack_length_direct+2)
      slices[si].u.branch.min_length -= 2;
  }
  
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
stip_length_type
direct_defense_direct_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min)
{
  stip_length_type result;
  slice_index const togoal = slices[si].u.branch_fork.towards_goal;
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
  slice_index const togoal = slices[si].u.branch_fork.towards_goal;

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

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void direct_defense_direct_solve_continuations_in_n(slice_index si,
                                                    stip_length_type n,
                                                    stip_length_type n_min)
{
  slice_index const togoal = slices[si].u.branch_fork.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_direct);

  if (n_min<=slack_length_direct && slice_solve(togoal))
    ;
  else if (n>slack_length_direct)
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
    slice_index const togoal = slices[si].u.branch_fork.towards_goal;
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
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

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
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;

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

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean direct_defense_root_make_setplay_slice(slice_index si,
                                               struct slice_traversal *st)
{
  boolean const result = true;
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  prod->sibling = si;

  if (slices[si].u.branch.length==slack_length_direct+1)
  {
    slice_index const proxy_to_goal = slices[si].u.branch_fork.towards_goal;
    assert(slices[proxy_to_goal].type==STProxy);
    prod->setplay_slice = slices[proxy_to_goal].u.pipe.next;
  }
  else
    traverse_slices(slices[si].u.pipe.next,st);

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
boolean direct_defense_root_reduce_to_postkey_play(slice_index si,
                                                   struct slice_traversal *st)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(next,st);

  if (*postkey_slice!=no_slice)
    dealloc_slice(si);

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

/* Insert a STDirectDefense slice before each STDirectRoot and
 * STBranchDirect slice
 * @param si identifies slice before which to insert a STDirectDefense
 *           slice
 * @param st address of structure representing the traversal
 */
static boolean direct_guards_inserter_attack(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  {
    slice_index const * const proxy_to_goal = st->param;
    slice_index const prev = slices[si].prev;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const dirdef = alloc_direct_defense(length,min_length,
                                                    *proxy_to_goal);
    pipe_link(prev,dirdef);
    pipe_link(dirdef,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STDirectDefense slice after the terminal defense (if any)
 * @param si identifies defense
 * @param st address of structure representing the traversal
 */
static boolean direct_guards_inserter_defense(slice_index si,
                                              slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  if (next==no_slice)
  {
    slice_index const * const proxy_to_goal = st->param;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const dirdef = alloc_direct_defense(length-1,min_length-1,
                                                    *proxy_to_goal);
    pipe_link(si,dirdef);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const direct_guards_inserters[] =
{
  &slice_traverse_children,       /* STProxy */
  &direct_guards_inserter_attack, /* STBranchDirect */
  &direct_guards_inserter_defense,       /* STBranchDirectDefender */
  &slice_traverse_children,       /* STBranchHelp */
  0,                              /* STBHelpFork */
  &slice_traverse_children,       /* STBranchSeries */
  0,                              /* STSeriesFork */
  &slice_operation_noop,          /* STLeafDirect */
  &slice_operation_noop,          /* STLeafHelp */
  &slice_operation_noop,          /* STLeafForced */
  &slice_traverse_children,       /* STReciprocal */
  &slice_traverse_children,       /* STQuodlibet */
  &slice_traverse_children,       /* STNot */
  &slice_traverse_children,       /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,       /* STMoveInverterSolvableFilter */
  &slice_traverse_children,       /* STMoveInverterSeriesFilter */
  0,                              /* STDirectRoot */
  &direct_guards_inserter_defense,       /* STDirectDefenderRoot */
  &slice_traverse_children,       /* STDirectHashed */
  &slice_traverse_children,       /* STHelpRoot */
  &slice_traverse_children,       /* STHelpShortcut */
  &slice_traverse_children,       /* STHelpHashed */
  &slice_traverse_children,       /* STSeriesRoot */
  &slice_traverse_children,       /* STSeriesShortcut */
  &slice_traverse_children,       /* STParryFork */
  &slice_traverse_children,       /* STSeriesHashed */
  0,                              /* STSelfCheckGuardRootSolvableFilter */
  0,                              /* STSelfCheckGuardSolvableFilter */
  0,                              /* STSelfCheckGuardRootDefenderFilter */
  0,                              /* STSelfCheckGuardAttackerFilter */
  0,                              /* STSelfCheckGuardDefenderFilter */
  0,                              /* STSelfCheckGuardHelpFilter */
  0,                              /* STSelfCheckGuardSeriesFilter */
  &slice_traverse_children,       /* STDirectDefense */
  0,                              /* STReflexHelpFilter */
  0,                              /* STReflexSeriesFilter */
  0,                              /* STReflexAttackerFilter */
  0,                              /* STReflexDefenderFilter */
  0,                              /* STSelfAttack */
  0,                              /* STSelfDefense */
  0,                              /* STRestartGuardRootDefenderFilter */
  0,                              /* STRestartGuardHelpFilter */
  0,                              /* STRestartGuardSeriesFilter */
  0,                              /* STIntelligentHelpFilter */
  0,                              /* STIntelligentSeriesFilter */
  0,                              /* STGoalReachableGuardHelpFilter */
  0,                              /* STGoalReachableGuardSeriesFilter */
  0,                              /* STKeepMatingGuardRootDefenderFilter */
  0,                              /* STKeepMatingGuardAttackerFilter */
  0,                              /* STKeepMatingGuardDefenderFilter */
  0,                              /* STKeepMatingGuardHelpFilter */
  0,                              /* STKeepMatingGuardSeriesFilter */
  0,                              /* STMaxFlightsquares */
  0,                              /* STDegenerateTree */
  0,                              /* STMaxNrNonTrivial */
  0,                              /* STMaxThreatLength */
  0,                              /* STMaxTimeHelpFilter */
  0                               /* STMaxTimeSeriesFilter */
};

/* Instrument a branch with STDirectDefense slices
 * @param si root of branch to be instrumented
 * @param to_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index to_goal)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  if (slices[to_goal].type!=STProxy)
  {
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,to_goal);
    to_goal = proxy;
  }

  slice_traversal_init(&st,&direct_guards_inserters,&to_goal);
  traverse_slices(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
