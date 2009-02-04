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
  who_decides_on_starter result = dont_know_who_decides_on_starter;
  slice_index const next = slices[si].u.branch.next;
  slice_index next_relevant = next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",is_duplex);
  TraceFunctionParam("%u\n",same_side_as_root);
  
  if (slices[next].type==STMoveInverter)
    next_relevant = slices[next].u.move_inverter.next;

  TraceValue("%u\n",next_relevant);

  /* TODO move to branch files */
  switch (slices[si].type)
  {
    case STBranchDirect:
      result = slice_detect_starter(next,is_duplex,same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        switch (slices[next_relevant].type)
        {
          case STLeafDirect:
            slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,slices[si].u.branch.starter);
            break;

          case STLeafSelf:
            slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,slices[si].u.branch.starter);
            break;

          case STLeafHelp:
            slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,advers(slices[si].u.branch.starter));
            break;

          default:
            result = slice_detect_starter(next,is_duplex,same_side_as_root);
            slices[si].u.branch.starter = slice_get_starter(next);
            break;
        }
      }
      else
        slices[si].u.branch.starter = slice_get_starter(next);

      TraceValue("%u\n",slices[si].u.branch.starter);
      break;

    case STBranchHelp:
    {
      boolean const even_length = slices[si].u.branch.length%2==0;
      TraceValue("%u\n",even_length);

      switch (slices[next_relevant].type)
      {
        case STLeafDirect:
        {
          boolean const next_same_side_as_root =
              (even_length
               ? same_side_as_root
               : !same_side_as_root);
          result = slice_detect_starter(next,
                                        is_duplex,
                                        next_same_side_as_root);
          if (slice_get_starter(next)==no_side)
          {
            /* next can't tell - let's tell him */
            slices[si].u.branch.starter = is_duplex ? White : Black;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,advers(slices[si].u.branch.starter));
          }
          else
            slices[si].u.branch.starter =
                (even_length
                 ? slice_get_starter(next)
                 : advers(slice_get_starter(next)));
          break;
        }

        case STLeafSelf:
        {
          boolean const next_same_side_as_root =
              (even_length
               ? same_side_as_root
               : !same_side_as_root);
          result = slice_detect_starter(next,
                                        is_duplex,
                                        next_same_side_as_root);
          if (slice_get_starter(next)==no_side)
          {
            /* next can't tell - let's tell him */
            slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,slices[si].u.branch.starter);
          }
          else
            slices[si].u.branch.starter =
                (even_length
                 ? slice_get_starter(next)
                 : advers(slice_get_starter(next)));
          break;
        }

        case STLeafHelp:
        {
          boolean const next_same_side_as_root =
              (even_length
               ? same_side_as_root
               : !same_side_as_root);
          result = slice_detect_starter(next,
                                        is_duplex,
                                        next_same_side_as_root);
          if (slice_get_starter(next)==no_side)
          {
            /* next can't tell - let's tell him */
            slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,slices[si].u.branch.starter);
          }
          else
            slices[si].u.branch.starter =
                (even_length
                 ? slice_get_starter(next)
                 : advers(slice_get_starter(next)));
          break;
        }

        default:
          result = slice_detect_starter(next,is_duplex,same_side_as_root);
          slices[si].u.branch.starter = (even_length
                                         ? slice_get_starter(next)
                                         : advers(slice_get_starter(next)));
          break;
      }

      TraceValue("%u\n",slices[si].u.branch.starter);
      break;
    }

    case STBranchSeries:
      result = slice_detect_starter(next,is_duplex,!same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        switch (slices[next_relevant].type)
        {
          case STLeafDirect:
            if (next==next_relevant)
              /* e.g. ser-h# */
              slices[si].u.branch.starter = is_duplex ? White : Black;
            else
              /* e.g. ser-# */
              slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,advers(slices[si].u.branch.starter));
            break;

          case STLeafSelf:
            slices[si].u.branch.starter = is_duplex ? Black : White;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,slices[si].u.branch.starter);
            break;

          case STLeafHelp:
            slices[si].u.branch.starter = is_duplex ? White : Black;
            TraceValue("%u\n",slices[si].u.branch.starter);
            slice_impose_starter(next,slices[si].u.branch.starter);
            break;

          default:
            result = slice_detect_starter(next,is_duplex,same_side_as_root);
            slices[si].u.branch.starter = slice_get_starter(next);
            break;
        }
      }
      else
        slices[si].u.branch.starter = advers(slice_get_starter(next));

      TraceValue("%u\n",slices[si].u.branch.starter);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
