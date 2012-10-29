#include "stipulation/end_of_branch_tester.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/conditional_pipe.h"
#include "solving/fork_on_remaining.h"
#include "debugging/trace.h"

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

static void help_insert_detour_with_tester(slice_index si,
                                           stip_structure_traversal *st,
                                           slice_index tester)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const proxy3 = alloc_proxy_slice();
    /* avoid writing short solutions when looking for longer ones*/
    pipe_link(slices[si].prev,alloc_fork_on_remaining_slice(proxy1,proxy2,1));
    pipe_append(si,proxy3);
    pipe_link(proxy1,tester);
    pipe_link(proxy2,si);
    pipe_set_successor(tester,proxy3);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_detour_with_tester(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help)
  {
    slice_index const to_next_branch = slices[si].next2;
    slice_index const to_next_branch_tester = slices[to_next_branch].tester;
    slice_index const tester = alloc_end_of_branch_tester_slice(to_next_branch_tester);
    help_insert_detour_with_tester(si,st,tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_detour_with_tester_goal(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help)
  {
    slice_index const to_next_branch = slices[si].next2;
    slice_index const to_next_branch_tester = slices[to_next_branch].tester;
    slice_index const tester = alloc_end_of_branch_goal_tester_slice(to_next_branch_tester);
    help_insert_detour_with_tester(si,st,tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_detour(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_help)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
    pipe_link(slices[si].prev,fork);
    pipe_append(si,proxy1);
    pipe_link(proxy2,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const end_of_branch_tester_inserters[] =
{
  { STEndOfBranch,             &insert_detour                  },
  { STEndOfBranchGoalImmobile, &insert_detour                  },
  { STEndOfBranchForced,       &insert_detour_with_tester      },
  { STEndOfBranchGoal,         &insert_detour_with_tester_goal }
};

enum
{
  nr_end_of_branch_tester_inserters = (sizeof end_of_branch_tester_inserters
                                       / sizeof end_of_branch_tester_inserters[0])
};

/* Instrument STEndOfBranch* slices in help play in order to
 * - avoid writing short solutions when looking for longer ones
 * - avoid going on solving if a non-immobilising goal has been reached
 * @param root_slice identifes root slice of stipulation
 */
void stip_instrument_help_ends_of_branches(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    end_of_branch_tester_inserters,
                                    nr_end_of_branch_tester_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
