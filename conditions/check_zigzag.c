#include "conditions/check_zigzag.h"
#include "stipulation/if_then_else.h"
#include "stipulation/branch.h"
#include "stipulation/move_played.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/boolean/true.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/goals/check/reached_tester.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
    slice_index const dummy = alloc_pipe(STDummyMove);
    slice_index const played = alloc_defense_played_slice();
    slice_index const condition = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const jump = alloc_if_then_else_slice(proxy1,proxy2,condition);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    assert(deadend!=no_slice);
    pipe_link(condition,alloc_true_slice());
    defense_branch_insert_slices(ready,&landing_proto,1);
    pipe_link(proxy2,SLICE_NEXT1(deadend));
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

static slice_index help_branch_locate_ready(slice_index si)
{
  slice_index result = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = branch_find_slice(STReadyForHelpMove,result,stip_traversal_context_help);
  assert(result!=no_slice);

  while ((SLICE_U(result).branch.length-slack_length)%2!=0)
  {
    slice_index const next = branch_find_slice(STReadyForHelpMove,
                                               result,
                                               stip_traversal_context_help);
    assert(next!=no_slice);
    if (result==next)
    {
      result = no_slice;
      break;
    }
    else
      result = next;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    slice_index const ready = help_branch_locate_ready(adapter);
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const played = alloc_help_move_played_slice();
    slice_index const condition = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const jump = alloc_if_then_else_slice(proxy1,proxy2,condition);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    pipe_link(condition,alloc_true_slice());
    help_branch_insert_slices(ready,&landing_proto,1);
    pipe_link(SLICE_PREV(ready),jump);
    pipe_link(proxy2,ready);
    pipe_link(proxy1,played);

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
