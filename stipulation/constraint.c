#include "stipulation/constraint.h"
#include "pypipe.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/solving.h"
#include "trace.h"

#include <assert.h>


/* Allocate a STConstraintSolver slice
 * @param proxy_to_condition prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_solver_slice(slice_index proxy_to_condition)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_condition);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STConstraintSolver,proxy_to_condition);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STConstraintTester slice
 * @param proxy_to_condition prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_tester_slice(slice_index proxy_to_condition)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_condition);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STConstraintTester,proxy_to_condition);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin a STContraintSolver slice off a STContraintTester sliceto add it to the
 * root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void constraint_tester_make_root(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  if (state->spun_off[slices[si].u.pipe.next]!=no_slice)
  {
    state->spun_off[si] = alloc_constraint_solver_slice(stip_deep_copy(slices[si].u.fork.fork));
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
  }

  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a constraint solver slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_constraint_solver(slice_index si,
                                             stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    state->spun_off[si] = alloc_constraint_tester_slice(slices[si].u.fork.fork);
    stip_traverse_structure_children(si,st);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
    slices[state->spun_off[si]].u.fork.fork = state->spun_off[slices[si].u.fork.fork];
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type constraint_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = can_attack(next,n);
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
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type constraint_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (slice_solve(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_solution:
      result = attack(next,n);
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

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type constraint_can_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_no_solution:
      result = n+4;
      break;

    case has_solution:
      result = can_defend(next,n);
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

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
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type constraint_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_solution:
      result = defend(next,n);
      break;

    case has_no_solution:
      result = n+4;
      break;

    default:
      assert(0);
      result = n+4;
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
stip_length_type constraint_solver_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  switch (slice_solve(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = help(next,n);
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

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
stip_length_type constraint_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = can_help(next,n);
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type constraint_tester_solve(slice_index si)
{
  has_solution_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(condition);
  if (result==has_solution)
    result = slice_solve(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type constraint_tester_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(condition);
  if (result==has_solution)
    result = slice_has_solution(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type constraint_tester_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = slack_length_battle-2;
      break;

    case has_solution:
      result = attack(next,n);
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
stip_length_type constraint_tester_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.fork.fork;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = help(next,n);
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_constraint_irrelevant(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (branch_find_slice(STAttackAdapter,si)==no_slice
            && branch_find_slice(STDefenseAdapter,si)==no_slice
            && branch_find_slice(STHelpAdapter,si)==no_slice
            && branch_find_slice(STGoalReachedTester,si)!=no_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_constraint_if_irrelevant(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_constraint_irrelevant(slices[si].u.fork.fork))
  {
    stip_traverse_structure_pipe(si,st);
    dealloc_slices(slices[si].u.fork.fork);
    pipe_remove(si);
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors unsatisfiable_goal_checker_removers[] =
{
  { STConstraintTester,  &remove_constraint_if_irrelevant },
  { STConstraintSolver,  &remove_constraint_if_irrelevant },
  { STReadyForAttack,    &stip_structure_visitor_noop     },
  { STReadyForDefense,   &stip_structure_visitor_noop     },
  { STReadyForHelpMove,  &stip_structure_visitor_noop     },
  { STReadyForDummyMove, &stip_structure_visitor_noop     }
};

enum
{
  nr_unsatisfiable_goal_checker_removers =
    (sizeof unsatisfiable_goal_checker_removers
     / sizeof unsatisfiable_goal_checker_removers[0])
};

/* Remove goal checker slices that we know can't possibly be met
 * @param si identifies entry slice to stipulation
 */
void stip_remove_irrelevant_constraints(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    unsatisfiable_goal_checker_removers,
                                    nr_unsatisfiable_goal_checker_removers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

}
