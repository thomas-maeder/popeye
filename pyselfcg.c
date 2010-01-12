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
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean selfcheck_guard_are_threats_refuted_in_n(table threats,
                                                 stip_length_type len_threat,
                                                 slice_index si,
                                                 stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = false;
  else if (slack_length_direct<=len_threat
           && len_threat<=n
           && table_length(threats)>0)
    result = direct_are_threats_refuted_in_n(threats,len_threat,
                                             slices[si].u.pipe.next,
                                             n);
  else
    result = true;

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
void selfcheck_guard_direct_solve_continuations_in_n(slice_index si,
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
stip_length_type
selfcheck_guard_direct_solve_threats_in_n(table threats,
                                          slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = (n-slack_length_direct)%2;
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    result = direct_solve_threats_in_n(threats,next,n,n_min);
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
selfcheck_guard_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = 0;
  else
    result = direct_has_solution_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean selfcheck_guard_root_defend(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = true;
  else
    result = direct_defender_root_defend(next);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean selfcheck_guard_defend_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = true;
  else
    result = direct_defender_defend_in_n(next,n);

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
unsigned int selfcheck_guard_can_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             unsigned int max_result)
{
  unsigned int result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = max_result+1;
  else
    result = direct_defender_can_defend_in_n(next,n,max_result);

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

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void selfcheck_guard_help_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (!echecc(nbply,advers(slices[si].starter)))
    help_solve_threats_in_n(threats,slices[si].u.pipe.next,n);

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

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void selfcheck_guard_series_solve_threats_in_n(table threats,
                                               slice_index si,
                                               stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (!echecc(nbply,advers(slices[si].starter)))
    series_solve_threats_in_n(threats,slices[si].u.pipe.next,n);

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

/* Solve a slice at non-root level
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type selfcheck_guard_solve_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = 0;
  else
    result = direct_solve_in_n(slices[si].u.pipe.next,n,n_min);

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
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STSelfCheckGuard slice after a
 * STBranch{Help,Series,Direct} slice
 */
static boolean selfcheck_guards_inserter_branch(slice_index si,
                                                slice_traversal *st)
{
  boolean const result = true;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[next].type!=STSelfCheckGuard)
  {
    pipe_insert_before(next);
    init_selfcheck_guard_slice(next);
    slice_traverse_children(next,st);
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
  {
    /* in self stipulations, the last defender's move may be allowed
     * to expose its own king (e.g. in s##!) */
    guard_pos = slices[guard_pos].u.pipe.next;
    TraceValue("->%u\n",guard_pos);
  }

  TraceEnumerator(SliceType,slices[guard_pos].type,"\n");
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

/* Insert a STSelfCheckGuard slice after a STParryFork slice
 */
static boolean selfcheck_guards_inserter_parry_fork(slice_index si,
                                                    slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const inverter = slices[si].u.pipe.next;
    slice_index const parrying = slices[si].u.pipe.u.parry_fork.parrying;

    pipe_insert_after(parrying);
    init_selfcheck_guard_slice(slices[parrying].u.pipe.next);

    /* circumvent STMoveInverter to prevent it from creating a
     * STSelfCheckGuard; if we take this path, we already know that
     * there is no check!
     */
    assert(slices[inverter].type==STMoveInverter);
    slice_traverse_children(inverter,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const selfcheck_guards_inserters[] =
{
  &selfcheck_guards_inserter_branch,        /* STBranchDirect */
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
  &selfcheck_guards_inserter_branch,        /* STDirectRoot */
  &selfcheck_guards_inserter_branch_direct_defender, /* STDirectDefenderRoot */
  &slice_traverse_children,                 /* STDirectHashed */
  &selfcheck_guards_inserter_branch,        /* STHelpRoot */
  &slice_traverse_children,                 /* STHelpHashed */
  &slice_traverse_children,                 /* STSeriesRoot */
  &selfcheck_guards_inserter_parry_fork,    /* STParryFork */
  &slice_traverse_children,                 /* STSeriesHashed */
  &slice_operation_noop,                    /* STSelfCheckGuard */
  &slice_traverse_children,                 /* STDirectDefense */
  &slice_traverse_children,                 /* STReflexGuard */
  &slice_traverse_children,                 /* STSelfAttack */
  &slice_traverse_children,                 /* STSelfDefense */
  0,                                        /* STRestartGuard */
  &slice_traverse_children,                 /* STGoalReachableGuard */
  0,                                        /* STKeepMatingGuard */
  0,                                        /* STMaxFlightsquares */
  &slice_traverse_children,                 /* STDegenerateTree */
  &slice_traverse_children,                 /* STMaxNrNonTrivial */
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
  slice_index new_leaf;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  new_leaf = copy_slice(si);
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
  0,                                             /* STHelpHashed */
  &selfcheck_guards_inserter_toplevel_root,      /* STSeriesRoot */
  0,                                             /* STParryFork */
  0,                                             /* STSeriesHashed */
  0,                                             /* STSelfCheckGuard */
  &selfcheck_guards_inserter_toplevel_root,      /* STDirectDefense */
  &selfcheck_guards_inserter_toplevel_reflex_guard, /* STReflexGuard */
  0,                                             /* STSelfAttack */
  0,                                             /* STSelfDefense */
  0,                                             /* STRestartGuard */
  0,                                             /* STGoalReachableGuard */
  0,                                             /* STKeepMatingGuard */
  0,                                             /* STMaxFlightsquares */
  0,                                             /* STDegenerateTree */
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
