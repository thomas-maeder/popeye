#include "pydirctg.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STDirectDefenderFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_direct_defender_filter_slice(stip_length_type length,
                                               stip_length_type min_length,
                                               slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STDirectDefenderFilter,
                             length,min_length,
                             proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Slice **********
 */

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
direct_defender_filter_root_defend(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min,
                                   unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  if (n_min<=slack_length_battle+1)
  {
    if (defense_root_solve(to_goal))
      result = n_min;
    else if (n>slack_length_battle)
      result = defense_root_defend(next,n,n_min,max_nr_refutations);
    else
      result = n+4;
  }
  else
    result = defense_root_defend(next,n,n_min,max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
stip_length_type direct_defender_filter_defend_in_n(slice_index si,
                                                    stip_length_type n,
                                                    stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (n_min<=slack_length_battle+1)
  {
    if (!defense_defend(to_goal))
      result = n_min;
    else if (n>slack_length_battle)
      result = defense_defend_in_n(next,n,n_min);
    else
      result = n+2;
  }
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
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
direct_defender_filter_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_min,
                                       unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();


  if (n_min<=slack_length_battle+1
      && !defense_can_defend(to_goal))
    result = n_min;
  else if (n>slack_length_battle)
    result = defense_can_defend_in_n(next,n,n_min,max_nr_refutations);
  else
    result = n+4;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void direct_defender_filter_insert_root(slice_index si,
                                        stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  slice_index root_filter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.pipe.next,st);

  root_filter = copy_slice(si);
  pipe_link(root_filter,*root);
  *root = root_filter;

  battle_branch_shorten_slice(si);
 
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void direct_defender_filter_reduce_to_postkey_play(slice_index si,
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

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void direct_defense_impose_starter(slice_index si, stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STDirectDefenderFilter slice after each STAttackMove slice
 * @param si identifies slice before which to insert a *
 *           STDirectDefenderFilter slice
 * @param st address of structure representing the traversal
 */
static void direct_guards_inserter_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index * const to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const
        filter = alloc_direct_defender_filter_slice(length-1,min_length-1,
                                                    *to_goal); 
    pipe_append(si,filter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const direct_guards_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &direct_guards_inserter_attack,    /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &stip_traverse_structure_children, /* STBHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &stip_traverse_structure_children, /* STSeriesFork */
  &stip_structure_visitor_noop,      /* STLeafDirect */
  &stip_structure_visitor_noop,      /* STLeafHelp */
  &stip_structure_visitor_noop,      /* STLeafForced */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children, /* STAttackRoot */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STRefutationsWriter */
  &stip_traverse_structure_children, /* STThreatWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STThreatCollector */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &stip_traverse_structure_children, /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &stip_traverse_structure_children, /* STReflexDefenderFilter */
  &stip_traverse_structure_children, /* STSelfDefense */
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentHelpFilter */
  &stip_traverse_structure_children, /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &stip_traverse_structure_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &stip_traverse_structure_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument a branch with STDirectDefenderFilter slices
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index proxy_to_goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(slices[proxy_to_goal].type==STProxy);

  stip_structure_traversal_init(&st,&direct_guards_inserters,&proxy_to_goal);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
