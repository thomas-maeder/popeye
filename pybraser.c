#include "pybraser.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "pyhash.h"
#include "pyslice.h"
#include "pyoutput.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Allocate a STBranchSeries slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_ser_slice(stip_length_type length,
                                   stip_length_type min_length,
                                   slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u\n",next);

  slices[result].type = STBranchSeries; 
  slices[result].u.branch.starter = no_side; 
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;
  slices[result].u.branch.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_ser_must_starter_resign(slice_index si)
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
void branch_ser_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.branch.next);
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_ser_has_non_starter_solved(slice_index si)
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
boolean branch_ser_has_starter_apriori_lost(slice_index si)
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
boolean branch_ser_has_starter_won(slice_index si)
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
boolean branch_ser_has_starter_reached_goal(slice_index si)
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
boolean branch_ser_is_goal_reached(Side just_moved, slice_index si)
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

/* Solve a composite slice with series play
 * @param n number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean branch_ser_solve_in_n_recursive(slice_index si,
                                               stip_length_type n)
{
  boolean result = false;
  Side const series_side = slices[si].u.branch.starter;
  Side other_side = advers(series_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_series)
    result = slice_solve(slices[si].u.branch.next);
  else if (echecc(nbply,other_side))
    TraceText("echecc(nbply,other_side)\n");
  else
  {
    if (!slice_must_starter_resign(si))
    {
      active_slice[nbply+1] = si;
      genmove(series_side);

      if (series_side==White)
        WhMovesLeft--;
      else
        BlMovesLeft--;

      while (encore())
      {
        if (!(jouecoup(nbply,first_play) && TraceCurrentMove(nbply)))
          TraceText("!jouecoup(nbply,first_play)\n");
        else if (echecc(nbply,series_side))
          TraceText("echecc(nbply,series_side)\n");
        else if (isIntelligentModeActive && !isGoalReachable())
          TraceText("isIntelligentModeActive && !isGoalReachable()\n");
        else
        {
          (*encode)();
          if (inhash(si,SerNoSucc,n-slack_length_series))
            TraceText("in hash\n");
          else if (branch_ser_solve_in_n_recursive(si,n-1))
            result = true;
          else
            addtohash(si,SerNoSucc,n-slack_length_series);
        }

        repcoup();

        if ((OptFlag[maxsols] && solutions>=maxsolutions)
            || maxtime_status==MAXTIME_TIMEOUT)
          break;
      }

      if (series_side==White)
        WhMovesLeft++;
      else
        BlMovesLeft++;

      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with series play at root level
 * @param n number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean branch_ser_root_solve_in_n_recursive(slice_index si,
                                                    stip_length_type n)
{
  Side const series_side = slices[si].u.branch.starter;
  Side const other_side = advers(series_side);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_series)
    result = slice_root_solve(slices[si].u.branch.next);
  else if (echecc(nbply,other_side))
    TraceText("echecc(nbply,other_side)\n");
  else
  {
    if (!slice_must_starter_resign(si))
    {
      active_slice[nbply+1] = si;
      genmove(series_side);

      if (series_side==White)
        WhMovesLeft--;
      else
        BlMovesLeft--;

      while (encore())
      {
        if (!(jouecoup(nbply,first_play) && TraceCurrentMove(nbply)))
          TraceText("!jouecoup(nbply,first_play)\n");
        else if (echecc(nbply,series_side))
          TraceText("echecc(nbply,series_side)\n");
        else if (OptFlag[restart] && MoveNbr<RestartNbr)
          TraceText("OptFlag[restart] && MoveNbr<RestartNbr\n");
        else if (isIntelligentModeActive && !isGoalReachable())
          TraceText("isIntelligentModeActive && !isGoalReachable()\n");
        else
        {
          (*encode)();
          if (inhash(si,SerNoSucc,n-slack_length_series))
            TraceText("in hash\n");
          else if (branch_ser_solve_in_n_recursive(si,n-1))
            result = true;
          else
            addtohash(si,SerNoSucc,n-slack_length_series);
        }

        if (OptFlag[movenbr])
          IncrementMoveNbr();

        repcoup();

        if ((OptFlag[maxsols] && solutions>=maxsolutions)
            || maxtime_status==MAXTIME_TIMEOUT)
          break;
      }

      if (series_side==White)
        WhMovesLeft++;
      else
        BlMovesLeft++;

      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve full-length solutions in n in series play at root level
 * @param si slice index
 * @param n number of half moves
 * @return true iff >=1 solution was found
 */
