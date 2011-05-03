#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/operators/binary.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/help_play/find_shortest.h"
#include "stipulation/help_play/move_generator.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types dealing with help moves appear
 */
static slice_index const help_slice_rank_order[] =
{
  STHelpAdapter,
  STStopOnShortSolutionsInitialiser,
  STHelpFindByIncreasingLength,
  STHelpFindShortest,
  STEndOfBranchForced,
  STDeadEnd,
  STIntelligentHelpFilter,
  STForkOnRemaining,

  STReadyForHelpMove,
  STHelpHashed,
  STDoubleMateFilter,
  STCounterMateFilter,
  STEnPassantFilter,
  STCastlingFilter,
  STPrerequisiteOptimiser,
  STForkOnRemaining,
  STHelpMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STHelpMove,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STStopOnShortSolutionsFilter,
  STRestartGuard,
  STKeepMatingFilter,
  STGoalReachableGuardFilter,
  STEndOfRoot,
  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STDeadEndGoal,
  STSelfCheckGuard,
  STReflexAttackerFilter,
  STEndOfBranch
};

enum
{
  nr_help_slice_rank_order_elmts = (sizeof help_slice_rank_order
                                    / sizeof help_slice_rank_order[0]),
  no_help_slice_type = INT_MAX
};

/* Determine the rank of a help slice type
 * @param type help slice type
 * @param base base rank value
 * @return rank of type; nr_help_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_slice_rank(SliceType type, unsigned int base)
{
  unsigned int result = no_help_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_help_slice_rank_order_elmts; ++i)
    if (help_slice_rank_order[(i+base)%nr_help_slice_rank_order_elmts]==type)
    {
      result = i+base;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void help_branch_insert_slices_recursive(slice_index si_start,
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
    unsigned int prototype_rank = get_slice_rank(prototype_type,base);

    do
    {
      slice_index const next = slices[si].u.pipe.next;
      if (slices[next].type==STProxy)
        si = next;
      else if (slices[next].type==STQuodlibet || slices[next].type==STReciprocal)
      {
        help_branch_insert_slices_recursive(slices[next].u.binary.op1,
                                            prototypes,nr_prototypes,
                                            base);
        help_branch_insert_slices_recursive(slices[next].u.binary.op2,
                                            prototypes,nr_prototypes,
                                            base);
        break;
      }
      else
      {
        unsigned int const rank_next = get_slice_rank(slices[next].type,base);
        if (rank_next==no_help_slice_type)
          break;
        else if (rank_next>prototype_rank)
        {
          slice_index const copy = copy_slice(prototypes[0]);
          pipe_append(si,copy);
          if (nr_prototypes>1)
            help_branch_insert_slices_recursive(copy,
                                                prototypes+1,nr_prototypes-1,
                                                prototype_rank+1);
          break;
        }
        else
        {
          if (slices[next].type==STEndOfBranch
                   || slices[next].type==STEndOfBranchGoal
                   || slices[next].type==STEndOfBranchGoalImmobile
                   || slices[next].type==STEndOfBranchForced)
            branch_insert_slices_nested(slices[next].u.fork.fork,
                                        prototypes,nr_prototypes);

          base = rank_next;
          si = next;
        }
      }
    } while (si!=si_start && prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a help branch; the elements of
 * prototypes are *not* deallocated by help_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices_nested(slice_index si,
                                      slice_index const prototypes[],
                                      unsigned int nr_prototypes)
{
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_slice_rank(slices[si].type,0);
  assert(base!=no_help_slice_type);

  help_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a help branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by help_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  help_branch_insert_slices_nested(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void help_branch_shorten_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.branch.length -= 2;
  slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten the intro slices
 * @param si identifies the entry slice
 * @param length length to shorten to
 * @param min_length min_length to shorten to
 */
