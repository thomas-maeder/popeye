#include "pybraser.h"
#include "pybranch.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "pyhash.h"
#include "pyslice.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Solve a composite slice with series play
 * @param n number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean branch_ser_solve_in_n_recursive(slice_index si,
                                               stip_length_type n)
{
  boolean solution_found = false;
  Side const series_side = slices[si].u.branch.starter;
  Side other_side = advers(series_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_series)
    solution_found = slice_solve(slices[si].u.branch.next);
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
        if (!(jouecoup(nbply,first_play) && TraceCurrentMove()))
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
            solution_found = true;
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
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
}

/* Solve a composite slice with series play at root level
 * @param n number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static void branch_ser_root_solve_in_n_recursive(slice_index si,
                                                 stip_length_type n)
{
  Side const series_side = slices[si].u.branch.starter;
  Side const other_side = advers(series_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_series)
    slice_root_solve(slices[si].u.branch.next);
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
        if (!(jouecoup(nbply,first_play) && TraceCurrentMove()))
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
          else if (!branch_ser_solve_in_n_recursive(si,n-1))
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
  TraceText("\n");
}

/* Solve full-length solutions in n in series play at root level
 * @param si slice index
 * @param n number of half moves
 */
static void branch_ser_root_solve_full_in_n(slice_index si,
                                            stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=1);

  if (isIntelligentModeActive)
    Intelligent(n);
  else
    branch_ser_root_solve_in_n_recursive(si,n);

  TraceFunctionExit(__func__);
  TraceText("\n");
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
    result = Intelligent(n);
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

/* Solve a composite slice with series play at root level
 * @param si slice index
 */
void branch_ser_root_solve(slice_index si)
{
  Side const starter = slices[si].u.branch.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",starter);
  
  move_generation_mode = move_generation_not_optimized;

  if (OptFlag[solapparent] && !OptFlag[restart])
  {
    if (echecc(nbply,starter))
      ErrorMsg(KingCapture);
    else
    {
      /* TODO next slice(s) (e.g. leaf_root_solve_setplay()) should
       * invoke Intelligent() */
      boolean const save_isIntelligentModeActive = isIntelligentModeActive;
      isIntelligentModeActive = false;
      output_start_move_inverted_level();
      slice_root_solve(slices[si].u.branch.next);
      output_end_move_inverted_level();
      isIntelligentModeActive = save_isIntelligentModeActive;
    }

    write_end_of_solution_phase();
  }

  solutions= 0;    /* reset after set play */
  FlagShortSolsReached= false;

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
        FlagShortSolsReached = true;

    if (!(FlagShortSolsReached && OptFlag[stoponshort]))
      branch_ser_root_solve_full_in_n(si,full_length);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
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

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_ser_impose_starter(slice_index si, Side s)
{
  slices[si].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,advers(s));
}
