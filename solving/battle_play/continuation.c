#include "stipulation/battle_play/continuation.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STContinuationSolver defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_continuation_solver_slice(stip_length_type length,
                                            stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STContinuationSolver,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type continuation_solver_defend(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = can_defend(next,n,n_max_unsolvable);
  if (slack_length_battle<=result && result<n+4)
  {
    stip_length_type const n_next = n<result ? n : result;
#if !defined(NDEBUG)
    stip_length_type const defend_result =
#endif
    defend(next,n_next,n_max_unsolvable);
    assert(defend_result==result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
continuation_solver_can_defend(slice_index si,
                               stip_length_type n,
                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = can_defend(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Remember a STContinuationSolver slice for possible optimisation
 * @param si identifies STContinuationSolver slice
 * @param st address of structure that holds the state of the traversal
 */
static void remember_solver(slice_index si, stip_structure_traversal *st)
{
  slice_index * const solver = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  *solver = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget about a possibly remembered STContinuationSolver slice
 * @param si identifies slice being traversed
 * @param st address of structure that holds the state of the traversal
 */
static void forget_solver(slice_index si, stip_structure_traversal *st)
{
  slice_index * const solver = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  *solver = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Perform the optimisation if appropriate
 * @param si identifies STDefenseAdapter slice
 * @param st address of structure that holds the state of the traversal
 */
static void optimise_solver_defense(slice_index si, stip_structure_traversal *st)
{
  slice_index * const solver = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *solver = no_slice;

  stip_traverse_structure_children(si,st);

  if (*solver!=no_slice)
  {
    if (slices[si].u.branch.length==slack_length_battle)
      pipe_remove(*solver);
    *solver = no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors optimisers[] =
{
  { STContinuationSolver, &remember_solver         },
  { STAttackAdapter,      &forget_solver           },
  { STDefenseAdapter,     &optimise_solver_defense }
};

enum
{
  nr_optimisers = sizeof optimisers / sizeof optimisers[0]
};

/* Remove STContinuationSolver slices that don't contribute to the solution (but
 * that may cause unnecessary calculations to be performed)
 * @param si identifies stipulation entry slice
 */
void optimise_away_redundant_continuation_solvers(slice_index si)
{
  stip_structure_traversal st;
  slice_index next_solver;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&next_solver);
  stip_structure_traversal_override(&st,optimisers,nr_optimisers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