static slice_index shorten_intro(slice_index si,
                                 stip_length_type length,
                                 stip_length_type min_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  while (true)
  {
    if (slice_has_structure(si,slice_structure_branch))
    {
      slices[si].u.branch.length = length;
      slices[si].u.branch.min_length = min_length;
    }

    if (slices[slices[si].u.pipe.next].prev==si)
      si = slices[si].u.pipe.next;
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",si);
  TraceFunctionResultEnd();
  return si;
}

/* Shorten the slices in the loop
 * @param loop_entry identifies the loop entry slice
 * @param length length at entry into the loop
 * @param min_length min_length at entry into the loop
 */
static void shorten_loop(slice_index loop_entry,
                         stip_length_type length,
                         stip_length_type min_length)
{
  slice_index si = loop_entry;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  do
  {
    TraceValue("%u\n",si);
    if (slice_has_structure(si,slice_structure_branch))
    {
      slices[si].u.branch.length = length;
      slices[si].u.branch.min_length = min_length;
    }
    if (slices[si].type==STHelpMove)
    {
      --length;
      --min_length;
    }
    si = slices[si].u.pipe.next;
  }
  while (si!=loop_entry);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 */
void help_branch_shorten(slice_index si)
{
  stip_length_type length = slices[si].u.branch.length;
  stip_length_type min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STHelpAdapter);

  --length;
  if (min_length>slack_length_help)
    --min_length;
  else
    ++min_length;

  {
    slice_index const last_in_intro = shorten_intro(si,length,min_length);
    slice_index const loop_entry = slices[last_in_intro].u.pipe.next;
    slice_index const new_entry_pos = branch_find_slice(STReadyForHelpMove,
                                                        loop_entry);
    shorten_loop(new_entry_pos,length,min_length);
    link_to_branch(last_in_intro,new_entry_pos);
  }

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a slice marking the end of the branch
 * @param si identifies the entry slice of a help branch
 * @param end_proto end of branch prototype slice
 */
static void insert_end_of_branch(slice_index si, slice_index end_proto)
{
  slice_index pos = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",end_proto);
  TraceFunctionParamListEnd();

  do
  {
    pos = branch_find_slice(STReadyForHelpMove,pos);
    assert(pos!=no_slice);
  } while ((slices[pos].u.branch.length-slack_length_help)%2==0);

  help_branch_insert_slices(pos,&end_proto,1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_end(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  insert_end_of_branch(si,alloc_end_of_branch_slice(next));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the goal branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_end_goal(slice_index si, slice_index to_goal)
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
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_end_forced(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  insert_end_of_branch(si,alloc_end_of_branch_forced(next));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate the intro slices of a help branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param entry_point identifies the loop entry slice
 * @return index of initial intro slice
 */
static slice_index alloc_help_branch_intro(stip_length_type length,
                                           stip_length_type min_length,
                                           slice_index entry_point)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_help_adapter_slice(length,min_length);
    slice_index const finder = alloc_help_find_shortest_slice(length,
                                                              min_length);
    slice_index const deadend = alloc_dead_end_slice();

    result = adapter;
    pipe_link(adapter,finder);
    pipe_link(finder,deadend);
    link_to_branch(deadend,entry_point);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const ready1 = alloc_branch(STReadyForHelpMove,
                                            length,min_length);
    slice_index const generator1 = alloc_help_move_generator_slice();
    slice_index const move1 = alloc_help_move_slice();
    slice_index const deadend1 = alloc_dead_end_slice();
    slice_index const ready2 = alloc_branch(STReadyForHelpMove,
                                            length-1,min_length-1);
    slice_index const generator2 = alloc_help_move_generator_slice();
    slice_index const move2 = alloc_help_move_slice();
    slice_index const deadend2 = alloc_dead_end_slice();

    pipe_link(ready1,generator1);
    pipe_link(generator1,move1);
    pipe_link(move1,deadend1);
    pipe_link(deadend1,ready2);
    pipe_link(ready2,generator2);
    pipe_link(generator2,move2);
    pipe_link(move2,deadend2);
    pipe_link(deadend2,ready1);

    result = alloc_help_branch_intro(length,min_length,ready1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static structure_traversers_visitors help_root_slice_inserters[] =
{
  { STHelpFindShortest, &help_find_shortest_make_root },
  { STHelpMove,         &help_move_make_root          },
  { STReciprocal,       &binary_make_root             },
  { STQuodlibet,        &binary_make_root             }
};

enum
{
  nr_help_root_slice_inserters = (sizeof help_root_slice_inserters
                                  / sizeof help_root_slice_inserters[0])
};

/* Create the root slices of a helpbranch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
static slice_index help_branch_make_root_slices(slice_index adapter)
{
  stip_structure_traversal st;
  slice_structural_type i;
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  for (i = 0; i!=nr_slice_structure_types; ++i)
    if (slice_structure_is_subclass(i,slice_structure_pipe))
      stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
  stip_structure_traversal_override(&st,
                                    help_root_slice_inserters,
                                    nr_help_root_slice_inserters);
  stip_traverse_structure(adapter,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
slice_index help_branch_make_root(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  result = help_branch_make_root_slices(adapter);

  {
    slice_index const ready_root = branch_find_slice(STReadyForHelpMove,result);
    slice_index const ready1 = branch_find_slice(STReadyForHelpMove,ready_root);
    slice_index const ready2 = branch_find_slice(STReadyForHelpMove,ready1);
    slice_index si;

    /* shorten the slices of which copies were added to the root intro */
    for (si = ready2; si!=ready1; si = slices[si].u.pipe.next)
      if (slice_has_structure(si,slice_structure_branch))
        help_branch_shorten_slice(si);
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
slice_index help_branch_make_setplay(slice_index adapter)
{
  slice_index result;
  stip_length_type const length = slices[adapter].u.branch.length;
  stip_length_type min_length = slices[adapter].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  if (min_length==slack_length_help)
    min_length += 2;

  if (length>slack_length_help+1)
  {
    slice_index const ready_for_skipped = branch_find_slice(STReadyForHelpMove,
                                                            adapter);
    slice_index const ready_for_set = branch_find_slice(STReadyForHelpMove,
                                                        ready_for_skipped);
    slice_index const set_intro = alloc_help_branch_intro(length-1,
                                                          min_length-1,
                                                          ready_for_set);
    assert(ready_for_skipped!=no_slice);
    assert(ready_for_set!=no_slice);
    result = help_branch_make_root_slices(set_intro);
  }
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}
