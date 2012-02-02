#include "stipulation/goals/immobile/reached_tester.h"
#include "pydata.h"
#include "pybrafrk.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/true.h"
#include "stipulation/help_play/branch.h"
#include "solving/legal_move_counter.h"
#include "trace.h"

#include <assert.h>

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
    link_to_branch(tester,
                   alloc_help_branch(slack_length_help+1,slack_length_help+1));
    slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

    {
      slice_index const prototype = alloc_legal_move_counter_slice();
      branch_insert_slices(tester,&prototype,1);
    }
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
void impose_starter_immobility_tester(slice_index si,
                                      stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,*starter,"");
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_pipe(si,st);

  {
    Side const immobilised = (slices[si].u.goal_filter.applies_to_who
                              ==goal_applies_to_starter
                              ? slices[si].starter
                              : advers(slices[si].starter));
    *starter = immobilised;
    stip_traverse_structure(slices[si].u.goal_filter.tester,st);
  }

  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type immobility_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* avoid concurrent counts */
  assert(legal_move_counter_count[nbply+1]==0);

  /* stop counting once we have >1 legal king moves */
  legal_move_counter_interesting[nbply+1] = 0;

  slice_has_solution(slices[si].u.pipe.next);

  result = legal_move_counter_count[nbply+1]==0 ? has_solution : has_no_solution;

  /* clean up after ourselves */
  legal_move_counter_count[nbply+1] = 0;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_immobile_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_has_solution(slices[si].u.goal_filter.tester)==has_solution)
    result = slice_has_solution(slices[si].u.goal_filter.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
