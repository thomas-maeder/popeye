#include "pybranch.h"
#include "pyslice.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_must_starter_resign(slices[si].u.branch.next);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.branch.next);
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean branch_root_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_root_solve_complete_set(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_has_non_starter_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_non_starter_solved(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean branch_has_starter_apriori_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(0); /* just to be sure; this can't possibly be right: */
  result = slice_has_starter_apriori_lost(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_has_starter_won(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(0); /* just to be sure; this can't possibly be right: */
  result = slice_has_starter_won(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_has_starter_reached_goal(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_starter_reached_goal(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_is_goal_reached(just_moved,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_detect_starter(slice_index si,
                                             boolean is_duplex,
                                             boolean same_side_as_root)
{
  who_decides_on_starter result;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",is_duplex);
  TraceFunctionParam("%u\n",same_side_as_root);

  switch (slices[next].type)
  {
    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_detect_starter(next,is_duplex,!same_side_as_root);
      if (slice_get_starter(next)==no_side)
        slices[si].u.branch.starter = no_side;
      else
        slices[si].u.branch.starter = advers(slice_get_starter(next));
      break;

    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
    {
      /* TODO? separate implementations per branch type */
      boolean const next_same_side_as_root =
          (slices[si].type!=STBranchHelp || slices[si].u.branch.length%2==0
           ? same_side_as_root
           : !same_side_as_root);
      result = slice_detect_starter(next,is_duplex,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = is_duplex ? Black : White;
        slice_impose_starter(next,slices[si].u.branch.starter);
      }
      else
        slices[si].u.branch.starter = slice_get_starter(next);
      break;
    }

    default:
      result = slice_detect_starter(next,is_duplex,same_side_as_root);
      slices[si].u.branch.starter = slice_get_starter(next);
      break;
  }

  if (si==root_slice && result!=leaf_decides_on_starter)
    regular_starter = slice_get_starter(si);

  if (slices[si].type==STBranchHelp
      && slices[si].u.branch.length%2 == 1
      && slice_get_starter(si)!=no_side)
    slice_impose_starter(si,advers(slice_get_starter(si)));

  if (si==root_slice && result==leaf_decides_on_starter)
    regular_starter = slice_get_starter(si);

  TraceValue("%u\n",slices[si].u.branch.starter);

  if (si==root_slice)
    TraceValue("%u\n",regular_starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
