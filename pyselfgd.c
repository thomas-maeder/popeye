#include "pyselfgd.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

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
  { STGoalReachedTester, &stip_traverse_structure_pipe },
  { STLeaf,              &instrument_leaf              }
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
