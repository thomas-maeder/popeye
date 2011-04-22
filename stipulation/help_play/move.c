#include "stipulation/help_play/move.h"
#include "pydata.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/ready_for_help_move.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/move_generator.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpMove slice.
 * @return index of allocated slice
 */
slice_index alloc_help_move_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STHelpMove);

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
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help(next,n-1)==n-1)
      result = n;

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static structure_traversers_visitors setplay_makers[] =
{
  { STReadyForHelpMove,  &ready_for_help_move_make_setplay_slice },
  { STHelpFork,          &stip_traverse_structure_pipe           },
  { STHelpMoveGenerator, &help_move_generator_make_setplay_slice },
  { STHelpMove,          &help_move_make_setplay_slice           }
};

enum
{
  nr_setplay_makers = (sizeof setplay_makers / sizeof setplay_makers[0])
};

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
static slice_index make_setplay(slice_index si)
{
  slice_index result = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override(&st,setplay_makers,nr_setplay_makers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
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

  *setplay_slice = make_setplay(slices[si].u.pipe.next);

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const end = alloc_pipe(STEndOfRoot);
    slice_index const move = alloc_help_move_slice();
    pipe_append(si,end);
    *setplay_slice = move;
    pipe_set_successor(move,end);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
