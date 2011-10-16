#include "optimisations/intelligent/mate/filter.h"
#include "pyint.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/any/reached_tester.h"
#include "solving/legal_move_finder.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STIntelligentMateFilter slice.
 * @param goal_tester_fork fork into the goal goal_tester_fork branch
 * @return allocated slice
 */
slice_index alloc_intelligent_mate_filter(slice_index goal_tester_fork)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal_tester_fork);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy_branch = alloc_proxy_slice();
    slice_index const help = alloc_help_branch(slack_length_help+1,
                                               slack_length_help+1);
    slice_index const proto = alloc_legal_move_finder_slice();
    slice_index const proxy_goal = alloc_proxy_slice();
    slice_index const system = alloc_goal_any_reached_tester_system();
    link_to_branch(proxy_goal,system);
    help_branch_set_end_goal(help,proxy_goal,1);
    branch_insert_slices(proxy_goal,&proto,1);
    link_to_branch(proxy_branch,help);
    result = alloc_branch_fork(STIntelligentMateFilter,proxy_branch);
    slices[result].u.intelligent_mate_filter.goal_tester_fork = goal_tester_fork;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_intelligent_mate_filter(slice_index si,
                                            stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_pipe(si,st);

  /* in duplexes, the colors swapped when looking for the "black solutions".
   * we thus have to make sure that the legal move finder always tests for
   * Black.
   */
  *starter = Black;
  stip_traverse_structure(slices[si].u.fork.fork,st);
  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean Intelligent(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  solutions_found = false;
  current_start_slice = si;
  intelligent_duplicate_avoider_init();
  IntelligentRegulargoal_types(n);
  intelligent_duplicate_avoider_cleanup();
  current_start_slice = no_slice;
  result = solutions_found;

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
stip_length_type intelligent_mate_filter_help(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = Intelligent(si,n) ? n : n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
