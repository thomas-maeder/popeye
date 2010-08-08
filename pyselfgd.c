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
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
self_defense_direct_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_min,
                                      stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index const length = slices[si].u.branch_fork.length;
  slice_index const min_length = slices[si].u.branch_fork.min_length;
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
  {
    switch (slice_has_solution(to_goal))
    {
      case opponent_self_check:
        result = slack_length_battle-2;
        break;

      case has_no_solution:
        n_max_unsolvable = slack_length_battle;
        result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);
        break;

      case has_solution:
        if (n-slack_length_battle<=length-min_length)
          result = slack_length_battle;
        break;

      default:
        assert(0);
        break;
    }
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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type self_defense_solve_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const towards_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
  {
    switch (slice_has_solution(towards_goal))
    {
      case opponent_self_check:
        result = slack_length_battle-2;
        break;

      case has_solution:
        slice_solve(towards_goal);
        result = slack_length_battle;
        break;

      case has_no_solution:
        /* delegate to next even if (n==slack_length_battle) - we need
         * to distinguish between self-check and other ways of not
         * reaching the goal
         */
        n_max_unsolvable = slack_length_battle;
        result = attack_solve_in_n(next,n,n_max_unsolvable);
        break;

      default:
        assert(0);
        break;
    }
  }
  else
    result = attack_solve_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
      pipe_append(si,alloc_self_defense(length,min_length,*proxy_to_goal));
    }
    else
    {
      pipe_set_successor(si,next_prev);
      slices[next_prev].u.branch.length = length;
      slices[next_prev].u.branch.min_length = min_length;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors self_guards_inserters[] =
{
  { STDefenseMovePlayed, &self_guards_inserter_defense_move }
};

enum
{
  nr_self_guards_inserters = (sizeof self_guards_inserters
                              / sizeof self_guards_inserters[0])
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

  stip_structure_traversal_init(&st,&proxy_to_goal);
  stip_structure_traversal_override(&st,
                                    self_guards_inserters,
                                    nr_self_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
