#include "stipulation/help_play/adapter.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_adapter_slice(stip_length_type length,
                                     stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpAdapter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_help_adapter_slice(slice_index si,
                                            stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED)
  {
    stip_length_type const save_full_length = st->full_length;
    st->full_length = slices[si].u.branch.length-slack_length_help;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    stip_traverse_moves_pipe(si,st);
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = save_full_length;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors setplay_appliers[] =
{
  { STHelpMove,        &help_move_apply_setplay      },
  { STHelpFork,        &stip_structure_visitor_noop  },
  { STForkOnRemaining, &stip_traverse_structure_pipe }
};

enum
{
  nr_setplay_appliers = (sizeof setplay_appliers / sizeof setplay_appliers[0])
};

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param si identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void help_adapter_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,setplay_slice);
    stip_structure_traversal_override(&st,setplay_appliers,nr_setplay_appliers);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_adapter_solve(slice_index si)
{
  has_solution_type result;
  stip_length_type const full_length = slices[si].u.branch.length;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = help(next,full_length);
  result = nr_moves_needed<=full_length ? has_solution : has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_adapter_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = can_help(next,full_length);
  result = nr_moves_needed<=full_length ? has_solution : has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
