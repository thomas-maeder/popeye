#include "stipulation/battle_play/fork.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "trace.h"


/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_battle_fork(slice_index branch,
                                         stip_structure_traversal *st)
{
  stip_traverse_structure_pipe(branch,st);
  stip_traverse_structure(slices[branch].u.branch_fork.towards_goal,st);
}

/* Traversal of the moves beyond a series fork slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_battle_fork(slice_index si, stip_move_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==0)
  {
    st->full_length = slices[si].u.branch.length;
    TraceValue("->%u",st->full_length);
    st->remaining = slices[si].u.branch.length;
  }

  TraceValue("%u\n",st->remaining);
  if (st->remaining<=slack_length_battle+1)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
