#include "conditions/ultraschachzwang/goal_filter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pydata.h"
#include "conditions/conditions.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "debugging/trace.h"

#include <assert.h>

static void instrument_move(slice_index si, stip_structure_traversal *st)
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
  {
    slice_index const prototype = alloc_ultraschachzwang_legality_tester_slice();
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor ultraschachzwang_enforcer_inserters[] =
{
  { STGoalMateReachedTester, &stip_structure_visitor_noop           },
  { STMove,                  &instrument_move                       }
};

enum
{
  nr_ultraschachzwang_enforcer_inserters =
      (sizeof ultraschachzwang_enforcer_inserters
       / sizeof ultraschachzwang_enforcer_inserters[0])
};

/* Instrument the solving machinery with Ultraschachzwang
 * @param si root of branch to be instrumented
 */
void ultraschachzwang_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    ultraschachzwang_enforcer_inserters,
                                    nr_ultraschachzwang_enforcer_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
