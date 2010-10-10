#include "pykeepmt.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STKeepMatingGuardAttackerFilter slice
 * @param mating mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_attacker_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardAttackerFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardDefenderFilter slice
 * @param mating mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_defender_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardDefenderFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardHelpFilter slice
 * @param side mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_help_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardHelpFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STKeepMatingGuardSeriesFilter slice
 * @param side mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_guard_series_filter(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingGuardSeriesFilter);
  slices[result].u.keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Direct ***************
 */

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
stip_length_type
keepmating_guard_direct_has_solution_in_n(slice_index si,
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
    result = attack_has_solution_in_n(next,n,n_max_unsolvable);
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
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
keepmating_guard_direct_solve_in_n(slice_index si,
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
    result = attack_solve_in_n(slices[si].u.pipe.next,n,n_max_unsolvable);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface DirectDefender **********
 */

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type keepmating_guard_defend_in_n(slice_index si,
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
    result = defense_defend_in_n(next,n,n_max_unsolvable);
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
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
keepmating_guard_can_defend_in_n(slice_index si,
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
    result = defense_can_defend_in_n(next,n,n_max_unsolvable);
  else
    result = n+4;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Help ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type keepmating_guard_help_solve_in_n(slice_index si,
                                                  stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    result = help_solve_in_n(slices[si].u.pipe.next,n);
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
stip_length_type keepmating_guard_help_has_solution_in_n(slice_index si,
                                                         stip_length_type n)
{
  Side const mating = slices[si].u.keepmating_guard.mating;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (is_a_mating_piece_left(mating))
    result = help_has_solution_in_n(slices[si].u.pipe.next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Series ***************
 */

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type keepmating_guard_series_solve_in_n(slice_index si,
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
    result = series_solve_in_n(slices[si].u.pipe.next,n);
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
stip_length_type keepmating_guard_series_has_solution_in_n(slice_index si,
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
    result = series_has_solution_in_n(slices[si].u.pipe.next,n);
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
  unsigned int nr_moves_passed;
} keepmatingguard_insertion_state_type;

static void keepmating_guards_inserter_goal(slice_index si,
                                            stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->for_side[advers(slices[si].starter)] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_quodlibet(slice_index si,
                                                 stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type * const state = st->param;
  keepmatingguard_insertion_state_type state1 = { { false, false },
                                                  state->nr_moves_passed };
  keepmatingguard_insertion_state_type state2 = { { false, false },
                                                  state->nr_moves_passed };

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

static void keepmating_guards_inserter_reciprocal(slice_index si,
                                                  stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type * const state = st->param;
  keepmatingguard_insertion_state_type state1 = { { false, false },
                                                  state->nr_moves_passed };
  keepmatingguard_insertion_state_type state2 = { { false, false },
                                                  state->nr_moves_passed };

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
void keepmating_guards_inserter_branch_fork(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order stip_traverse_structure_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_defender(slice_index si,
                                                stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type const * const state = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->nr_moves_passed>0)
  {
    if (state->for_side[White])
      guard = alloc_keepmating_guard_attacker_filter(White);
    else if (state->for_side[Black])
      guard = alloc_keepmating_guard_attacker_filter(Black);

    if (guard!=no_slice)
    {
      slices[guard].starter = slices[si].starter;
      pipe_append(slices[si].prev,guard);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void keepmating_guards_inserter_battle_fork(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* towards goal first, to detect the mating side */
  stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_attacker(slice_index si,
                                                stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type const * const state = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->nr_moves_passed>0)
  {
    if (state->for_side[White])
      guard = alloc_keepmating_guard_defender_filter(White);
    else if (state->for_side[Black])
      guard = alloc_keepmating_guard_defender_filter(Black);

    if (guard!=no_slice)
    {
      slices[guard].starter = slices[si].starter;
      pipe_append(slices[si].prev,guard);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_remember_move(slice_index si,
                                            stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type * const state = st->param;

  ++state->nr_moves_passed;
  stip_traverse_structure_children(si,st);
  --state->nr_moves_passed;
}

static void keepmating_guards_inserter_help_move(slice_index si,
                                                 stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type const * const state = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  keepmating_guards_remember_move(si,st);

  if (state->nr_moves_passed>0)
  {
    if (state->for_side[White])
      guard = alloc_keepmating_guard_help_filter(White);
    else if (state->for_side[Black])
      guard = alloc_keepmating_guard_help_filter(Black);

    if (guard!=no_slice)
    {
      slices[guard].starter = advers(slices[si].starter);
      pipe_append(si,guard);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_guards_inserter_series_move(slice_index si,
                                                   stip_structure_traversal *st)
{
  keepmatingguard_insertion_state_type const * const state = st->param;
  slice_index guard = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  keepmating_guards_remember_move(si,st);

  if (state->nr_moves_passed>0)
  {
    if (state->for_side[White])
      guard = alloc_keepmating_guard_series_filter(White);
    else if (state->for_side[Black])
      guard = alloc_keepmating_guard_series_filter(Black);

    if (guard!=no_slice)
    {
      slices[guard].starter = advers(slices[si].starter);
      pipe_append(si,guard);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors keepmating_guards_inserters[] =
{
  { STAttackMove,                       &keepmating_guards_remember_move        },
  { STDefenseMove,                      &keepmating_guards_remember_move        },
  { STAttackMoveFiltered,               &keepmating_guards_inserter_attacker    },
  { STDefenseMoveFiltered,              &keepmating_guards_inserter_defender    },
  { STHelpMove,                         &keepmating_guards_inserter_help_move   },
  { STHelpMoveToGoal,                   &keepmating_guards_inserter_help_move   },
  { STHelpFork,                         &keepmating_guards_inserter_branch_fork },
  { STSeriesMove,                       &keepmating_guards_inserter_series_move },
  { STSeriesMoveToGoal,                 &keepmating_guards_inserter_series_move },
  { STSeriesFork,                       &keepmating_guards_inserter_branch_fork },
  { STReciprocal,                       &keepmating_guards_inserter_reciprocal  },
  { STQuodlibet,                        &keepmating_guards_inserter_quodlibet   },
  { STReflexAttackerFilter,             &keepmating_guards_inserter_battle_fork },
  { STReflexDefenderFilter,             &keepmating_guards_inserter_battle_fork },
  { STSelfDefense,                      &keepmating_guards_inserter_battle_fork },
  { STDefenseFork,                      &keepmating_guards_inserter_battle_fork }
};

enum
{
  nr_keepmating_guards_inserters = (sizeof keepmating_guards_inserters
                                    / sizeof keepmating_guards_inserters[0])
};

/* Instrument stipulation with STKeepMatingGuard slices
 * @param si identifies slice where to start
 */
void stip_insert_keepmating_guards(slice_index si)
{
  keepmatingguard_insertion_state_type state = { { false, false }, 0};
  stip_structure_traversal st;
  SliceType type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);

  for (type = first_goal_tester_slice_type;
       type<=last_goal_tester_slice_type;
       ++type)
    stip_structure_traversal_override_single(&st,
                                             type,
                                             &keepmating_guards_inserter_goal);

  stip_structure_traversal_override(&st,
                                    keepmating_guards_inserters,
                                    nr_keepmating_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
