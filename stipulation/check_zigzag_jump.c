#include "stipulation/check_zigzag_jump.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STCheckZigzagJump slice.
 * @param shortcut identifies entry slice of shortcut
 * @return index of allocated slice
 */
static slice_index alloc_check_zigzag_jump_slice(slice_index op1, slice_index op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result =  alloc_binary_slice(STCheckZigzagJump,op1,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type check_zigzag_jump_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index succ;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  succ = echecc(nbply,slices[si].starter) ? op1 : op2;
  result = defend(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type check_zigzag_jump_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index succ;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  succ = echecc(nbply,slices[si].starter) ? op1 : op2;
  result = attack(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a battle branch with a STCheckZigzagJump slice providing a
 * shortcut for the defense moe
 * @param adapter identifies adapter slice into the battle branch
 */
void battle_branch_insert_defense_check_zigzag(slice_index adapter)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,adapter);
    slice_index const deadend = branch_find_slice(STDeadEnd,ready);
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const dummy = alloc_dummy_move_slice();
    slice_index const played = alloc_move_played_slice();
    slice_index const jump = alloc_check_zigzag_jump_slice(proxy1,proxy2);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    assert(deadend!=no_slice);
    defense_branch_insert_slices(ready,&landing_proto,1);
    pipe_link(proxy1,slices[deadend].u.pipe.next);
    pipe_link(proxy2,dummy);
    pipe_link(dummy,played);
    pipe_link(deadend,jump);

    {
      slice_index const landing = branch_find_slice(STCheckZigzagLanding,deadend);
      assert(landing!=no_slice);
      link_to_branch(played,landing);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a help branch with a STCheckZigzagJump slice
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
    slice_index const jump = alloc_check_zigzag_jump_slice(proxy1,proxy2);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    help_branch_insert_slices(ready,&landing_proto,1);
    pipe_link(proxy1,slices[ready].u.pipe.next);
    pipe_link(proxy2,played);
    pipe_link(ready,jump);

    {
      slice_index const landing = branch_find_slice(STCheckZigzagLanding,ready);
      assert(landing!=no_slice);
      link_to_branch(played,landing);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