static boolean branch_ser_root_solve_full_in_n(slice_index si,
                                               stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=1);

  if (isIntelligentModeActive)
    result = Intelligent(si,n);
  else
    result = branch_ser_root_solve_in_n_recursive(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve short solutions in n in series play at root level
 * @param si slice index
 * @param n number of half moves
 * @return true iff >=1 short solution was found
 */
static boolean branch_ser_root_solve_short_in_n(slice_index si,
                                                stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=slack_length_series);

  if (isIntelligentModeActive)
    result = Intelligent(si,n);
  else
  {
    /* we only display move numbers when looking for full length
     * solutions
     */
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    result = branch_ser_solve_in_n_recursive(si,n);
    OptFlag[movenbr] = save_movenbr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_ser_root_make_setplay_slice(slice_index si)
{
  slice_index const next = slices[si].u.branch.next;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (slices[next].type==STMoveInverter)
  {
    slice_index const next_next = slices[next].u.move_inverter.next;
    result = slice_root_make_setplay_slice(next_next);
  }
  else
    result = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with series play at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_ser_root_solve(slice_index si)
{
  Side const starter = slices[si].u.branch.starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",starter);
  
  move_generation_mode = move_generation_not_optimized;

  init_output(si);

  if (echecc(nbply,advers(starter)))
    ErrorMsg(KingCapture);
  else
  {
    stip_length_type const full_length = slices[si].u.branch.length;

    stip_length_type len;
    for (len = slices[si].u.branch.min_length;
         len<full_length && !(OptFlag[stoponshort] && FlagShortSolsReached);
         len++)
      if (branch_ser_root_solve_short_in_n(si,len))
      {
        FlagShortSolsReached = true;
        result = true;
      }

    if (!(FlagShortSolsReached && OptFlag[stoponshort]))
      result = branch_ser_root_solve_full_in_n(si,full_length);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve the root composite slice with series play
 * @param si slice index
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
/* TODO when is the following implementation better? */
/*boolean branch_ser_root_solve(slice_index si,
  stip_length_type n)
  {
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",n);

  if (n==slices[si].u.branch.min_length)
  result = branch_ser_root_solve_in_n_recursive(si,n);
  else if (OptFlag[restart])
  result = branch_ser_root_maximal_solve(n,si);
  else
  result = branch_ser_root_solve(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
  }*/

/* Solve a composite slice with series play
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean branch_ser_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.branch.length;
  stip_length_type i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].u.branch.length);

  for (i = slices[si].u.branch.min_length; i<n; i++)
    if (branch_ser_solve_in_n_recursive(si,i))
    {
      TraceText("solution found\n");
      result = true;
      if (OptFlag[stoponshort])
      {
        FlagShortSolsReached = true;
        break;
      }
    }

  if (!FlagShortSolsReached
      && branch_ser_solve_in_n_recursive(si,n))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void branch_ser_root_solve_in_n(slice_index si, stip_length_type n)
{
  branch_ser_root_solve_in_n_recursive(si,n);
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_ser_detect_starter(slice_index si,
                                                 boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;
  slice_index const next = slices[si].u.branch.next;
  slice_index next_relevant = next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",same_side_as_root);
  
  if (slices[next].type==STMoveInverter)
    next_relevant = slices[next].u.move_inverter.next;

  TraceValue("%u\n",next_relevant);

  result = slice_detect_starter(next,!same_side_as_root);
  if (slice_get_starter(next)==no_side)
  {
    /* next can't tell - let's tell him */
    switch (slices[next_relevant].type)
    {
      case STLeafDirect:
        if (next==next_relevant)
          /* e.g. ser-h# */
          slices[si].u.branch.starter = Black;
        else
          /* e.g. ser-# */
          slices[si].u.branch.starter = White;
        TraceValue("%u\n",slices[si].u.branch.starter);
        slice_impose_starter(next,advers(slices[si].u.branch.starter));
        break;

      case STLeafSelf:
        slices[si].u.branch.starter = White;
        TraceValue("%u\n",slices[si].u.branch.starter);
        slice_impose_starter(next,slices[si].u.branch.starter);
        break;

      case STLeafHelp:
        slices[si].u.branch.starter = Black;
        TraceValue("%u\n",slices[si].u.branch.starter);
        slice_impose_starter(next,slices[si].u.branch.starter);
        break;

      default:
        result = slice_detect_starter(next,same_side_as_root);
        slices[si].u.branch.starter = slice_get_starter(next);
        break;
    }
  }
  else
    slices[si].u.branch.starter = advers(slice_get_starter(next));

  TraceValue("%u\n",slices[si].u.branch.starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_ser_impose_starter(slice_index si, Side s)
{
  slices[si].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,advers(s));
}
