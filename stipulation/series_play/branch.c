#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "stipulation/goals/goals.h"
#include "stipulation/series_play/play.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/series_play/ready_for_series_move.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/series_play/find_shortest.h"
#include "stipulation/series_play/fork.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/dummy_move.h"
#include "stipulation/series_play/shortcut.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types dealing with series moves appear
 * STSeriesFork is not mentioned because it has a variable rank.
 */
static slice_index const series_slice_rank_order[] =
{
  STSeriesAdapter,
  STStopOnShortSolutionsInitialiser,
  STSeriesRoot,
  STSeriesFindShortest,
  STIntelligentSeriesFilter,
  STSeriesShortcut,

  STReadyForSeriesMove,
  STSeriesHashed,
  STSeriesFork,
  STSeriesHashed,
  STParryFork,
  STDoubleMateFilter,
  STCounterMateFilter,
  STSeriesDummyMove,
  STSeriesMove,
  STSeriesMoveToGoal,
  STMaxTimeSeriesFilter,
  STMaxSolutionsSeriesFilter,
  STStopOnShortSolutionsFilter,
  STKeepMatingFilter,
  STGoalReachableGuardSeriesFilter,
  STPiecesParalysingMateFilter,
  STRestartGuard,
  STEndOfRoot,
  STSelfCheckGuard,
  STReflexSeriesFilter,

  STSeriesFork,
  STSeriesDummyMove,
  STSelfCheckGuard
};

enum
{
  nr_series_slice_rank_order_elmts = (sizeof series_slice_rank_order
                                      / sizeof series_slice_rank_order[0]),
  no_series_slice_type = INT_MAX
};

/* Determine the rank of a series slice type
 * @param type series slice type
 * @param base base rank value
 * @return rank of type; nr_series_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_series_slice_rank(SliceType type, unsigned int base)
{
  unsigned int result = no_series_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_series_slice_rank_order_elmts; ++i)
    if (series_slice_rank_order[(i+base)%nr_series_slice_rank_order_elmts]==type)
    {
      result = i+base;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void series_branch_insert_slices_recursive(slice_index si_start,
                                                  slice_index const prototypes[],
                                                  unsigned int nr_prototypes,
                                                  unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si_start);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  {
    slice_index si = si_start;
    SliceType const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_series_slice_rank(prototype_type,base);

    do
    {
      slice_index const next = slices[si].u.pipe.next;
      if (slices[next].type==STProxy)
        si = next;
      else if (slices[next].type==STGoalReachedTesting)
      {
        leaf_branch_insert_slices_nested(next,prototypes,nr_prototypes);
        break;
      }
      else if (slices[next].type==STDefenseAdapter)
      {
        battle_branch_insert_slices_nested(next,prototypes,nr_prototypes);
        break;
      }
      else if (slices[next].type==STQuodlibet || slices[next].type==STReciprocal)
      {
        series_branch_insert_slices_recursive(slices[next].u.binary.op1,
                                              prototypes,nr_prototypes,
                                              base);
        series_branch_insert_slices_recursive(slices[next].u.binary.op2,
                                              prototypes,nr_prototypes,
                                              base);
        break;
      }
      else
      {
        unsigned int const rank_next = get_series_slice_rank(slices[next].type,base);
        if (rank_next==no_series_slice_type)
          break;
        else if (rank_next>prototype_rank)
        {
          slice_index const copy = copy_slice(prototypes[0]);
          pipe_append(si,copy);
          if (nr_prototypes>1)
            series_branch_insert_slices_recursive(copy,
                                                  prototypes+1,nr_prototypes-1,
                                                  prototype_rank+1);
          break;
        }
        else
        {
          if (slices[next].type==STSeriesFork)
            series_branch_insert_slices_recursive(slices[next].u.branch_fork.towards_goal,
                                                  prototypes,nr_prototypes,
                                                  base);

          base = rank_next;
          si = next;
        }
      }
    } while (si!=si_start && prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a series branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by series_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void series_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  unsigned int i;
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_series_slice_rank(slices[si].type,0);
  assert(base!=no_series_slice_type);

  series_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a series pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
void shorten_series_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  slices[pipe].u.branch.length -= 2;
  if (slices[pipe].u.branch.min_length>slack_length_series+1)
    slices[pipe].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a the appropriate proxy slices before each
 * STGoalReachedTesting slice
 * @param si identifies slice
 * @param st address of structure representing the traversal
 */
static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(slices[si].prev,
              alloc_series_move_slice(slack_length_series+1,
                                      slack_length_series+1));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch leading to a goal to be a series goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_series_goal_branch(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTesting,
                                           &instrument_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


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
    slice_index const finder = alloc_series_find_shortest_slice(length,
                                                                min_length);
    slice_index const ready = alloc_ready_for_series_move_slice(length,
                                                                min_length);
    slice_index const move = alloc_series_move_slice(length,min_length);
    slice_index const ready2 = alloc_ready_for_series_move_slice(length-1,
                                                                 min_length-1);
    slice_index const dummy = alloc_series_dummy_move_slice();

    result = adapter;
    pipe_link(adapter,finder);
    pipe_set_successor(finder,ready);

    pipe_link(ready,move);
    pipe_link(move,ready2);
    pipe_link(ready2,dummy);
    pipe_link(dummy,ready);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a fork to the branch leading to the goal
 * @param si identifies the entry slice of a series branch
 * @param to_goal identifies the entry slice of the branch leading to
 *                the goal
 */
void series_branch_set_goal_slice(slice_index si, slice_index to_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STSeriesAdapter);

  {
    slice_index const prototype = alloc_series_fork_slice(to_goal);
    series_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a series branch
 * @param next identifies the entry slice of the next branch
 */
void series_branch_set_next_slice(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STSeriesAdapter);

  {
    slice_index const ready1 = branch_find_slice(STReadyForSeriesMove,si);
    slice_index const ready2 = branch_find_slice(STReadyForSeriesMove,ready1);
    assert(ready1!=no_slice);
    assert(ready2!=no_slice);
    pipe_append(slices[ready2].prev,alloc_series_fork_slice(next));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
