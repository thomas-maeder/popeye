#include "conditions/circe/steingewinn_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STCirceSteingewinnFilter
 */

/* Allocate a STCirceSteingewinnFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_circe_steingewinn_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCirceSteingewinnFilter);

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
has_solution_type circe_steingewinn_filter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sqrenais[nbply]==initsquare)
    result = slice_has_solution(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type circe_steingewinn_filter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sqrenais[nbply]==initsquare)
    result = slice_solve(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

static void prepend_goal_filter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.goal_reached_tester.goal.type==goal_steingewinn)
    pipe_append(slices[si].prev,alloc_circe_steingewinn_filter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors circe_goal_filter_inserters[] =
{
  { STGoalReachedTester, &prepend_goal_filter }
};

enum
{
  nr_circe_goal_filter_inserters = (sizeof circe_goal_filter_inserters
                                    / sizeof circe_goal_filter_inserters[0])
};

/* Instrument a stipulation with AMU mate filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_circe_steingewinn_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    circe_goal_filter_inserters,
                                    nr_circe_goal_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
