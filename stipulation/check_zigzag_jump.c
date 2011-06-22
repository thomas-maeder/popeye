#include "stipulation/check_zigzag_jump.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/dummy_move.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STCheckZigzagJump slice.
 * @param shortcut identifies entry slice of shortcut
 * @return index of allocated slice
 */
slice_index alloc_check_zigzag_jump_slice(slice_index shortcut)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result =  alloc_branch_fork(STCheckZigzagJump,shortcut);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_check_zigzag_jump(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  stip_traverse_structure(slices[si].u.fork.fork,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type check_zigzag_jump_defend(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index succ;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  succ = echecc(nbply,slices[si].starter) ? next : fork;
  result = defend(succ,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type check_zigzag_jump_can_defend(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index succ;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  succ = echecc(nbply,slices[si].starter) ? next : fork;
  result = can_defend(succ,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type check_zigzag_jump_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index succ;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  succ = echecc(nbply,slices[si].starter) ? next : fork;
  result = help(succ,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type check_zigzag_jump_can_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index succ;
  slice_index const next = slices[si].u.fork.next;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  succ = echecc(nbply,slices[si].starter) ? next : fork;
  result = can_help(succ,n);

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
    slice_index const dummy = alloc_dummy_move_slice();
    slice_index const jump = alloc_check_zigzag_jump_slice(dummy);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(ready!=no_slice);
    assert(deadend!=no_slice);
    pipe_append(deadend,jump);
    battle_branch_insert_slices(jump,&landing_proto,1);

    {
      slice_index const landing = branch_find_slice(STCheckZigzagLanding,jump);
      assert(landing!=no_slice);
      pipe_set_successor(dummy,landing);
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
    slice_index const dummy = alloc_dummy_move_slice();
    slice_index const jump = alloc_check_zigzag_jump_slice(dummy);
    slice_index const landing_proto = alloc_pipe(STCheckZigzagLanding);

    assert(jump!=no_slice);
    pipe_append(ready,jump);
    help_branch_insert_slices(jump,&landing_proto,1);

    {
      slice_index const landing = branch_find_slice(STCheckZigzagLanding,jump);
      assert(landing!=no_slice);
      pipe_set_successor(dummy,landing);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
