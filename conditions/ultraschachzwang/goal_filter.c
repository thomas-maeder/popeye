#include "conditions/ultraschachzwang/goal_filter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pydata.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "debugging/trace.h"

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
stip_length_type ultraschachzwang_goal_filter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  Cond const cond = (starter==White
                     ? whiteultraschachzwang
                     : blackultraschachzwang);
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(CondFlag[cond]);
  CondFlag[cond] = false;
  result = solve(next,n);
  CondFlag[cond] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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

  stip_traverse_structure_children_pipe(si,st);

  if (CondFlag[cond])
    pipe_append(slices[si].prev,alloc_ultraschachzwang_goal_filter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_ultraschachzwang_legality_tester_slice();
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor ultraschachzwang_filter_inserters[] =
{
  /* someone seems to have defined that Ultraschachzwang doesn't count when
   * looking for SAT flights */
  { STSATFlightsCounterFork, &stip_traverse_structure_children_pipe },
  { STMove,                  &instrument_move                       },
  { STGoalMateReachedTester, &prepend_mate_filter                   }
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
