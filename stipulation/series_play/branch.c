#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "pypipe.h"
#include "stipulation/constraint.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move.h"
#include "stipulation/dummy_move.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/goals.h"
#include "trace.h"

#include <assert.h>

/* Allocate a series branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_series_adapter_slice(length,min_length);
    slice_index const ready = alloc_branch(STReadyForHelpMove,
                                           length,min_length);
    slice_index const move = alloc_move_slice();
    slice_index const deadend = alloc_dead_end_slice();
    slice_index const ready2 = alloc_pipe(STReadyForSeriesDummyMove);
    slice_index const dummy = alloc_dummy_move_slice();

    result = adapter;

    pipe_link(adapter,ready);
    pipe_link(ready,move);
    pipe_link(move,deadend);
    pipe_link(deadend,ready2);
    pipe_link(ready2,dummy);
    pipe_link(dummy,adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a slice marking the end of the branch
 * @param si identifies the entry slice of a help branch
 * @param end_proto end of branch prototype slice
 */
static void insert_end_of_branch(slice_index si, slice_index end_proto)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",end_proto);
  TraceFunctionParamListEnd();

  {
    slice_index const ready = branch_find_slice(STReadyForHelpMove,si);
    assert(ready!=no_slice);
    help_branch_insert_slices(ready,&end_proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the branch leading to the goal
 * @param si identifies the entry slice of a series branch
 * @param to_goal identifies the entry slice of the branch leading to
 *                the goal
 */
void series_branch_set_end_goal(slice_index si, slice_index to_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  insert_end_of_branch(si,alloc_end_of_branch_goal(to_goal));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a series branch
 * @param next identifies the entry slice of the next branch
 */
void series_branch_set_end(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  insert_end_of_branch(si,alloc_end_of_branch_slice(next));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a series branch
 * @param next identifies the entry slice of the next branch
 */
void series_branch_set_end_forced(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  insert_end_of_branch(si,alloc_end_of_branch_forced(next));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void serve_as_root_hook(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *root_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Create the root slices of a series branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
static slice_index series_branch_make_root_slices(slice_index adapter)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    slice_structural_type i;

    TraceStipulation(adapter);

    stip_structure_traversal_init(&st,&result);
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
      else if (slice_structure_is_subclass(i,slice_structure_binary))
        stip_structure_traversal_override_by_structure(&st,i,&binary_make_root);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(adapter,&st);

    TraceStipulation(result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index series_make_setplay(slice_index adapter)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const next = slices[adapter].u.pipe.next;
    slice_index const prototypes[] =
    {
      alloc_series_adapter_slice(slack_length_help,slack_length_help),
      alloc_pipe(STEndOfRoot)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    help_branch_insert_slices(next,prototypes,nr_prototypes);

    {
      slice_index const set_adapter = branch_find_slice(STSeriesAdapter,next);
      assert(set_adapter!=no_slice);
      if (slices[slices[set_adapter].u.pipe.next].type==STDeadEnd)
        ; /* set play not applicable */
      else
        result = series_branch_make_root_slices(set_adapter);
      pipe_remove(set_adapter);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}


static void constraint_inserter_series_adapter(slice_index si,
                                               stip_structure_traversal *st)
{
  slice_index const * const constraint = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_constraint_slice(*constraint);
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a series branch with STConstraint slices (typically for a ser-r
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void series_branch_insert_constraint(slice_index si, slice_index constraint)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  assert(slices[constraint].type==STProxy);

  stip_structure_traversal_init(&st,&constraint);
  stip_structure_traversal_override_single(&st,
                                           STSeriesAdapter,
                                           &constraint_inserter_series_adapter);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void end_of_branch_forced_inserter_series_move(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const * const forced = st->param;
    slice_index const prototypes[] =
    {
        alloc_end_of_branch_forced(*forced),
        alloc_dead_end_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    help_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a series branch with STEndOfBranchForced slices (typically for a
 * ser-r stipulation)
 * @param si entry slice of branch to be instrumented
 * @param forced identifies branch forced on the defender
 */
void series_branch_insert_end_of_branch_forced(slice_index si,
                                               slice_index forced)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",forced);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(forced);

  assert(slices[forced].type==STProxy);

  stip_structure_traversal_init(&st,&forced);
  stip_structure_traversal_override_single(&st,
                                           STReadyForHelpMove,
                                           &end_of_branch_forced_inserter_series_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
