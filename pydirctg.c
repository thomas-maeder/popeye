#include "pydirctg.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/goals.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/ready_for_attack.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "trace.h"

#include <assert.h>

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 * @param append_deadend should we append a deadend after the fork to
 *                       proxy_to_goal
 */
void battle_branch_set_direct_goal_branch(slice_index si,
                                          slice_index proxy_to_goal,
                                          boolean append_deadend)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParam("%u",append_deadend);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(proxy_to_goal);

  assert(slices[proxy_to_goal].type==STProxy);

  {
    if (append_deadend)
    {
      slice_index const prototypes[] =
      {
        alloc_end_of_branch_goal(proxy_to_goal),
        alloc_dead_end_slice()
      };
      enum {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      battle_branch_insert_slices(si,prototypes,nr_prototypes);
    }
    else
    {
      slice_index const prototypes[] =
      {
        alloc_end_of_branch_goal(proxy_to_goal)
      };
      enum {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      battle_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a the appropriate proxy slices before each STGoal*ReachedTester slice
 * @param si identifies STGoal*ReachedTester slice
 * @param st address of structure representing the traversal
 */
static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
      alloc_defense_adapter_slice(slack_length_battle,slack_length_battle-1)
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
                                           STGoalReachedTesting,
                                           &instrument_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
