#include "solving/avoid_unsolvable.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

/* maximum number of half-moves that we know have no solution
 */
stip_length_type max_unsolvable;

/* Allocate a STEndOfBranch slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_avoid_unsolvable_slice(slice_index proxy_op1,
                                         slice_index proxy_op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_op1);
  TraceFunctionParam("%u",proxy_op2);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STAvoidUnsolvable,proxy_op1,proxy_op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_reset_unsolvable_attack(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_reset_unsolvable_slice();
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_reset_unsolvable_defense(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototype = alloc_reset_unsolvable_slice();
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_reset_unsolvable_help(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_reset_unsolvable_slice();
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_avoid_unsolvable(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_attack
      || st->context==stip_traversal_context_defense)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_avoid_unsolvable_slice(proxy1,proxy2);
    pipe_link(slices[si].prev,fork);
    pipe_append(si,proxy2);
    pipe_link(proxy1,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const avoid_unusable_inserters[] =
{
  { STAttackAdapter,         &insert_reset_unsolvable_attack  },
  { STDefenseAdapter,        &insert_reset_unsolvable_attack  },
  { STHelpAdapter,           &insert_reset_unsolvable_help    },
  { STAttackPlayed,          &insert_reset_unsolvable_defense },
  { STDefensePlayed,         &insert_reset_unsolvable_defense },
  { STEndOfBranchGoal,       &insert_avoid_unsolvable         },
  { STCounterMateFilter,     &insert_avoid_unsolvable         },
  { STDoubleMateFilter,      &insert_avoid_unsolvable         },
  { STPrerequisiteOptimiser, &insert_avoid_unsolvable         }
};

enum
{
  nr_avoid_unusable_inserters = (sizeof avoid_unusable_inserters
                                 / sizeof avoid_unusable_inserters[0])
};

/* Instrument STEndOfBranch* slices with the necessary STAvoidUnusable slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_avoid_unsolvable_forks(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_end_of_branch,
                                                &insert_avoid_unsolvable);
  stip_structure_traversal_override(&st,
                                    avoid_unusable_inserters,
                                    nr_avoid_unusable_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type avoid_unsolvable_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  TraceValue("%u\n",max_unsolvable);
  if (max_unsolvable<=slack_length)
    result = solve(slices[si].next1,n);
  else
    result = solve(slices[si].next2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STResetUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_reset_unsolvable_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STResetUnsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type reset_unsolvable_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  max_unsolvable = slack_length;
  TraceValue("->%u\n",max_unsolvable);

  result = solve(slices[si].next1,n);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u\n",max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STLearnUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_learn_unsolvable_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLearnUnsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type learn_unsolvable_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(slices[si].next1,n);

  if (result>n)
  {
    max_unsolvable = n;
    TraceValue("->%u\n",max_unsolvable);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
