#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/help_play/move.h"
#include "trace.h"

#include <assert.h>

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
 * @param si identifies STGoalReachedTester slice
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

static structure_traversers_visitors help_goal_instrumenters[] =
{
  { STGoalReachedTester,          &instrument_tester },
  { STGoalMateReachedTester,      &instrument_tester },
  { STGoalStalemateReachedTester, &instrument_tester },
  { STGoalTargetReachedTester,    &instrument_tester }
};

enum
{
  nr_help_goal_instrumenters = (sizeof help_goal_instrumenters
                                / sizeof help_goal_instrumenters[0])
};

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
  stip_structure_traversal_override(&st,
                                    help_goal_instrumenters,
                                    nr_help_goal_instrumenters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
