#include "conditions/check_zigzag.h"
#include "stipulation/if_then_else.h"
#include "stipulation/branch.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/check/reached_tester.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a battle branch with a STIfThenElse slice providing a
 * shortcut for the defense moe
 * @param adapter identifies adapter slice into the battle branch
 */
void battle_branch_insert_defense_check_zigzag(slice_index adapter)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,
                                                adapter,
                                                stip_traversal_context_intro);
    slice_index const deadend = branch_find_slice(STDeadEnd,
                                                  ready,
                                                  stip_traversal_context_defense);
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const dummy = alloc_dummy_move_slice();
    slice_index const played = alloc_defense_played_slice();
    slice_index const condition = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const jump = alloc_if_then_else_slice(proxy1,proxy2,condition);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    assert(deadend!=no_slice);
    pipe_link(condition,alloc_true_slice());
    defense_branch_insert_slices(ready,&landing_proto,1);
    pipe_link(proxy2,slices[deadend].next1);
    /* the dummy move is needed to make sure that the killer move mechanism
     * applies to the same play whether the attacker has delivered check or not
     * TODO only insert the dummy move if the killer move optimisation is used
     */
    pipe_link(proxy1,dummy);
    pipe_link(dummy,played);
    pipe_link(deadend,jump);

    {
      slice_index const landing = branch_find_slice(STCheckZigzagLanding,
                                                    deadend,
                                                    stip_traversal_context_defense);
      assert(landing!=no_slice);
      link_to_branch(played,landing);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a help branch with a STIfThenElse slice
 * @param adapter identifies adapter slice into the help branch
 */
void help_branch_insert_check_zigzag(slice_index adapter)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    unsigned int const parity = 0;
    slice_index const ready = help_branch_locate_ready(adapter,parity);
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const played = alloc_help_move_played_slice();
    slice_index const condition = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const jump = alloc_if_then_else_slice(proxy1,proxy2,condition);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    pipe_link(condition,alloc_true_slice());
    help_branch_insert_slices(ready,&landing_proto,1);
    pipe_link(proxy2,slices[ready].next1);
    pipe_link(proxy1,played);
    pipe_link(ready,jump);

    {
      slice_index const landing = branch_find_slice(STCheckZigzagLanding,
                                                    ready,
                                                    stip_traversal_context_help);
      assert(landing!=no_slice);
      link_to_branch(played,landing);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
