#include "pyreflxg.h"
#include "stipulation/branch.h"
#include "stipulation/reflex_attack_solver.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/min_length_guard.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/play.h"
#include "stipulation/series_play/ready_for_series_move.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STReflexHelpFilter slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_help_filter(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexHelpFilter,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface attacker_filter **************
 */

/* Allocate a STReflexAttackerFilter slice
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_attacker_filter(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexAttackerFilter,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_attacker_filter_make_root(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index solver;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solver = alloc_reflex_attack_solver(avoided);

  stip_traverse_structure_pipe(si,st);
  pipe_link(solver,*root_slice);
  *root_slice = solver;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
reflex_attacker_filter_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(avoided))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = attack_has_solution_in_n(next,n,n_max_unsolvable);
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
stip_length_type
reflex_attacker_filter_solve_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (n==n_max_unsolvable
          ? slice_solve(avoided) /* we are solving refutations */
          : slice_has_solution(avoided))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_solution:
      result = attack_solve_in_n(next,n,n_max_unsolvable);
      break;

    case has_no_solution:
      result = n+2;
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

/* **************** Implementation of interface defender_filter **********
 */

/* Allocate a STReflexDefenderFilter slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_defender_filter(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexDefenderFilter,proxy_to_avoided);
TraceStipulation(result);

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
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
reflex_defender_filter_defend_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.reflex_guard.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      && slice_solve(avoided)==has_no_solution)
    result = slack_length_battle;
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

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
reflex_defender_filter_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle
      && slice_has_solution(avoided)==has_no_solution)
    result = slack_length_battle;
  else
    result = defense_can_defend_in_n(next,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void
reflex_guard_defender_filter_make_setplay_slice(slice_index si,
                                                stip_structure_traversal *st)
{
  slice_index * const result = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const guard = alloc_reflex_attack_solver(avoided);
    pipe_link(guard,*result);
    *result = guard;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void reflex_defender_filter_reduce_to_postkey_play(slice_index si,
                                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index * const postkey_slice = st->param;
    slice_index const avoided = slices[si].u.reflex_guard.avoided;
    slice_index const filter = alloc_reflex_attack_solver(avoided);
    pipe_link(filter,*postkey_slice);
    *postkey_slice = filter;

    pipe_unlink(slices[si].prev);
    dealloc_slice(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Implementation of interface help_filter ************
 */

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_help_filter_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const solver = alloc_reflex_attack_solver(avoided);
    pipe_link(solver,*root_slice);
    *root_slice = solver;
  }

  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type reflex_help_filter_solve_in_n(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = help_solve_in_n(next,n);
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
stip_length_type reflex_help_filter_has_solution_in_n(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = help_has_solution_in_n(next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface series_filter ************
 */

/* Allocate a STReflexSeriesFilter slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @param proxy_to_avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
static slice_index alloc_reflex_series_filter(slice_index proxy_to_avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STReflexSeriesFilter,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_series_filter_make_root(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const solver = alloc_reflex_attack_solver(avoided);
    pipe_link(solver,*root_slice);
    *root_slice = solver;
  }

  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a reflex attacker filter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_reflex_series_filter(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==slack_length_series)
    stip_traverse_moves_branch(slices[si].u.reflex_guard.avoided,st);
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type reflex_series_filter_solve_in_n(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = series_solve_in_n(next,n);
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
stip_length_type reflex_series_filter_has_solution_in_n(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = series_has_solution_in_n(next,n);
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

typedef struct
{
    slice_index avoided_defense;
    slice_index avoided_attack;
} init_param;

/* In battle play, insert a STReflexAttackFilter slice before a
 * slice where the reflex stipulation might force the side at the move
 * to reach the goal
 */
static void reflex_guards_inserter_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  init_param * const param = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy_to_avoided = param->avoided_attack;
    slice_index const prototype = alloc_reflex_attacker_filter(proxy_to_avoided);
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In battle play, insert a STReflexDefenseFilter slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static void reflex_guards_inserter_defense(slice_index si,
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
      init_param const * const param = st->param;
      slice_index const proxy_to_avoided = param->avoided_defense;
      slice_index const prototype = alloc_reflex_defender_filter(proxy_to_avoided);
      battle_branch_insert_slices(si,&prototype,1);
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

/* Prevent STReflex* slice insertion from recursing into the following
 * branch
 */
static void reflex_guards_inserter_branch_fork(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't traverse .avoided! */
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static structure_traversers_visitors reflex_guards_inserters[] =
{
  { STReadyForAttack,     &reflex_guards_inserter_defense     },
  { STReadyForDefense,    &reflex_guards_inserter_attack      },
  { STHelpFork,           &reflex_guards_inserter_branch_fork },
  { STSeriesFork,         &reflex_guards_inserter_branch_fork },
  { STGoalReachedTesting, &stip_structure_visitor_noop        }
};

enum
{
  nr_reflex_guards_inserters = (sizeof reflex_guards_inserters
                                / sizeof reflex_guards_inserters[0])
};

/* Instrument a branch with STReflex* slices for a (non-semi)
 * reflex stipulation
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided_attack identifies branch that the
 *                                attacker attempts to avoid
 * @param proxy_to_avoided_defense identifies branch that the
 *                                 defender attempts to avoid
 */
void slice_insert_reflex_filters(slice_index si,
                                 slice_index proxy_to_avoided_attack,
                                 slice_index proxy_to_avoided_defense)
{
  stip_structure_traversal st;
  init_param param = { proxy_to_avoided_defense, proxy_to_avoided_attack };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_avoided_attack);
  TraceFunctionParam("%u",proxy_to_avoided_defense);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(proxy_to_avoided_attack);
  TraceStipulation(proxy_to_avoided_defense);

  assert(slices[proxy_to_avoided_attack].type==STProxy);
  assert(slices[proxy_to_avoided_defense].type==STProxy);

  stip_structure_traversal_init(&st,&param);
  stip_structure_traversal_override(&st,
                                    reflex_guards_inserters,
                                    nr_reflex_guards_inserters);
  stip_traverse_structure(si,&st);

  {
    slice_index const prototype = alloc_reflex_attack_solver(proxy_to_avoided_attack);
    root_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In alternate play, insert a STReflexHelpFilter slice before a slice
 * where the reflex stipulation might force the side at the move to
 * reach the goal
 */
static void reflex_guards_inserter_help(slice_index si,
                                        stip_structure_traversal *st)
{
  init_param * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  /* in general, only one side is bound to deliver reflexmate */
  if ((length-slack_length_help)%2==0)
  {
    slice_index const proxy_to_avoided = param->avoided_defense;
    pipe_append(slices[si].prev,alloc_reflex_help_filter(proxy_to_avoided));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* In series play, insert a STReflexSeriesFilter slice before a slice where
 * the reflex stipulation might force the side at the move to reach
 * the goal
 */
static void reflex_guards_inserter_series(slice_index si,
                                          stip_structure_traversal *st)
{
  init_param * const param = st->param;
  slice_index const proxy_to_avoided = param->avoided_defense;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (!ready_for_series_move_is_move_dummy(si))
    pipe_append(slices[si].prev,alloc_reflex_series_filter(proxy_to_avoided));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors reflex_guards_inserters_semi[] =
{
  { STReadyForAttack,     &reflex_guards_inserter_defense     },
  { STHelpAdapter,        &reflex_guards_inserter_help        },
  { STReadyForHelpMove,   &reflex_guards_inserter_help        },
  { STHelpFork,           &reflex_guards_inserter_branch_fork },
  { STSeriesAdapter,      &reflex_guards_inserter_series      },
  { STReadyForSeriesMove, &reflex_guards_inserter_series      },
  { STSeriesFork,         &reflex_guards_inserter_branch_fork }
};

enum
{
  nr_reflex_guards_inserters_semi = (sizeof reflex_guards_inserters_semi
                                     / sizeof reflex_guards_inserters_semi[0])
};

/* Instrument a branch with STReflex* slices for a semi-reflex
 * stipulation
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided identifies branch that needs to be guarded from
 */
void slice_insert_reflex_filters_semi(slice_index si,
                                      slice_index proxy_to_avoided)
{
  stip_structure_traversal st;
  init_param param = { proxy_to_avoided, no_slice };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_avoided);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(slices[proxy_to_avoided].type==STProxy);

  stip_structure_traversal_init(&st,&param);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTesting,
                                           &stip_structure_visitor_noop);
  stip_structure_traversal_override(&st,
                                    reflex_guards_inserters_semi,
                                    nr_reflex_guards_inserters_semi);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_reflex_filter(slice_index branch,
                                           stip_structure_traversal *st)
{
  slice_index const avoided = slices[branch].u.reflex_guard.avoided;
  stip_traverse_structure_pipe(branch,st);
  stip_traverse_structure(avoided,st);
}
