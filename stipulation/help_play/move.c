#include "stipulation/help_play/move.h"
#include "pydata.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/shortcut.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_move_slice(stip_length_type length,
                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void help_move_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*root_slice==no_slice);

  pipe_append(si,alloc_pipe(STEndOfRoot));
  *root_slice = copy_slice(si);
  help_branch_shorten_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void help_move_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    stip_traverse_structure_children(si,st);
    slices[si].starter = (slices[next].starter==no_side
                          ? no_side
                          : advers(slices[next].starter));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try solving with all generated moves
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type foreach_move_solve(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help(next,n-1)==n-1)
      result = n;

    repcoup();
  }

  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_move_help(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  move_generation_mode= move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side_at_move);
  result = foreach_move_solve(si,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Iterate moves until a solution has been found
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
static stip_length_type find_solution(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n+2;

  while (encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && can_help(next,n-1)==n-1)
    {
      result = n;
      repcoup();
      break;
    }
    else
      repcoup();

  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_move_can_help(slice_index si, stip_length_type n)
{
  Side const side_at_move = slices[si].starter;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  move_generation_mode= move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side_at_move);
  result = find_solution(si,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void help_move_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *setplay_slice = stip_make_setplay(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void help_move_make_setplay_slice(slice_index si, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type min_length = slices[si].u.branch.min_length;

  if (min_length<slack_length_help)
    min_length += 2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*setplay_slice==no_slice);
  pipe_append(si,alloc_pipe(STEndOfRoot));
  *setplay_slice = alloc_help_move_slice(length,min_length);
  pipe_set_successor(*setplay_slice,slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
