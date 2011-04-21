#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/help_play/find_shortest.h"
#include "stipulation/help_play/end_of_branch.h"
#include "stipulation/help_play/fork.h"
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
  STHelpRoot,
  STHelpFindShortest,
  STIntelligentHelpFilter,
  STForkOnRemaining,

  STEndOfAdapter,

  STReadyForHelpMove,
  STHelpHashed,
  STHelpFork,
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
  STGoalReachedTesting,
  STSelfCheckGuard,
  STReflexHelpFilter,
  STEndOfHelpBranch
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
          if (slices[next].type==STGoalReachedTesting)
          {
            leaf_branch_insert_slices_nested(next,prototypes,nr_prototypes);
            break;
          }
          else if (slices[next].type==STHelpFork
                   || slices[next].type==STEndOfHelpBranch)
            help_branch_insert_slices_recursive(slices[next].u.fork.fork,
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

/* Shorten a sequence of slices
 * @param begin start of sequence (member of the sequence)
 * @param end end of sequence (first non-member of the sequence)
 */
static void shorten_slices(slice_index begin, slice_index end)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",begin);
  TraceFunctionParam("%u",end);
  TraceFunctionParamListEnd();

  while (begin!=end)
  {
    if (slice_has_structure(begin,slice_structure_branch))
      help_branch_shorten_slice(begin);
    begin = slices[begin].u.pipe.next;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten the introductory slices of a help branch (i.e. those that don't
 * belong to the loop) by 1 half move
 * @param si identifies the entry slice
 * @return index of the last introductory slice
 */
static slice_index shorten_intro_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (1)
  {
    if (slice_has_structure(si,slice_structure_branch))
    {
      --slices[si].u.branch.length;
      if (slices[si].u.branch.min_length>slack_length_help+1)
        --slices[si].u.branch.min_length;
      else
        ++slices[si].u.branch.min_length;
    }

    if (slices[slices[si].u.pipe.next].type==STEndOfAdapter)
      break;
    else
      si = slices[si].u.pipe.next;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",si);
  TraceFunctionResultEnd();
  return si;
}

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 */
void help_branch_shorten(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STHelpAdapter);

  {
    slice_index const last_introductory = shorten_intro_slices(si);
    slice_index const loop_entry = slices[last_introductory].u.pipe.next;
    slice_index const loop_entry_next = slices[loop_entry].u.pipe.next;
    slice_index const new_entry_pos = branch_find_slice(STReadyForHelpMove,
                                                        loop_entry_next);
    pipe_link(slices[loop_entry].prev,loop_entry_next);
    shorten_slices(loop_entry_next,new_entry_pos);
    pipe_append(slices[new_entry_pos].prev,loop_entry);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a help branch with an even number of half moves
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of initial slice of allocated help branch
 */
static slice_index alloc_help_branch_odd(stip_length_type length,
                                         stip_length_type min_length)
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
    slice_index const end = alloc_pipe(STEndOfAdapter);
    slice_index const ready1 = alloc_branch(STReadyForHelpMove,
                                            length,min_length);
    slice_index const generator1 = alloc_help_move_generator_slice();
    slice_index const move1 = alloc_help_move_slice();
    slice_index const ready2 = alloc_branch(STReadyForHelpMove,
                                            length-1,min_length-1);
    slice_index const generator2 = alloc_help_move_generator_slice();
    slice_index const move2 = alloc_help_move_slice();

    result = adapter;
    pipe_link(adapter,finder);
    pipe_set_successor(finder,end);

    pipe_link(end,ready1);
    pipe_link(ready1,generator1);
    pipe_link(generator1,move1);
    pipe_link(move1,ready2);
    pipe_link(ready2,generator2);
    pipe_link(generator2,move2);
    pipe_link(move2,end);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the position of a STHelpFork slice to be inserted
 * @param si identifies the entry slice of a help branch
 * @param n indicates at what n slice next should kick in
 * @return identifier of slice behind which to insert STHelpFork slice
 */
static slice_index find_fork_pos(slice_index si, stip_length_type n)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STHelpAdapter);

  do
  {
    si = branch_find_slice(STReadyForHelpMove,si);
    assert(si!=no_slice);
  } while ((n-slices[si].u.branch.length)%2==1);

  result = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param n indicates at what n slice next should kick in
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_next_slice(slice_index si,
                                stip_length_type n,
                                slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  {
    slice_index const pos = find_fork_pos(si,n);
    pipe_append(slices[pos].prev,alloc_end_of_help_branch_slice(next));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the goal branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_goal_slice(slice_index si, slice_index to_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParamListEnd();

  {
    slice_index const pos = find_fork_pos(si,slack_length_help+1);
    pipe_append(pos,alloc_help_fork_slice(to_goal));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  if ((length-slack_length_help)%2==1)
    result = alloc_help_branch_odd(length,min_length);
  else
  {
    /* this indirect approach avoids some code duplication */
    result = alloc_help_branch_odd(length+1,min_length+1);
    help_branch_shorten(result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

  pipe_append(slices[si].prev,alloc_help_move_generator_slice());
  pipe_append(slices[si].prev,alloc_help_move_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch leading to a goal to be a help goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_help_goal_branch(slice_index si)
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
