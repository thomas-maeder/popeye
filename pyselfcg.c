#include "pyselfcg.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/goals.h"
#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pyproc.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>


/* Allocate a STSelfCheckGuard slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSelfCheckGuard);

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
stip_length_type selfcheck_guard_attack(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = slack_length_battle-2;
  else
    result = attack(slices[si].u.pipe.next,n,n_max_unsolvable);

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
stip_length_type selfcheck_guard_can_attack(slice_index si,
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

  if (echecc(nbply,advers(slices[si].starter)))
    result = slack_length_battle-2;
  else
    result = can_attack(next,n,n_max_unsolvable);

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
stip_length_type selfcheck_guard_defend(slice_index si,
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

  if (echecc(nbply,advers(slices[si].starter)))
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
stip_length_type selfcheck_guard_can_defend(slice_index si,
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

  if (echecc(nbply,advers(slices[si].starter)))
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
stip_length_type selfcheck_guard_help(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = n+2;
  else
    result = help(slices[si].u.pipe.next,n);

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
stip_length_type selfcheck_guard_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (echecc(nbply,advers(slices[si].starter)))
    result = n+2;
  else
    result = can_help(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at
 * @param si slice index
 * @return true iff >=1 solution was found
 */
has_solution_type selfcheck_guard_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = opponent_self_check;
  else
    result = slice_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type selfcheck_guard_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = opponent_self_check;
  else
    result = slice_has_solution(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

static
void insert_selfcheck_guard_battle_branch(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length_battle)
  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_selfcheck_guard_help_branch(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    help_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_self_check_ignorance(slice_index si, stip_structure_traversal *st)
{
  boolean * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.goal_filter.applies_to_who==goal_applies_to_adversary)
    *result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a goal ignores selfcheck
 * @param goal_reached_tester identifies STGoalReachedTester slice
 * @return true iff the tested goal ignores selfcheck
 */
static boolean does_goal_ignore_selfcheck(slice_index goal_reached_tester)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal_reached_tester);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalCheckReachedTester,
                                           &test_self_check_ignorance);
  stip_traverse_structure(goal_reached_tester,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_selfcheck_guard_constraint(slice_index si,
                                              stip_structure_traversal *st)
{
  boolean * const in_constraint = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *in_constraint = true;
  stip_traverse_structure_next_branch(si,st);
  *in_constraint = false;

  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_selfcheck_guard_goal(slice_index si,
                                        stip_structure_traversal *st)
{
  boolean const * const in_constraint = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (!does_goal_ignore_selfcheck(slices[si].u.fork.fork))
  {
    slice_index const fork = slices[si].u.goal_tester.fork;
    slice_index const not_slice = branch_find_slice(STNot,fork);
    if (not_slice==no_slice || *in_constraint)
    {
      if (slices[si].u.goal_tester.goal.type==goal_dblstale)
        goal_doublestalemate_insert_selfcheck_guard(si);
      else
      {
        slice_index const prototype = alloc_selfcheck_guard_slice();
        goal_branch_insert_slices(fork,&prototype,1);
      }
    }
    else
    {
      /* make sure that not_slice doesn't convert has_no_solution into
       * has_solution if the last move was a self-check
       */
      slice_index const proxy_regular = alloc_proxy_slice();
      slice_index const proxy_selfcheck = alloc_proxy_slice();
      slice_index const guard = alloc_selfcheck_guard_slice();
      slice_index const leaf_selfcheck = alloc_true_slice();
      pipe_append(not_slice,proxy_regular);
      pipe_link(not_slice,alloc_and_slice(proxy_selfcheck,proxy_regular));
      pipe_link(proxy_selfcheck,guard);
      pipe_link(guard,leaf_selfcheck);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_selfcheck_guard_check_zigzag(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const guard = branch_find_slice(STSelfCheckGuard,
                                                slices[si].u.binary.op2);
    if (guard!=no_slice)
      pipe_remove(guard);
  }

  stip_traverse_structure(slices[si].u.binary.op1,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors in_branch_guards_inserters[] =
{
  { STReadyForAttack,    &insert_selfcheck_guard_battle_branch },
  { STReadyForDefense,   &insert_selfcheck_guard_battle_branch },
  { STReadyForHelpMove,  &insert_selfcheck_guard_help_branch   },
  { STReadyForDummyMove, &insert_selfcheck_guard_help_branch   },
  { STConstraint,        &insert_selfcheck_guard_constraint    },
  { STGoalReachedTester, &insert_selfcheck_guard_goal          },
  { STCheckZigzagJump,   &remove_selfcheck_guard_check_zigzag  },
  { STCounterMateFilter, &stip_traverse_structure_pipe         },
  { STIsardamDefenderFinder, &stip_traverse_structure_pipe     }
};

enum
{
  nr_in_branch_guards_inserters = (sizeof in_branch_guards_inserters
                                   / sizeof in_branch_guards_inserters[0])
};

static void insert_in_branch_guards(slice_index si)
{
  stip_structure_traversal st;
  boolean in_constraint = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&in_constraint);
  stip_structure_traversal_override(&st,
                                    in_branch_guards_inserters,
                                    nr_in_branch_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_immobile_reached_tester(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_in_branch_guards(slices[si].u.fork.fork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guards_in_immobility_testers(slice_index si)
{
  stip_structure_traversal st;
  boolean in_constraint = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&in_constraint);
  stip_structure_traversal_override_single(&st,
                                           STGoalImmobileReachedTester,
                                           &instrument_immobile_reached_tester);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  Side last_guarded_side;
  boolean guard_needed;
} insertion_state_type_adapter;

static void insert_selfcheck_guard_inverter(slice_index si,
                                            stip_structure_traversal *st)
{
  insertion_state_type_adapter * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->guard_needed = false;

  stip_traverse_structure_children(si,st);

  if (state->guard_needed)
  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    root_branch_insert_slices(si,&prototype,1);
    state->guard_needed = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_selfcheck_guard_adapter(slice_index si,
                                           stip_structure_traversal *st)
{
  insertion_state_type_adapter * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  state->guard_needed = state->last_guarded_side!=no_side && slices[si].starter!=state->last_guarded_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_checked_side(slice_index si,
                                        stip_structure_traversal *st)
{
  Side * const side = st->param;
  Side const save_side = *side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *side = slices[si].starter;
  stip_traverse_structure_children(si,st);
  *side = save_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors adapters_guards_inserters[] =
{
  { STAttackAdapter,     &insert_selfcheck_guard_adapter  },
  { STDefenseAdapter,    &insert_selfcheck_guard_adapter  },
  { STHelpAdapter,       &insert_selfcheck_guard_adapter  },
  { STSelfCheckGuard,    &remember_checked_side           },
  { STMoveInverter,      &insert_selfcheck_guard_inverter },
  { STTemporaryHackFork, &stip_traverse_structure_pipe    }
};

enum
{
  nr_adapters_guards_inserters = (sizeof adapters_guards_inserters
                                  / sizeof adapters_guards_inserters[0])
};

static void insert_selfcheck_guard_adapters(slice_index si)
{
  stip_structure_traversal st;
  insertion_state_type_adapter state = { no_side, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    adapters_guards_inserters,
                                    nr_adapters_guards_inserters);
  stip_traverse_structure(si,&st);

  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    root_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with slices dealing with selfcheck detection
 * @param si root of branch to be instrumented
 */
void stip_insert_selfcheck_guards(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  insert_in_branch_guards(si);
  insert_guards_in_immobility_testers(si);
  stip_impose_starter(si,slices[si].starter);
  insert_selfcheck_guard_adapters(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
