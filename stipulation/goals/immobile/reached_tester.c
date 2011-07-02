#include "stipulation/goals/immobile/reached_tester.h"
#include "pydata.h"
#include "pyproc.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/true.h"
#include "stipulation/help_play/branch.h"
#include "conditions/ohneschach/immobility_tester.h"
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
    result = alloc_branch_fork(STGoalImmobileReachedTester,proxy);
    pipe_link(proxy,tester);
    link_to_branch(tester,alloc_help_branch(slack_length_help+1,
                                            slack_length_help+1));
    slices[result].u.immobility_tester.applies_to_who = starter_or_adversary;
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
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_pipe(si,st);

  {
    Side const immobilised = (slices[si].u.immobility_tester.applies_to_who
                              ==goal_applies_to_starter
                              ? slices[si].starter
                              : advers(slices[si].starter));
    *starter = immobilised;
    stip_traverse_structure(slices[si].u.immobility_tester.fork,st);
  }

  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean find_any_move(Side side)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side);

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play)
        && TraceCurrentMove(nbply)
        && !echecc(nbply,side))
      result = true;

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

  result = find_any_move(slices[si].starter) ? has_no_solution : has_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Is side immobile? */
boolean immobile(slice_index si, Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (CondFlag[ohneschach])
    result = ohneschach_immobile(side);
  else
    result = slice_has_solution(slices[si].u.immobility_tester.fork)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
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
  Side const immobilised = (slices[si].u.immobility_tester.applies_to_who
                            ==goal_applies_to_starter
                            ? slices[si].starter
                            : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (immobile(si,immobilised))
    result = slice_has_solution(slices[si].u.immobility_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
