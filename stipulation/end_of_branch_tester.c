#include "stipulation/end_of_branch_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "solving/fork_on_remaining.h"
#include "trace.h"

#include <assert.h>

static void end_of_branch_tester_inserter_end_of_branch(slice_index si,
                                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const proxy3 = alloc_proxy_slice();
    slice_index const end_of_branch = alloc_conditional_pipe(STEndOfBranchTester,
                                                             slices[si].u.fork.fork);
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
    pipe_link(slices[si].prev,fork);
    pipe_append(si,proxy3);
    pipe_link(proxy1,end_of_branch);
    pipe_link(proxy2,si);
    pipe_set_successor(end_of_branch,proxy3);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const end_of_branch_tester_inserters[] =
{
  { STEndOfBranchForced, &end_of_branch_tester_inserter_end_of_branch },
  { STEndOfBranchGoal,   &end_of_branch_tester_inserter_end_of_branch }
};

enum
{
  nr_end_of_branch_tester_inserters = (sizeof end_of_branch_tester_inserters
                                       / sizeof end_of_branch_tester_inserters[0])
};

/* Instrument STEndOfBranchGoal (and STEndOfBranchForced) slices with the
 * necessary STEndOfBranchTester slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_end_of_branch_testers(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    end_of_branch_tester_inserters,
                                    nr_end_of_branch_tester_inserters);
  stip_traverse_structure(root_slice,&st);

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
stip_length_type end_of_branch_tester_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.conditional_pipe.condition;
  slice_index const next = slices[si].u.conditional_pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help+2);

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = help(next,n);
      break;

    case has_solution:
      result = n;
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
stip_length_type end_of_branch_tester_can_help(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const condition = slices[si].u.conditional_pipe.condition;
  slice_index const next = slices[si].u.conditional_pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help+2);

  switch (slice_has_solution(condition))
  {
    case opponent_self_check:
      result = n+4;
      break;

    case has_no_solution:
      result = can_help(next,n);
      break;

    case has_solution:
      result = n;
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
