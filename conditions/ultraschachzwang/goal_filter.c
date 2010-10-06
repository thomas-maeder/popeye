#include "conditions/ultraschachzwang/goal_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STUltraschachzwangGoalFilter
 */

/* Allocate a STUltraschachzwangGoalFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_ultraschachzwang_goal_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STUltraschachzwangGoalFilter);

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
has_solution_type ultraschachzwang_goal_filter_has_solution(slice_index si)
{
  has_solution_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = slice_has_solution(next);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ultraschachzwang_goal_filter_solve(slice_index si)
{
  has_solution_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = slice_solve(next);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

static void prepend_mate_filter(slice_index si, stip_structure_traversal *st)
{
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (CondFlag[cond])
    pipe_append(slices[si].prev,alloc_ultraschachzwang_goal_filter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors ultraschachzwang_filter_inserters[] =
{
  { STGoalMateReachedTester, &prepend_mate_filter }
};

enum
{
  nr_ultraschachzwang_filter_inserters =
      (sizeof ultraschachzwang_filter_inserters
       / sizeof ultraschachzwang_filter_inserters[0])
};

/* Instrument a stipulation with Ultraschachzwang goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_ultraschachzwang_goal_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    ultraschachzwang_filter_inserters,
                                    nr_ultraschachzwang_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
