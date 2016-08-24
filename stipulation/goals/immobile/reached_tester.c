#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/fork.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/true.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Allocate a system of slices that tests whether the side to be immobilised has
 * been
 * @return index of entry slice
 */
slice_index alloc_goal_immobile_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
  pipe_link(result,alloc_true_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalImmobileReachedTester slice.
 * @param starter_or_adversary is the starter immobilised or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const tester = alloc_pipe(STImmobilityTester);
    result = alloc_conditional_pipe(STGoalImmobileReachedTester,proxy);
    pipe_link(proxy,tester);
    link_to_branch(tester,alloc_defense_branch(1,1));
    SLICE_U(result).goal_filter.applies_to_who = starter_or_adversary;
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
void impose_starter_goal_immobile_tester(slice_index si,
                                         stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,*starter);
  TraceFunctionParamListEnd();

  SLICE_STARTER(si) = *starter;
  stip_traverse_structure_children_pipe(si,st);

  {
    Side const immobilised = (SLICE_U(si).goal_filter.applies_to_who
                              ==goal_applies_to_starter
                              ? SLICE_STARTER(si)
                              : advers(SLICE_STARTER(si)));
    *starter = immobilised;
    stip_traverse_structure_conditional_pipe_tester(si,st);
  }

  *starter = SLICE_STARTER(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
