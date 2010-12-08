#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types dealing with help moves appear
 * STHelpFork is not mentioned because it has a variable rank.
 */
static slice_index const help_slice_rank_order[] =
{
  STReadyForHelpMove,
  STHelpHashed,
  STEnPassantHelpFilter,
  STCastlingHelpFilter,
  STHelpMove,
  STHelpMoveToGoal,
  STMaxTimeHelpFilter,
  STMaxSolutionsHelpFilter,
  STStopOnShortSolutionsHelpFilter,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STGoalReachableGuardHelpFilter,
  STHelpMovePlayed,
  STSelfCheckGuard,
  STHelpMoveLegalityChecked,
  STHelpMoveDealtWith
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
          pipe_append(si,copy_slice(prototypes[0]));
          if (nr_prototypes>1)
            help_branch_insert_slices_recursive(si,
                                                prototypes+1,nr_prototypes-1,
                                                base);
          break;
        }
        else
          si = next;
      }
    } while (si!=si_start && prototype_type!=slices[si].type);
  }

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
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_slice_rank(slices[si].type,0);
  assert(base!=no_help_slice_type);

  help_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);

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
  while (begin!=end)
  {
    help_branch_shorten_slice(begin);
    begin = slices[begin].u.pipe.next;
  }
}

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 * @return entry slice of shortened branch
 *         no_slice if shortening isn't applicable
 */
slice_index help_branch_shorten(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = branch_find_slice(STHelpMoveLegalityChecked,si);

  assert(result!=no_slice);
  assert(result!=si);

  shorten_slices(si,result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    slice_index const checked1 = alloc_branch(STHelpMoveLegalityChecked,
                                              length,min_length);
    slice_index const dealt1 = alloc_branch(STHelpMoveDealtWith,
                                            length,min_length);
    slice_index const ready1 = alloc_branch(STReadyForHelpMove,
                                            length,min_length);
    slice_index const move1 = alloc_help_move_slice(length,min_length);
    slice_index const played1 = alloc_branch(STHelpMovePlayed,
                                             length-1,min_length-1);
    slice_index const checked2 = alloc_branch(STHelpMoveLegalityChecked,
                                              length-1,min_length-1);
    slice_index const dealt2 = alloc_branch(STHelpMoveDealtWith,
                                            length-1,min_length-1);

    slice_index const ready2 = alloc_branch(STReadyForHelpMove,
                                            length-1,min_length-1);
    slice_index const move2 = alloc_help_move_slice(length-1,min_length-1);
    slice_index const played2 = alloc_branch(STHelpMovePlayed,
                                             length-2,min_length-2);

    pipe_link(ready1,move1);
    pipe_link(move1,played1);
    pipe_link(played1,checked2);
    pipe_link(checked2,dealt2);
    pipe_link(dealt2,ready2);

    pipe_link(ready2,move2);
    pipe_link(move2,played2);
    pipe_link(played2,checked1);
    pipe_link(checked1,dealt1);
    pipe_link(dealt1,ready1);

    result = checked1;
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

  assert(slices[si].type==STHelpMoveLegalityChecked);

  result = branch_find_slice(STReadyForHelpMove,si);
  assert(result!=no_slice);

  if ((slices[result].u.branch.length-slack_length_help)%2
      !=(n-slack_length_help)%2)
  {
    result = branch_find_slice(STReadyForHelpMove,result);
    assert(result!=no_slice);
    assert((slices[result].u.branch.length-slack_length_help)%2
           ==(n-slack_length_help)%2);
  }

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
    stip_length_type length = slices[pos].u.branch.length;
    stip_length_type min_length = slices[pos].u.branch.min_length;
    pipe_append(pos,alloc_help_fork_slice(length,min_length,next));
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
    slice_index const branch = alloc_help_branch_odd(length+1,min_length+1);
    result = help_branch_shorten(branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a the appropriate proxy slices before each
 * STGoal*ReachedTester slice
 * @param si identifies slice
 * @param st address of structure representing the traversal
 */
static void instrument_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const ready = alloc_branch(STReadyForHelpMove,
                                            slack_length_help+1,
                                            slack_length_help+1);
    slice_index const move = alloc_help_move_slice(slack_length_help+1,
                                                   slack_length_help+1);
    slice_index const played = alloc_branch(STHelpMovePlayed,
                                            slack_length_help,
                                            slack_length_help);
    slice_index const checked = alloc_branch(STHelpMoveLegalityChecked,
                                             slack_length_help,
                                             slack_length_help);
    slice_index const dealt = alloc_branch(STHelpMoveDealtWith,
                                           slack_length_help,
                                           slack_length_help);
    pipe_append(slices[si].prev,ready);
    pipe_append(ready,move);
    pipe_append(move,played);

    pipe_append(si,checked);
    pipe_append(checked,dealt);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch leading to a goal to be a help goal branch
 * @param si identifies entry slice of branch
 */
void stip_make_help_goal_branch(slice_index si)
{
  stip_structure_traversal st;
  SliceType type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);

  for (type = first_goal_tester_slice_type;
       type<=last_goal_tester_slice_type;
       ++type)
    stip_structure_traversal_override_single(&st,type,&instrument_tester);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
