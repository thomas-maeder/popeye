#include "pyselfgd.h"
#include "pybrafrk.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyoutput.h"
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
stip_length_type self_defense_direct_has_solution_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n_min>=slack_length_battle-1);

  if (n_min==slack_length_battle-1
      && slice_has_solution(towards_goal)>=has_solution)
    result = n_min;
  else
    result = attack_has_solution_in_n(next,n,n_min);

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
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const max_n_for_goal = length-min_length+slack_length_battle;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",max_n_for_goal);

  if (n<max_n_for_goal && slice_has_solution(towards_goal)>=has_solution)
    result = false;
  else
    result = attack_are_threats_refuted_in_n(threats,len_threat,next,n);

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

  result = attack_solve_threats_in_n(threats,next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Slice ***************
 */

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 */
void self_attack_root_make_setplay_slice(slice_index si,
                                         stip_structure_traversal *st)
{
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const copy = copy_slice(si);
    pipe_link(copy,prod->setplay_slice);
    prod->setplay_slice = copy;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void self_attack_root_reduce_to_postkey_play(slice_index si,
                                             stip_structure_traversal *st)
{
  slice_index *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *postkey_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 */
stip_length_type self_defense_solve_in_n(slice_index si,
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

  switch (slice_has_solution(towards_goal))
  {
    case is_solved:
      if (n_min<=slack_length_battle)
      {
        result = n_min;
        write_final_defense();
        {
          boolean const solving_result = slice_solve(towards_goal);
          assert(solving_result);
        }
      }
      else
        result = slack_length_battle-1;
      break;

    case has_solution:
      if (n_min<=slack_length_battle)
      {
        result = n_min;
        write_defense();
        {
          boolean const solving_result = slice_solve(towards_goal);
          assert(solving_result);
        }
      }
      else
        result = slack_length_battle-1;
      break;

    case has_no_solution:
      result = attack_solve_in_n(next,n,n_min);
      break;

    default:
      result = n_min-2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice - adapter for direct slices
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean self_defense_solve(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length==slack_length_battle-1 && slice_solve(towards_goal))
    result = false;
  else if (length>slack_length_battle
           && attack_has_solution_in_n(next,length,min_length)<=length)
    result = attack_solve(next);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void self_attack_impose_starter(slice_index si, stip_structure_traversal *st)
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
  &stip_traverse_structure_children,  /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,  /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,  /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children,  /* STContinuationWriter */
  &stip_traverse_structure_children,  /* STRefutationsWriter */
  &stip_traverse_structure_children,  /* STThreatWriter */
  &stip_traverse_structure_children,  /* STThreatEnforcer */
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
  &stip_traverse_structure_children,  /* STSelfCheckGuardRootDefenderFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children,  /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,  /* STDirectDefenderFilter */
  &stip_traverse_structure_children,  /* STReflexHelpFilter */
  &stip_traverse_structure_children,  /* STReflexSeriesFilter */
  &stip_traverse_structure_children,  /* STReflexRootSolvableFilter */
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
  &stip_traverse_structure_children,  /* STKeepMatingGuardRootDefenderFilter */
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
 * @return identifier of branch entry slice after insertion
 */
slice_index slice_insert_self_guards(slice_index si, slice_index proxy_to_goal)
{
  stip_structure_traversal st;
  slice_index const result = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(slices[proxy_to_goal].type==STProxy);

  stip_structure_traversal_init(&st,&self_guards_inserters,&proxy_to_goal);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
