#include "pyselfgd.h"
#include "pybrafrk.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STSelfDefense slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_self_defense(stip_length_type length,
                                      stip_length_type min_length,
                                      slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STSelfDefense,length,min_length,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void self_defense_insert_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.pipe.next,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
    slice_index const self_defense = alloc_self_defense(length,min_length,
                                                        to_goal);
    pipe_link(self_defense,*root);
    *root = self_defense;

    battle_branch_shorten_slice(si);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
self_defense_direct_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_min,
                                      stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n_min>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
    switch (slice_has_solution(slices[si].u.branch_fork.towards_goal))
    {
      case opponent_self_check:
        result = n_min-2;
        break;

      case has_solution:
        if (n_min<slack_length_battle+2)
          result = n_min;
        else
          result = n+2;
        break;

      case has_no_solution:
        result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);
        break;

      default:
        assert(0);
        break;
    }
  else
    result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Slice ***************
 */

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type self_defense_solve_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n_min<=slack_length_battle+1)
    switch (slice_solve(towards_goal))
    {
      case opponent_self_check:
        result = n_min-2;
        break;

      case has_solution:
        result = n_min;
        break;

      case has_no_solution:
        result = attack_solve_in_n(next,n,n_min,n_max_unsolvable);
        break;

      default:
        assert(0);
        result = n+2;
        break;
    }
  else
    switch (slice_has_solution(towards_goal))
    {
      case opponent_self_check:
        result = n_min-2;
        break;

      case has_solution:
        result = n_min;
        break;

      case has_no_solution:
        result = attack_solve_in_n(next,n,n_min,n_max_unsolvable);
        break;

      default:
        assert(0);
        result = n+2;
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type self_defense_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type n_max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  n_max_unsolvable = battle_branch_calc_n_min(si,length)-2;

  if (min_length==slack_length_battle)
  {
    has_solution_type const goal_solution = slice_solve(towards_goal);
    if (goal_solution==has_no_solution)
    {
      if (length>slack_length_battle+1
          && (attack_has_solution_in_n(next,length,min_length,n_max_unsolvable)
              <=length))
        result = attack_solve(next);
      else
        result = has_no_solution;
    }
    else
      result = goal_solution;
  }
  else
  {
    if (length>slack_length_battle+1
        && (attack_has_solution_in_n(next,length,min_length,n_max_unsolvable)
            <=length))
      result = attack_solve(next);
    else
      result = has_no_solution;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void self_defense_impose_starter(slice_index si, stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure(slices[si].u.pipe.next,st);
  stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STSelfDefense after each STDefenseMove
 */
static void self_guards_inserter_defense_move(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const next_prev = slices[next].prev;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    if (next_prev==si)
    {
      slice_index const * const proxy_to_goal = st->param;
      pipe_append(si,alloc_self_defense(length-1,min_length-1,*proxy_to_goal));
    }
    else
    {
      pipe_set_successor(si,next_prev);
      slices[next_prev].u.branch.length = length-1;
      slices[next_prev].u.branch.min_length = min_length-1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const self_guards_inserters[] =
{
  &stip_traverse_structure_children,  /* STProxy */
  &stip_traverse_structure_children,  /* STAttackMove */
  &self_guards_inserter_defense_move, /* STDefenseMove */
  &stip_traverse_structure_children,  /* STHelpMove */
  &stip_traverse_structure_children,  /* STHelpFork */
  &stip_traverse_structure_children,  /* STSeriesMove */
  &stip_traverse_structure_children,  /* STSeriesFork */
  &stip_structure_visitor_noop,       /* STLeafDirect */
  &stip_structure_visitor_noop,       /* STLeafHelp */
  &stip_traverse_structure_children,  /* STLeafForced */
  &stip_traverse_structure_children,  /* STReciprocal */
  &stip_traverse_structure_children,  /* STQuodlibet */
  &stip_traverse_structure_children,  /* STNot */
  &stip_traverse_structure_children,  /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,  /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children,  /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children,  /* STAttackRoot */
  &stip_traverse_structure_children,  /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,  /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,  /* STContinuationWriter */
  &stip_traverse_structure_children,  /* STRefutationsWriter */
  &stip_traverse_structure_children,  /* STThreatWriter */
  &stip_traverse_structure_children,  /* STThreatEnforcer */
  &stip_traverse_structure_children,  /* STThreatCollector */
  &stip_traverse_structure_children,  /* STRefutationsCollector */
  &stip_traverse_structure_children,  /* STVariationWriter */
  &stip_traverse_structure_children,  /* STRefutingVariationWriter */
  &stip_traverse_structure_children,  /* STNoShortVariations */
  &stip_traverse_structure_children,  /* STAttackHashed */
  &stip_traverse_structure_children,  /* STHelpRoot */
  &stip_traverse_structure_children,  /* STHelpShortcut */
  &stip_traverse_structure_children,  /* STHelpHashed */
  &stip_traverse_structure_children,  /* STSeriesRoot */
  &stip_traverse_structure_children,  /* STSeriesShortcut */
  &stip_traverse_structure_children,  /* STParryFork */
  &stip_traverse_structure_children,  /* STSeriesHashed */
  &stip_traverse_structure_children,  /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,  /* STDirectDefenderFilter */
  &stip_traverse_structure_children,  /* STReflexRootFilter */
  &stip_traverse_structure_children,  /* STReflexHelpFilter */
  &stip_traverse_structure_children,  /* STReflexSeriesFilter */
  &stip_traverse_structure_children,  /* STReflexAttackerFilter */
  &stip_traverse_structure_children,  /* STReflexDefenderFilter */
  &stip_traverse_structure_children,  /* STSelfDefense */
  &stip_traverse_structure_children,  /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children,  /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children,  /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children,  /* STIntelligentHelpFilter */
  &stip_traverse_structure_children,  /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children,  /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children,  /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,  /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,  /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children,  /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children,  /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,  /* STMaxFlightsquares */
  &stip_traverse_structure_children,  /* STDegenerateTree */
  &stip_traverse_structure_children,  /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,  /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,  /* STMaxThreatLength */
  &stip_traverse_structure_children,  /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,  /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children,  /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children,  /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,  /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,  /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children,  /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children,  /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,  /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,  /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children   /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument a branch with STSelfDefense slices
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 */
void slice_insert_self_guards(slice_index si, slice_index proxy_to_goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(slices[proxy_to_goal].type==STProxy);

  stip_structure_traversal_init(&st,&self_guards_inserters,&proxy_to_goal);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
