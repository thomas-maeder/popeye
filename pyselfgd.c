#include "pyselfgd.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/min_length_guard.h"
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
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_self_defense(slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STSelfDefense,proxy_to_goal);

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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
self_defense_has_solution_in_n(slice_index si,
                               stip_length_type n,
                               stip_length_type n_max_unsolvable)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      || n<=n_max_unsolvable) /* exact refutation */
  {
    result = attack_has_solution_in_n(to_goal,n,n_max_unsolvable);
    if (result>n)
      /* delegate to next even if (n==slack_length_battle) - we need
       * to distinguish between self-check and other ways of not
       * reaching the goal */
      result = attack_has_solution_in_n(next,n,n_max_unsolvable);
  }
  else
    result = attack_has_solution_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type self_defense_solve_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      || n<=n_max_unsolvable) /* exact refutation */
  {
    result = attack_solve_in_n(to_goal,n,n_max_unsolvable);
    if (result>n)
      /* delegate to next even if (n==slack_length_battle) - we need
       * to distinguish between self-check and other ways of not
       * reaching the goal */
      result = attack_solve_in_n(next,n,n_max_unsolvable);
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
static void self_guards_inserter_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;

    {
      slice_index const * const proxy_to_goal = st->param;
      slice_index const prototypes[] =
      {
          alloc_self_defense(*proxy_to_goal)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      battle_branch_insert_slices(si,prototypes,nr_prototypes);
    }

    if (min_length>slack_length_battle+1)
    {
      slice_index const prototype = alloc_min_length_guard(length-1,
                                                           min_length-1);
      battle_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
  stip_structure_traversal_override_single(&st,
                                           STReadyForDefense,
                                           &self_guards_inserter_defense);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a the appropriate proxy slices before each STLeaf slice
 * @param si identifies STLeaf slice
 * @param st address of structure representing the traversal
 */
static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_attack_adapter_slice(slack_length_battle,
                                         slack_length_battle-2));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors direct_leaf_instrumenters[] =
{
  { STLeaf, &instrument_leaf }
};

enum
{
  nr_direct_leaf_instrumenters = (sizeof direct_leaf_instrumenters
                                  / sizeof direct_leaf_instrumenters[0])
};

/* Instrument a branch leading to a goal to be a self goal branch
 * @param si identifies entry slice of branch
 */
void slice_make_self_goal_branch(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    direct_leaf_instrumenters,
                                    nr_direct_leaf_instrumenters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
