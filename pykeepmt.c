#include "pykeepmt.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>


/* Allocate a STKeepMatingFilter slice
 * @param mating mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_filter_slice(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the mating side still has a piece that could
 * deliver the mate.
 * @return true iff the mating side has such a piece
 */
static boolean is_a_mating_piece_left(Side mating_side)
{
  boolean const is_white_mating = mating_side==White;

  piece p = roib+1;
  while (p<derbla && nbpiece[is_white_mating ? p : -p]==0)
    p++;

  return p<derbla;
}

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
stip_length_type keepmating_filter_can_attack(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = can_attack(next,n,n_max_unsolvable);
  else
    result = n+2;

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
stip_length_type keepmating_filter_attack(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = attack(slices[si].u.pipe.next,n,n_max_unsolvable);
  else
    result = n+2;

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
stip_length_type keepmating_filter_defend(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = defend(next,n,n_max_unsolvable);
  else
    result = n+4;

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
stip_length_type keepmating_filter_can_defend(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating,"\n");

  if (is_a_mating_piece_left(mating))
    result = can_defend(next,n,n_max_unsolvable);
  else
    result = n+4;

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
stip_length_type keepmating_filter_help(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    result = help(slices[si].u.pipe.next,n);
  else
    result = n+2;

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
stip_length_type keepmating_filter_can_help(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    result = can_help(slices[si].u.pipe.next,n);
  else
    result = n+2;

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
stip_length_type keepmating_filter_series(slice_index si, stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (is_a_mating_piece_left(mating))
    result = series(slices[si].u.pipe.next,n);
  else
    result = n+1;

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
stip_length_type keepmating_filter_has_series(slice_index si,
                                              stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (is_a_mating_piece_left(mating))
    result = has_series(slices[si].u.pipe.next,n);
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Data structure for remembering the side(s) that needs to keep >= 1
 * piece that could deliver mate
 */
typedef struct
{
  boolean for_side[nr_sides];
} insertion_state_type;

static slice_index alloc_appropriate_filter(insertion_state_type const *state)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (state->for_side[White]+state->for_side[Black]==1)
  {
    Side const side = state->for_side[White] ? White : Black;
    result = alloc_keepmating_filter_slice(side);
  }
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void keepmating_filter_inserter_goal(slice_index si,
                                            stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->for_side[advers(slices[si].starter)] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_quodlibet(slice_index si,
                                                 stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  insertion_state_type state1 = { { false, false } };
  insertion_state_type state2 = { { false, false } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &state1;
  stip_traverse_structure(slices[si].u.binary.op1,st);

  st->param = &state2;
  stip_traverse_structure(slices[si].u.binary.op2,st);

  state->for_side[White] = state1.for_side[White] && state2.for_side[White];
  state->for_side[Black] = state1.for_side[Black] && state2.for_side[Black];

  st->param = state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_reciprocal(slice_index si,
                                                  stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  insertion_state_type state1 = { { false, false } };
  insertion_state_type state2 = { { false, false } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->param = &state1;
  stip_traverse_structure(slices[si].u.binary.op1,st);

  st->param = &state2;
  stip_traverse_structure(slices[si].u.binary.op2,st);

  state->for_side[White] = state1.for_side[White] || state2.for_side[White];
  state->for_side[Black] = state1.for_side[Black] || state2.for_side[Black];

  st->param = state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void keepmating_filter_inserter_branch_fork(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order stip_traverse_structure_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  stip_traverse_structure(slices[si].u.fork.fork,st);
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_battle_move(slice_index si,
                                                   stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_appropriate_filter(state);
    if (prototype!=no_slice)
    {
      slices[prototype].starter = slices[si].starter;
      battle_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_help_move(slice_index si,
                                                 stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_appropriate_filter(state);
    if (prototype!=no_slice)
    {
      slices[prototype].starter = advers(slices[si].starter);
      help_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_series_move(slice_index si,
                                                   stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_appropriate_filter(state);
    if (prototype!=no_slice)
    {
      slices[prototype].starter = advers(slices[si].starter);
      series_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors keepmating_filter_inserters[] =
{
  { STReadyForAttack,     &keepmating_filter_inserter_battle_move },
  { STDefenseAdapter ,    &keepmating_filter_inserter_battle_move },
  { STReadyForDefense,    &keepmating_filter_inserter_battle_move },
  { STReadyForHelpMove,   &keepmating_filter_inserter_help_move   },
  { STSeriesAdapter,      &keepmating_filter_inserter_series_move },
  { STReadyForSeriesMove, &keepmating_filter_inserter_series_move },
  { STAnd,                &keepmating_filter_inserter_reciprocal  },
  { STOr,                 &keepmating_filter_inserter_quodlibet   },
  { STEndOfBranch,        &keepmating_filter_inserter_branch_fork },
  { STEndOfBranchGoal,    &keepmating_filter_inserter_branch_fork },
  { STEndOfBranchForced,  &keepmating_filter_inserter_branch_fork },
  { STConstraint,         &keepmating_filter_inserter_branch_fork },
  { STForkOnRemaining,    &keepmating_filter_inserter_branch_fork },
  { STGoalReachedTester,  &keepmating_filter_inserter_goal        }
};

enum
{
  nr_keepmating_filter_inserters = (sizeof keepmating_filter_inserters
                                    / sizeof keepmating_filter_inserters[0])
};

/* Instrument stipulation with STKeepMatingFilter slices
 * @param si identifies slice where to start
 */
void stip_insert_keepmating_filters(slice_index si)
{
  insertion_state_type state = { { false, false } };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    keepmating_filter_inserters,
                                    nr_keepmating_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
