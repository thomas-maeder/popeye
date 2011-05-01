#include "stipulation/dead_end.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/series_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDeadEnd defender slice.
 * @return index of allocated slice
 */
slice_index alloc_dead_end_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STDeadEnd);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the moves beyond a attack end slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_dead_end(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining>0)
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  slice_index optimisable_deadend;
  stip_length_type min_remaining;
} optimisation_state;

static void optimise_deadend_help(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;
  stip_length_type const save_min_remaining = state->min_remaining;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->min_remaining = 0;
  stip_traverse_moves_children(si,st);
  state->min_remaining = save_min_remaining;

  if (state->optimisable_deadend!=no_slice)
  {
    slice_index const prototype = alloc_pipe(STDeadEndGoal);
    help_branch_insert_slices(si,&prototype,1);
    pipe_remove(state->optimisable_deadend);
    state->optimisable_deadend = no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_deadend_series(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;
  stip_length_type const save_min_remaining = state->min_remaining;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->min_remaining = 0;
  stip_traverse_moves_children(si,st);
  state->min_remaining = save_min_remaining;

  if (state->optimisable_deadend!=no_slice)
  {
    slice_index const prototype = alloc_pipe(STDeadEndGoal);
    series_branch_insert_slices(si,&prototype,1);
    pipe_remove(state->optimisable_deadend);
    state->optimisable_deadend = no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_deadend(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (st->remaining)
  {
    case 0:
      state->optimisable_deadend = si;
      break;

    case 1:
      stip_traverse_moves_children(si,st);
      if (state->min_remaining==0)
        pipe_remove(si);
      break;

    default:
      stip_traverse_moves_children(si,st);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_deadend(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  state->optimisable_deadend = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_move(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;
  stip_length_type const save_min_remaining = state->min_remaining;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->min_remaining = 0;
  stip_traverse_moves_children(si,st);
  state->optimisable_deadend = no_slice;
  state->min_remaining = save_min_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void raise_min_remaining(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->min_remaining = 1;
  stip_traverse_moves_children(si,st);
  state->min_remaining = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const dead_end_optimisers[] =
{
  { STAttackAdapter,         &start_move              },
  { STDefenseAdapter,        &start_move              },
  { STHelpAdapter,           &optimise_deadend_help   },
  { STSeriesAdapter,         &optimise_deadend_series },
  { STReadyForAttack,        &start_move              },
  { STReadyForDefense,       &start_move              },
  { STReadyForHelpMove,      &optimise_deadend_help   },
  { STReadyForSeriesMove,    &optimise_deadend_series },
  { STDeadEnd,               &remember_deadend        },
  { STEndOfBranch,           &forget_deadend          },
  { STEndOfBranchForced,     &forget_deadend          },
  { STPrerequisiteOptimiser, &raise_min_remaining     }
};

enum
{
  nr_dead_end_optimisers =
  (sizeof dead_end_optimisers / sizeof dead_end_optimisers[0])
};

/* Optimise away redundant deadend slices
 * @param si identifies the entry slice
 */
void stip_optimise_dead_end_slices(slice_index si)
{
  stip_moves_traversal mt;
  optimisation_state state = { no_slice, 0 };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&mt,&state);
  stip_moves_traversal_override(&mt,dead_end_optimisers,nr_dead_end_optimisers);
  stip_traverse_moves(si,&mt);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type dead_end_can_attack(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
    n_max_unsolvable = slack_length_battle;

  if (n<=n_max_unsolvable)
    result = n+2;
  else
    result = can_attack(slices[si].u.pipe.next,n,n_max_unsolvable);

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
stip_length_type dead_end_attack(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
    n_max_unsolvable = slack_length_battle;

  if (n<=n_max_unsolvable)
    result = n+2;
  else
    result = attack(slices[si].u.pipe.next,n,n_max_unsolvable);

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
stip_length_type dead_end_defend(slice_index si,
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

  if (n_max_unsolvable<slack_length_battle)
    n_max_unsolvable = slack_length_battle;

  if (n==slack_length_battle
      /*|| n<=n_max_unsolvable*/) /* no dead end, we are solving refutations! */
    result = n+4;
  else
    result = defend(next,n,n_max_unsolvable);

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
stip_length_type dead_end_can_defend(slice_index si,
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

  if (n_max_unsolvable<slack_length_battle)
    n_max_unsolvable = slack_length_battle;

  if (n<=n_max_unsolvable)
    result = n+4;
  else
    result = can_defend(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type dead_end_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = n+2;
  else
    result = help(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type dead_end_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = n+2;
  else
    result = can_help(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type dead_end_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    result = n+1;
  else
    result = series(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type dead_end_has_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (n==slack_length_series)
    result = n+1;
  else
    result = has_series(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
