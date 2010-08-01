#include "stipulation/battle_play/attack_end.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STAttackEnd slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_end_slice(stip_length_type length,
                                   stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STAttackEnd,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the moves beyond a attack end slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_attack_end(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  if (st->remaining>slack_length_battle)
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void attack_end_reduce_to_postkey_play(slice_index si,
                                       stip_structure_traversal *st)
{
  slice_index const *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  if (*postkey_slice!=no_slice)
    dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
attack_end_has_solution_in_n(slice_index si,
                             stip_length_type n,
                             stip_length_type n_min,
                             stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n==slack_length_battle)
    result = slack_length_battle+2;
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const length = slices[si].u.branch.length;
    slice_index const min_length = slices[si].u.branch.min_length;

    if (n_max_unsolvable<slack_length_battle)
      n_max_unsolvable = slack_length_battle;

    if (n_max_unsolvable+length-min_length<n-1)
      n_max_unsolvable = n-(length-min_length)-1;

    if (n_min<=n_max_unsolvable)
      n_min = n_max_unsolvable+1;

    result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_end_solve_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n==slack_length_battle)
    result = slack_length_battle+2;
  else
  {
    slice_index const next = slices[si].u.pipe.next;
    slice_index const length = slices[si].u.branch.length;
    slice_index const min_length = slices[si].u.branch.min_length;

    if (n_max_unsolvable<slack_length_battle)
      n_max_unsolvable = slack_length_battle;

    if (n_max_unsolvable+length-min_length<n-1)
      n_max_unsolvable = n-(length-min_length)-1;

    result = attack_solve_in_n(next,n,n_max_unsolvable);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void attack_end_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    stip_traverse_structure_pipe(si,st);
    slices[si].starter = advers(slices[slices[si].u.pipe.next].starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
