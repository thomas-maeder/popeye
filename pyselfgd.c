#include "pyselfgd.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/min_length_guard.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Insert a STEndOfBranchGoal after each STDefenseMove
 */
static void self_guards_inserter_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;

    {
      slice_index const * const proxy_to_goal = st->param;
      slice_index const prototypes[] =
      {
          alloc_end_of_branch_goal(*proxy_to_goal)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      battle_branch_insert_slices(si,prototypes,nr_prototypes);
    }

    if (min_length>slack_length_battle+1)
    {
      slice_index const prototype = alloc_min_length_guard(length-1,
                                                           min_length-1);
      battle_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch for detecting whether the defense was forced to reach a
 * goal
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 */
void slice_insert_self_guards(slice_index si, slice_index proxy_to_goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(slices[proxy_to_goal].type==STProxy);

  stip_structure_traversal_init(&st,&proxy_to_goal);
  stip_structure_traversal_override_single(&st,
                                           STReadyForDefense,
                                           &self_guards_inserter_defense);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a the appropriate proxy slices before each STLeaf slice
 * @param si identifies STLeaf slice
 * @param st address of structure representing the traversal
 */
static void instrument_leaf(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_append(slices[si].prev,
              alloc_attack_adapter_slice(slack_length_battle,
                                         slack_length_battle-2));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors direct_leaf_instrumenters[] =
{
  { STLeaf, &instrument_leaf }
};

enum
{
  nr_direct_leaf_instrumenters = (sizeof direct_leaf_instrumenters
                                  / sizeof direct_leaf_instrumenters[0])
};

/* Instrument a branch leading to a goal to be a self goal branch
 * @param si identifies entry slice of branch
 */
void slice_make_self_goal_branch(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    direct_leaf_instrumenters,
                                    nr_direct_leaf_instrumenters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
