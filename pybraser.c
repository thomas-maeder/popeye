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
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STBranchSeries; 
  slices[result].starter = no_side; 
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_ser_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.pipe.next);
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_apriori_lost(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_won(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_reached_goal(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is the move just played playable in a help play solution?
 * @param si slice index
 * @param n number of half moves (including the move just played)
 * @param side_playing_series side that has just played
 * @return true iff the move just played is playable
 */
static boolean move_filter(slice_index si,
                           stip_length_type n,
                           Side side_playing_series)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",side_playing_series);
  TraceFunctionParamListEnd();

  result = ((!isIntelligentModeActive || isGoalReachable())
            && !echecc(nbply,side_playing_series)
            && !slice_must_starter_resign(slices[si].u.pipe.next));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  Side const side_playing_series = slices[si].starter;
  Side other_side = advers(side_playing_series);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n==slack_length_series)
    result = slice_solve(slices[si].u.pipe.next);
  else if (echecc(nbply,other_side))
    TraceText("echecc(nbply,other_side)\n");
  else
  {
    if (inhash(si,SerNoSucc,n-slack_length_series))
      TraceText("in hash\n");
    else
    {
      active_slice[nbply+1] = si;
      genmove(side_playing_series);

      --MovesLeft[side_playing_series];

      while (encore())
      {
        if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
            && move_filter(si,n,side_playing_series))
        {
          if (!slice_must_starter_resign_hashed(slices[si].u.pipe.next)
              && branch_ser_solve_in_n_recursive(si,n-1))
            result = true;
        }

        repcoup();

        if (OptFlag[maxsols] && solutions>=maxsolutions)
        {
          TraceValue("%u",maxsolutions);
          TraceValue("%u",solutions);
          TraceText("aborting\n");
          break;
        }

        if (periods_counter>=nr_periods)
          break;
      }

      ++MovesLeft[side_playing_series];

      finply();

      if (!result)
        addtohash(si,SerNoSucc,n-slack_length_series);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  Side const side_playing_series = slices[si].starter;
  Side const other_side = advers(side_playing_series);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (n==slack_length_series)
    result = slice_root_solve(slices[si].u.pipe.next);
  else if (echecc(nbply,other_side))
    TraceText("echecc(nbply,other_side)\n");
  else
  {
    active_slice[nbply+1] = si;
    genmove(side_playing_series);

    --MovesLeft[side_playing_series];

    while (encore())
    {
      extern unsigned int MoveNbr; /* TODO remove */
      extern unsigned int RestartNbr; /* TODO remove */
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && move_filter(si,n,side_playing_series))
      {
        if (!slice_must_starter_resign_hashed(slices[si].u.pipe.next)
            && branch_ser_solve_in_n_recursive(si,n-1))
          result = true;
      }

      if (OptFlag[movenbr])
        IncrementMoveNbr();

      repcoup();

      if (OptFlag[maxsols] && solutions>=maxsolutions)
      {
        TraceValue("%u",maxsolutions);
        TraceValue("%u",solutions);
        TraceText("aborting\n");
        break;
      }

      if (periods_counter>=nr_periods)
        break;
    }

    ++MovesLeft[side_playing_series];

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=1);

  if (isIntelligentModeActive)
    result = Intelligent(si,n,n);
  else
    result = branch_ser_root_solve_in_n_recursive(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_series);

  if (isIntelligentModeActive)
    result = Intelligent(si,n,slices[si].u.pipe.u.branch.length);
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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_ser_root_make_setplay_slice(slice_index si)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[next].type==STMoveInverter)
  {
    slice_index const next_next = slices[next].u.pipe.next;
    result = slice_root_make_setplay_slice(next_next);
  }
  else
    result = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a composite slice with series play at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_ser_root_solve(slice_index si)
{
  Side const starter = slices[si].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",starter);

  if (echecc(nbply,advers(starter)))
    ErrorMsg(KingCapture);
  else
  {
    stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
    stip_length_type len;
  
    move_generation_mode = move_generation_not_optimized;

    FlagShortSolsReached = false;
    solutions = 0;

    init_output(si);

    for (len = slices[si].u.pipe.u.branch.min_length;
         len<full_length && !(OptFlag[stoponshort] && FlagShortSolsReached);
         len++)
      if (branch_ser_root_solve_short_in_n(si,len))
      {
        FlagShortSolsReached = true;
        result = true;
      }

    if (FlagShortSolsReached && OptFlag[stoponshort])
      TraceText("aborting because of short solutions\n");
    else
      result = branch_ser_root_solve_full_in_n(si,full_length);

    if (OptFlag[maxsols] && solutions>=maxsolutions)
      /* signal maximal number of solutions reached to outer world */
      FlagMaxSolsReached = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a composite slice with series play
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean branch_ser_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  stip_length_type i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[si].u.pipe.u.branch.length);

  for (i = slices[si].u.pipe.u.branch.min_length; i<n; i++)
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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  who_decides_on_starter result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    result = slice_detect_starter(next,!same_side_as_root);
    slices[si].starter = (slices[next].starter==no_side
                          ? no_side
                          : advers(slices[next].starter));
  }
  else
    result = leaf_decides_on_starter;

  TraceValue("%u\n",slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean branch_ser_impose_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,*starter," ");
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;

  *starter = advers(*starter);
  result = slice_traverse_children(si,st);
  *starter = advers(*starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
