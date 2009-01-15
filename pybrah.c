#include "pybrah.h"
#include "pybranch.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

static boolean branch_h_solve_in_n_recursive(slice_index si,
                                             stip_length_type n,
                                             Side side_at_move);

/* Determine and write the solution(s) in a help stipulation; don't
 * consult nor fill the hash table regarding solutions of length n
 * (either we shouldn't right now, or it has already been/will be done
 * elsewhere).
 *
 * This is a recursive function.
 * Recursion works with decreasing parameter n; recursion stops at
 * n==2 (e.g. h#1).
 *
 * @param side_at_move side at move
 * @param n number of half moves until end state has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @param si slice index of slice being solved
 */
static void branch_h_root_solve_in_n_recursive_nohash(slice_index si,
                                                      stip_length_type n,
                                                      Side side_at_move)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    slice_root_solve(slices[si].u.branch.next);
  else
  {
    Side next_side = advers(side_at_move);

    genmove(side_at_move);
    active_slice[nbply] = si;
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(nbply,side_at_move)
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && !slice_must_starter_resign(slices[si].u.branch.next))
        branch_h_solve_in_n_recursive(si,n-1,next_side);

      if (OptFlag[movenbr])
        IncrementMoveNbr();

      repcoup();

      /* Stop solving if a given number of solutions was encountered */
      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || maxtime_status==MAXTIME_TIMEOUT)
        break;
    }
    
    if (side_at_move==Black)
      BlMovesLeft++;
    else
      WhMovesLeft++;

    finply();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the solution(s) in a help stipulation; don't
 * consult nor fill the hash table regarding solutions of length n. 
 *
 * This is a recursive function.
 * Recursion works with decreasing parameter n; recursion stops at
 * n==2 (e.g. h#1).
 *
 * @param side_at_move side at move
 * @param n number of half moves until end state has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @param si slice index of slice being solved
 * @return true iff >= 1 solution has been found
 */
static boolean branch_h_solve_in_n_recursive_nohash(slice_index si,
                                                    stip_length_type n,
                                                    Side side_at_move)

{
  boolean found_solution = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    found_solution = slice_solve(slices[si].u.branch.next);
  else
  {
    Side next_side = advers(side_at_move);

    genmove(side_at_move);
    active_slice[nbply] = si;
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(nbply,side_at_move)
          && !slice_must_starter_resign(slices[si].u.branch.next)
          && branch_h_solve_in_n_recursive(si,n-1,next_side))
        found_solution = true;

      repcoup();

      /* Stop solving if a given number of solutions was encountered */
      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || maxtime_status==MAXTIME_TIMEOUT)
        break;
    }
    
    if (side_at_move==Black)
      BlMovesLeft++;
    else
      WhMovesLeft++;

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}


/* Determine and write the solution(s) in a help stipulation.
 *
 * This is a recursive function.
 * Recursion works with decreasing parameter n; recursion stops at
 * n==2 (e.g. h#1).
 *
 * @param side_at_move side at move
 * @param n number of half moves until end state has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @param si slice index of slice being solved
 * @return true iff >= 1 solution has been found
 */
static boolean branch_h_solve_in_n_recursive(slice_index si,
                                             stip_length_type n,
                                             Side side_at_move)
{
  boolean found_solution = false;
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;
  HashBuffer hb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  (*encode)(&hb);
  if (!inhash(si,hash_no_succ,n/2,&hb))
  {
    if (branch_h_solve_in_n_recursive_nohash(si,n,side_at_move))
      found_solution = true;
    else
      addtohash(si,hash_no_succ,n/2,&hb);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Determine and write the solution(s) in a help stipulation.
 * @param si identifies slice being solved
 * @return true iff >= 1 solution was found
 */
static boolean branch_h_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  Side const starter = branch_h_starter_in_n(si,n);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",starter);

  if (n==slices[si].u.branch.length)
    result = branch_h_solve_in_n_recursive_nohash(si,n,starter);
  else
    result = branch_h_solve_in_n_recursive(si,n,starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve full-length solutions in exactly n in help play at root level
 * @param si slice index
 * @param n number of half moves
 */
static void branch_h_root_solve_full_in_n(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",n);

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    Intelligent(n);
  else
    branch_h_root_solve_in_n(si,n);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve short solutions in exactly n in help play at root level.
 * @param si slice index
 * @param n number of half moves
 * @return true iff >=1 short solution was found
 */
static boolean branch_h_root_solve_short_in_n(slice_index si,
                                              stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    result = Intelligent(n);
  else
  {
    /* we only display move numbers when looking for full length
     * solutions (incl. full length set play)
     */
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    result = branch_h_solve_in_n(si,n);
    OptFlag[movenbr] = save_movenbr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write set play in help play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_h_root_solve_setplay(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_setplay_level();

  if (echecc(nbply,slices[si].u.branch.starter))
    ErrorMsg(KingCapture);
  else
  {
    stip_length_type const full_length = slices[si].u.branch.length-1;

    if (full_length%2==1)
      result = slice_root_solve_setplay(slices[si].u.branch.next);

    if (full_length>1)
    {
      stip_length_type len;
      for (len = slices[si].u.branch.min_length+1;
           !result && len<full_length;
           len += 2)
        result = branch_h_root_solve_short_in_n(si,len);

      if (!result)
        branch_h_root_solve_full_in_n(si,full_length);
    }
  }

  output_end_setplay_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with help play at root level
 * @param si slice index
 */
void branch_h_root_solve(slice_index si)
{
  stip_length_type const full_length = slices[si].u.branch.length;
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  init_output_mode(output_mode_line);

  move_generation_mode = move_generation_not_optimized;

  if (OptFlag[solapparent] && !OptFlag[restart])
  {
    branch_h_root_solve_setplay(si);
    write_end_of_solution_phase();
  }

  solutions = 0;    /* reset after set play */
  FlagShortSolsReached = false;

  if (echecc(nbply,advers(slices[si].u.branch.starter)))
    ErrorMsg(KingCapture);
  else if (full_length==slack_length_help-1)
    slice_root_solve_setplay(next);
  else
  {
    stip_length_type len = slices[si].u.branch.min_length;

    if (len==slack_length_help-1)
    {
      FlagShortSolsReached = slice_root_solve_complete_set(next);
      len +=2;
    }

    while (len<full_length
           && !(OptFlag[stoponshort] && FlagShortSolsReached))
    {
      if (branch_h_root_solve_short_in_n(si,len))
        FlagShortSolsReached = true;
      len += 2;
    }

    if (!(FlagShortSolsReached && OptFlag[stoponshort]))
      branch_h_root_solve_full_in_n(si,full_length);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}


/* Determine the starting side in a help branch in n
 * @param si slice index
 * @param n number of half-moves
 */
Side branch_h_starter_in_n(slice_index si, stip_length_type n)
{
  Side const branch_starter = slices[si].u.branch.starter;
  return (slices[si].u.branch.length%2==n%2
          ? branch_starter
          : advers(branch_starter));
}

/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void branch_h_root_solve_in_n(slice_index si, stip_length_type n)
{
  Side const starter = branch_h_starter_in_n(si,n);
  branch_h_root_solve_in_n_recursive_nohash(si,n,starter);
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_h_impose_starter(slice_index si, Side s)
{
  /* help play in N.5 -> change starter */
  Side next_starter = (slices[si].u.branch.length%2==1 ? advers(s) : s);
  slices[si].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,next_starter);
}
