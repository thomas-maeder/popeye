#include "stipulation/end_of_branch_tester.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/conditional_pipe.h"
#include "solving/solving.h"
#include "solving/fork_on_remaining.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STEndOfBranchTester slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_end_of_branch_tester_slice(slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STEndOfBranchTester,to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STEndOfBranchGoalTester slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
static slice_index alloc_end_of_branch_goal_tester_slice(slice_index to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STEndOfBranchGoalTester,to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
    slice_index const end_of_branch = alloc_end_of_branch_tester_slice(slices[si].u.fork.fork);
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

static void end_of_branch_tester_inserter_end_of_branch_goal(slice_index si,
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
    slice_index const end_of_branch = alloc_end_of_branch_goal_tester_slice(slices[si].u.fork.fork);
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
  { STEndOfBranchForced, &end_of_branch_tester_inserter_end_of_branch      },
  { STEndOfBranchGoal,   &end_of_branch_tester_inserter_end_of_branch_goal }
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

/* Callback to stip_spin_off_testers
 * Spin a tester slice off an end of a branch tester slice
 * @param si identifies the branch tester slice
 * @param st address of structure representing traversal
 */
void start_spinning_off_end_of_branch_tester(slice_index si,
                                             stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->spinning_off)
  {
    state->spun_off[si] = copy_slice(si);
    stip_traverse_structure_children_pipe(si,st);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
    slices[state->spun_off[si]].u.fork.fork = state->spun_off[slices[si].u.fork.fork];
  }
  else
  {
    stip_traverse_structure_children_pipe(si,st);

    state->spinning_off = true;
    stip_traverse_structure_next_branch(si,st);
    state->spinning_off = false;
  }

  slices[si].u.fork.fork = state->spun_off[slices[si].u.fork.fork];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
