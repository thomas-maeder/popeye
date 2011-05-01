#include "stipulation/series_play/branch.h"
#include "pyslice.h"
#include "pyreflxg.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/operators/binary.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/series_play/ready_for_series_move.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/series_play/find_shortest.h"
#include "stipulation/series_play/move.h"
#include "stipulation/series_play/dummy_move.h"
#include "stipulation/series_play/play.h"
#include "stipulation/series_play/move_generator.h"
#include "stipulation/goals/goals.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types dealing with series moves appear
 */
static slice_index const series_slice_rank_order[] =
{
  STSeriesAdapter,
  STStopOnShortSolutionsInitialiser,
  STSeriesFindByIncreasingLength,
  STSeriesFindShortest,
  STDeadEnd,
  STIntelligentSeriesFilter,
  STForkOnRemaining,

  STReadyForSeriesMove,
  STSeriesHashed,
  STDoubleMateFilter,
  STCounterMateFilter,
  STEnPassantFilter,
  STCastlingFilter,
  STPrerequisiteOptimiser,
  STForkOnRemaining,
  STSeriesMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STSeriesMove,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STStopOnShortSolutionsFilter,
  STRestartGuard,
  STKeepMatingFilter,
  STGoalReachableGuardFilter,
  STEndOfRoot,
  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STGoalReachedTesting,
  STDeadEndGoal,
  STSelfCheckGuard,

  STEndOfBranch,

  STParryFork,
  STDefenseAdapter,

  STEndOfBranchForced,
  STDeadEnd,

  STReadyForSeriesDummyMove,
  STSeriesDummyMove,
  STGoalReachableGuardFilter, /* only used in pser stipulations */
  STSelfCheckGuard,

  STReflexAttackerFilter
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
          if (slices[next].type==STGoalReachedTesting)
          {
            leaf_branch_insert_slices_nested(next,prototypes,nr_prototypes);
            break;
          }
          else if (slices[next].type==STDefenseAdapter)
          {
            battle_branch_insert_slices_nested(next,prototypes,nr_prototypes);
            break;
          }
          else if (slices[next].type==STEndOfBranch)
            series_branch_insert_slices_recursive(slices[next].u.fork.fork,
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

/* Insert slices into a series branch; the elements of
 * prototypes are *not* deallocated by series_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void series_branch_insert_slices_nested(slice_index si,
                                        slice_index const prototypes[],
                                        unsigned int nr_prototypes)
{
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_series_slice_rank(slices[si].type,0);
  assert(base!=no_series_slice_type);

  series_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  series_branch_insert_slices_nested(si,prototypes,nr_prototypes);

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
    slice_index const generator = alloc_series_move_generator_slice();
    slice_index const move = alloc_series_move_slice();
    slice_index const deadend = alloc_dead_end_slice();
    slice_index const ready2 = alloc_pipe(STReadyForSeriesDummyMove);
    slice_index const dummy = alloc_series_dummy_move_slice();

    result = adapter;
    pipe_link(adapter,finder);
    pipe_set_successor(finder,ready);

    pipe_link(ready,generator);
    pipe_link(generator,move);
    pipe_link(move,deadend);
    pipe_link(deadend,ready2);
    pipe_link(ready2,dummy);
    pipe_link(dummy,ready);
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

  assert(slices[si].type==STSeriesAdapter);

  {
    slice_index const ready = branch_find_slice(STReadyForSeriesMove,si);
    assert(ready!=no_slice);
    series_branch_insert_slices(ready,&end_proto,1);
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

static structure_traversers_visitors series_root_slice_inserters[] =
{
  { STReflexAttackerFilter, &reflex_attacker_filter_make_root },
  { STSeriesFindShortest,   &series_find_shortest_make_root   },
  { STReadyForSeriesMove,   &ready_for_series_move_make_root  },
  { STSeriesMove,           &series_move_make_root            },
  { STReciprocal,           &binary_make_root                 },
  { STQuodlibet,            &binary_make_root                 }
};

enum
{
  nr_series_root_slice_inserters = (sizeof series_root_slice_inserters
                                    / sizeof series_root_slice_inserters[0])
};

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @return identifier of root slice
 */
slice_index series_branch_make_root(slice_index si)
{
  stip_structure_traversal st;
  slice_structural_type i;
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  for (i = 0; i!=nr_slice_structure_types; ++i)
    if (slice_structure_is_subclass(i,slice_structure_pipe))
      stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
  stip_structure_traversal_override(&st,
                                    series_root_slice_inserters,
                                    nr_series_root_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Find the slice where to start spawning off the set play
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of slice where to start if setplay is applicable
 *         no_slice otherwise
 */
static slice_index find_setplay_spawn_slice(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  result = branch_find_slice(STEndOfBranch,adapter);
  if (result==no_slice)
    result = branch_find_slice(STEndOfBranchForced,adapter);


  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index series_branch_make_setplay(slice_index adapter)
{
  slice_index result;
  slice_index spawn_pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  spawn_pos = find_setplay_spawn_slice(adapter);
  if (spawn_pos==no_slice)
    result = no_slice;
  else
  {
    slice_index const fork = slices[spawn_pos].u.fork.fork;
    slice_index const end_of_branch = alloc_end_of_branch_slice(fork);
    slice_index const dead_end = alloc_dead_end_slice();
    result = alloc_series_adapter_slice(slack_length_series,
                                        slack_length_series);
    pipe_link(result,end_of_branch);
    pipe_link(end_of_branch,dead_end);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}
