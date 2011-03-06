#include "stipulation/battle_play/fork.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "trace.h"


/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_battle_fork(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  stip_traverse_structure(slices[si].u.branch_fork.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a battle fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_battle_fork(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<=slack_length_battle+1)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);

  if (st->remaining>slack_length_battle)
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
