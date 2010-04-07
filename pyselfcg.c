#include "pyselfcg.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STSelfCheckGuardRootSolvableFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_root_solvable_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSelfCheckGuardRootSolvableFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfCheckGuardSolvableFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_solvable_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSelfCheckGuardSolvableFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfCheckGuardRootDefenderFilter slice
 * @return allocated slice
 */
slice_index
alloc_selfcheck_guard_root_defender_filter(stip_length_type length,
                                           stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSelfCheckGuardRootDefenderFilter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfCheckGuardAttackerFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_attacker_filter(stip_length_type length,
                                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSelfCheckGuardAttackerFilter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfCheckGuardDefenderFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_defender_filter(stip_length_type length,
                                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSelfCheckGuardDefenderFilter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfCheckGuardHelpFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_help_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSelfCheckGuardHelpFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STSelfCheckGuardSeriesFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_series_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSelfCheckGuardSeriesFilter);

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
  else if (slack_length_battle<=len_threat
           && len_threat<=n
           && table_length(threats)>0)
    result = attack_are_threats_refuted_in_n(threats,len_threat,
                                             slices[si].u.pipe.next,
                                             n);
  else
    result = true;

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
 *         (n-slack_length_battle)%2 if the attacker has something
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
    result = (n-slack_length_battle)%2;
  else if (n>=slack_length_battle)
  {
    slice_index const next = slices[si].u.pipe.next;
    result = attack_solve_threats_in_n(threats,next,n,n_min);
  }
  else
    result = n+2;

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
 *            <n_min defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_battle and we return
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
    result = n_min-2;
  else if (n>=slack_length_battle)
    result = attack_has_solution_in_n(next,n,n_min);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defending side can successfully defend
 */
boolean selfcheck_guard_root_defend(slice_index si, stip_length_type n_min)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = true;
  else
    result = defense_root_defend(next,n_min);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void selfcheckguard_root_defender_filter_reduce_to_postkey_play(slice_index si,
                                                                stip_structure_traversal *st)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(next,st);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean selfcheck_guard_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_min)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = true;
  else
    result = defense_defend_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_nr_refutations how many refutations should we look for
 * @return n+4 refuted - >max_nr_refutations refutations found
           n+2 refuted - <=max_nr_refutations refutations found
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
 */
stip_length_type
selfcheck_guard_can_defend_in_n(slice_index si,
                                stip_length_type n,
                                unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = n+4;
  else
    result = defense_can_defend_in_n(next,n,max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void selfcheck_guard_defender_filter_insert_root(slice_index si,
                                                 stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index root_filter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.pipe.next,st);

  root_filter = alloc_selfcheck_guard_root_defender_filter(length,min_length);
  pipe_link(root_filter,*root);
  *root = root_filter;

  slices[si].u.branch.length -= 2;
  if (min_length>=slack_length_battle+2)
    slices[si].u.branch.min_length -= 2;
 
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
 *         (n-slack_length_battle)%2 if the previous move led to a
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
    result = n_min-2;
  else if (n>=slack_length_battle)
    result = attack_solve_in_n(slices[si].u.pipe.next,n,n_min);
  else
    result = n+2;

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

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void selfcheck_guard_solvable_filter_insert_root(slice_index si,
                                                 stip_structure_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.pipe.next,st);

  {
    slice_index const
        root_guard = alloc_selfcheck_guard_root_solvable_filter();
    pipe_link(root_guard,*root);
    *root = root_guard;
    dealloc_slice(si);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Stipulation instrumentation ***************
 */

/* Insert a STSelfCheckGuard* at the beginning of a toplevel branch
 */
static
void selfcheck_guards_inserter_toplevel_root(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prev = slices[si].prev;
    slice_index const guard = alloc_selfcheck_guard_root_solvable_filter();
    pipe_link(prev,guard);
    pipe_link(guard,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const selfcheck_guards_toplevel_inserters[] =
{
  &stip_traverse_structure_children,        /* STProxy */
  &stip_traverse_structure_children,        /* STAttackMove */
  &stip_traverse_structure_children,        /* STDefenseMove */
  &stip_traverse_structure_children,        /* STHelpMove */
  &stip_traverse_structure_children,        /* STHelpFork */
  &stip_traverse_structure_children,        /* STSeriesMove */
  &stip_traverse_structure_children,        /* STSeriesFork */
  &selfcheck_guards_inserter_toplevel_root, /* STLeafDirect */
  &selfcheck_guards_inserter_toplevel_root, /* STLeafHelp */
  &selfcheck_guards_inserter_toplevel_root, /* STLeafForced */
  &stip_traverse_structure_children,        /* STReciprocal */
  &stip_traverse_structure_children,        /* STQuodlibet */
  &stip_traverse_structure_children,        /* STNot */
  &stip_structure_visitor_noop,             /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,        /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,        /* STMoveInverterSeriesFilter */
  &selfcheck_guards_inserter_toplevel_root, /* STAttackRoot */
  &stip_traverse_structure_children,        /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,        /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,        /* STContinuationWriter */
  &stip_traverse_structure_children,        /* STTryWriter */
  &stip_traverse_structure_children,        /* STThreatWriter */
  &stip_traverse_structure_children,        /* STThreatEnforcer */
  &stip_traverse_structure_children,        /* STRefutationsCollector */
  &stip_traverse_structure_children,        /* STVariationWriter */
  &stip_traverse_structure_children,        /* STRefutingVariationWriter */
  &stip_traverse_structure_children,        /* STNoShortVariations */
  &stip_traverse_structure_children,        /* STAttackHashed */
  &selfcheck_guards_inserter_toplevel_root, /* STHelpRoot */
  &stip_traverse_structure_children,        /* STHelpShortcut */
  &stip_traverse_structure_children,        /* STHelpHashed */
  &selfcheck_guards_inserter_toplevel_root, /* STSeriesRoot */
  &stip_traverse_structure_children,        /* STSeriesShortcut */
  &stip_traverse_structure_children,        /* STParryFork */
  &stip_traverse_structure_children,        /* STSeriesHashed */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardRootSolvableFilter */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardSolvableFilter */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardRootDefenderFilter */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardAttackerFilter */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardDefenderFilter */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardHelpFilter */
  &stip_structure_visitor_noop,             /* STSelfCheckGuardSeriesFilter */
  &selfcheck_guards_inserter_toplevel_root, /* STDirectDefenseRootSolvableFilter */
  &stip_traverse_structure_children,        /* STDirectDefense */
  &selfcheck_guards_inserter_toplevel_root, /* STReflexHelpFilter */
  &selfcheck_guards_inserter_toplevel_root, /* STReflexSeriesFilter */
  &selfcheck_guards_inserter_toplevel_root, /* STReflexRootSolvableFilter */
  &stip_traverse_structure_children,        /* STReflexAttackerFilter */
  &stip_traverse_structure_children,        /* STReflexDefenderFilter */
  &stip_traverse_structure_children,        /* STSelfDefense */
  &stip_traverse_structure_children,        /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,        /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,        /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,        /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,        /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,        /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,        /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,        /* STMaxFlightsquares */
  &stip_traverse_structure_children,        /* STDegenerateTree */
  &stip_traverse_structure_children,        /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,        /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,        /* STMaxThreatLength */
  &stip_traverse_structure_children,        /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,        /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,        /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,        /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,        /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,        /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,        /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children         /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument stipulation with STSelfCheckGuard slices
 */
void stip_insert_selfcheck_guards(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&selfcheck_guards_toplevel_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
