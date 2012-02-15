#include "pydirctg.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "trace.h"

#include <assert.h>

/* Insert a the appropriate proxy slices before each STGoal*ReachedTester slice
 * @param si identifies STGoal*ReachedTester slice
 * @param st address of structure representing the traversal
 */
static void instrument_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
      alloc_defense_adapter_slice(slack_length,slack_length-1),
      alloc_branch(STReadyForDefense,slack_length,slack_length-1)
    };
    enum {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    leaf_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch leading to a goal to be a direct goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_direct_goal_branch(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &instrument_tester);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
