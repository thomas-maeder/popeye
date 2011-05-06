#include "pyreflxg.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/min_length_guard.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/series_play/branch.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>


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

  result = alloc_branch_fork(STReflexAttackerFilter,proxy_to_avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attacker_filter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const avoided = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_solve(avoided)==has_solution)
    result = slice_solve(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attacker_filter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const avoided = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_has_solution(avoided)==has_solution)
    result = slice_has_solution(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
reflex_attacker_filter_can_attack(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.fork.fork;
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
      result = can_attack(next,n,n_max_unsolvable);
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
reflex_attacker_filter_attack(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.fork.fork;
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
      result = attack(next,n,n_max_unsolvable);
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

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type reflex_attacker_filter_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = help(next,n);
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
stip_length_type reflex_attacker_filter_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = can_help(next,n);
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
stip_length_type reflex_attacker_filter_series(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = series(next,n);
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
stip_length_type reflex_attacker_filter_has_series(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  slice_index const avoided = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  /* TODO exact - but what does it mean??? */
  if (slice_has_solution(avoided)==has_solution)
    result = has_series(next,n);
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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

static void reflex_guards_inserter_attack_adapter(slice_index si,
                                                  stip_structure_traversal *st)
{
  init_param * const param = st->param;
  slice_index const proxy_to_avoided = param->avoided_attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(slices[si].prev,alloc_reflex_attacker_filter(proxy_to_avoided));

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
      slice_index const prototypes[] =
      {
          alloc_end_of_branch_forced(proxy_to_avoided),
          alloc_dead_end_slice()
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

static void reflex_guards_inserter_help_adapter(slice_index si,
                                                stip_structure_traversal *st)
{
  init_param * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((length-slack_length_help)%2==1)
  {
    slice_index const proxy_to_avoided = param->avoided_defense;
    pipe_append(slices[si].prev,alloc_reflex_attacker_filter(proxy_to_avoided));
  }
  else
  {
    slice_index const proxy_to_avoided = param->avoided_attack;
    slice_index const prototype = alloc_end_of_branch_forced(proxy_to_avoided);
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reflex_guards_inserter_help_move(slice_index si,
                                             stip_structure_traversal *st)
{
  init_param * const param = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((length-slack_length_help)%2==1)
  {
    slice_index const proxy_to_avoided = param->avoided_attack;
    slice_index const prototype = alloc_end_of_branch_forced(proxy_to_avoided);
    help_branch_insert_slices(si,&prototype,1);
  }
  else
  {
    slice_index const proxy_to_avoided = param->avoided_defense;
    slice_index const prototype = alloc_reflex_attacker_filter(proxy_to_avoided);
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reflex_guards_inserter_series_adapter(slice_index si,
                                                  stip_structure_traversal *st)
{
  init_param * const param = st->param;
  slice_index const proxy_to_avoided = param->avoided_defense;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(slices[si].prev,alloc_reflex_attacker_filter(proxy_to_avoided));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reflex_guards_inserter_series_move(slice_index si,
                                               stip_structure_traversal *st)
{
  init_param * const param = st->param;
  slice_index const proxy_to_avoided = param->avoided_attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_end_of_branch_forced(proxy_to_avoided),
        alloc_dead_end_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    series_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reflex_guards_inserter_series_dummy_move(slice_index si,
                                                     stip_structure_traversal *st)
{
  init_param * const param = st->param;
  slice_index const proxy_to_avoided = param->avoided_defense;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_reflex_attacker_filter(proxy_to_avoided);
    series_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors reflex_guards_inserters[] =
{
  { STAttackAdapter,           &reflex_guards_inserter_attack_adapter    },
  { STReadyForAttack,          &reflex_guards_inserter_defense           },
  { STReadyForDefense,         &reflex_guards_inserter_attack            },
  { STHelpAdapter,             &reflex_guards_inserter_help_adapter      },
  { STReadyForHelpMove,        &reflex_guards_inserter_help_move         },
  { STSeriesAdapter,           &reflex_guards_inserter_series_adapter    },
  { STReadyForSeriesMove,      &reflex_guards_inserter_series_move       },
  { STReadyForSeriesDummyMove, &reflex_guards_inserter_series_dummy_move }
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
void stip_insert_reflex_filters(slice_index si,
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
