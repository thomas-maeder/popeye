#include "solving/battle_play/continuation.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STContinuationSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_continuation_solver_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STContinuationSolver,no_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_continuation_solver(slice_index si,
                                             stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_pipe(si,st);
  if (slices[si].u.fork.tester!=no_slice)
    stip_traverse_moves(slices[si].u.fork.tester,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type continuation_solver_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_defend(slices[si].u.fork.tester,n);
  if (slack_length_battle<=result && result<n+4)
  {
    stip_length_type const n_next = n<result ? n : result;
#if !defined(NDEBUG)
    stip_length_type const defend_result =
#endif
    defend(slices[si].u.fork.next,n_next);
    assert(defend_result==result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* nested defense branch
 */
static void insert_continuation_solvers_defense_adapter(slice_index si,
                                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->level!=structure_traversal_level_root
      && slices[si].u.branch.length>slack_length_battle)
  {
    slice_index const prototype = alloc_continuation_solver_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* attack branch
 */
static void insert_continuation_solvers_attack(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_continuation_solver_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const continuation_solver_inserters[] =
{
  { STDefenseAdapter, &insert_continuation_solvers_defense_adapter },
  { STReadyForAttack, &insert_continuation_solvers_attack          }
};

enum
{
  nr_continuation_solver_inserters = sizeof continuation_solver_inserters / sizeof continuation_solver_inserters[0]
};

/* Instrument the stipulation structure with STContinuationSolver slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_continuation_solvers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    continuation_solver_inserters,
                                    nr_continuation_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

