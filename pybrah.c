#include "pybrah.h"
#include "pybranch.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyoutput.h"
#include "pyslice.h"
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

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(nbply,side_at_move)
          && !(OptFlag[restart] && MoveNbr<RestartNbr))
      {
        (*encode)();
        if (!slice_must_starter_resign(slices[si].u.branch.next))
          branch_h_solve_in_n_recursive(si,n-1,next_side);
      }

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

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(nbply,side_at_move))
      {
        (*encode)();
        if (!slice_must_starter_resign(slices[si].u.branch.next)
            && branch_h_solve_in_n_recursive(si,n-1,next_side))
          found_solution = true;
      }

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (!inhash(si,hash_no_succ,n/2))
  {
    if (branch_h_solve_in_n_recursive_nohash(si,n,side_at_move))
      found_solution = true;
    else
      addtohash(si,hash_no_succ,n/2);
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
  TraceFunctionParam("%u",si);
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

  output_start_move_inverted_level();

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

  output_end_move_inverted_level();

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

  TraceValue("%u",slices[si].u.branch.min_length);
  TraceValue("%u\n",slices[si].u.branch.length);

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

    if (len==slack_length_help)
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

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_h_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(full_length>=slack_length_help);

  while (len<full_length && !result)
  {
    if (branch_h_solve_in_n(si,len))
    {
      result = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  result = result || branch_h_solve_in_n(si,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice.
 * @param table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
static
void branch_h_solve_continuations_in_n_recursive_nohash(int table,
                                                        slice_index si,
                                                        stip_length_type n,
                                                        Side side_at_move)

{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    slice_solve_continuations(table,slices[si].u.branch.next);
  else
  {
    Side next_side = advers(side_at_move);

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(nbply,side_at_move))
      {
        (*encode)();
        if (!slice_must_starter_resign(slices[si].u.branch.next)
            && branch_h_solve_in_n_recursive(si,n-1,next_side))
          pushtabsol(table);
      }

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

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
static
void branch_h_solve_continuations_in_n_recursive(int table,
                                                 slice_index si,
                                                 stip_length_type n,
                                                 Side side_at_move)
{
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (!inhash(si,hash_no_succ,n/2))
  {
    branch_h_solve_continuations_in_n_recursive_nohash(table,
                                                       si,
                                                       n,
                                                       side_at_move);
    if (tablen(table)==0)
      addtohash(si,hash_no_succ,n/2);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice.
 * @param table where to add first moves
 * @param si identifies slice being solved
 * @param n number of half moves until end state has to be reached
 */
static void branch_h_solve_continuations_in_n(int table,
                                              slice_index si,
                                              stip_length_type n)
{
  Side const starter = branch_h_starter_in_n(si,n);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",starter);

  if (n==slices[si].u.branch.length)
    branch_h_solve_continuations_in_n_recursive_nohash(table,
                                                       si,
                                                       n,
                                                       starter);
  else
    branch_h_solve_continuations_in_n_recursive(table,si,n,starter);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_h_solve_continuations(int table, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(full_length>=slack_length_help);

  while (len<full_length && !solution_found)
  {
    branch_h_solve_continuations_in_n(table,si,len);
    if (tablen(table)>0)
    {
      solution_found = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  if (!solution_found)
    branch_h_solve_continuations_in_n(table,si,full_length);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

static boolean branch_h_has_solution_in_n_recursive(slice_index si,
                                                    stip_length_type n,
                                                    Side side_at_move);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
static
boolean branch_h_has_solution_in_n_recursive_nohash(slice_index si,
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
    found_solution = slice_has_solution(slices[si].u.branch.next);
  else
  {
    Side next_side = advers(side_at_move);

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(nbply,side_at_move))
      {
        (*encode)();
        if (!slice_must_starter_resign(slices[si].u.branch.next)
            && branch_h_has_solution_in_n_recursive(si,n-1,next_side))
          found_solution = true;
      }

      repcoup();
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

/* Determine whether the slice has a solution in n half moves. Consult
 * hash table. 
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
static boolean branch_h_has_solution_in_n_recursive(slice_index si,
                                                    stip_length_type n,
                                                    Side side_at_move)
{
  boolean found_solution = false;
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (!inhash(si,hash_no_succ,n/2))
  {
    if (branch_h_has_solution_in_n_recursive_nohash(si,n,side_at_move))
      found_solution = true;
    else
      addtohash(si,hash_no_succ,n/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
static boolean branch_h_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  Side const starter = branch_h_starter_in_n(si,n);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",starter);

  if (n==slices[si].u.branch.length)
    result = branch_h_has_solution_in_n_recursive_nohash(si,n,starter);
  else
    result = branch_h_has_solution_in_n_recursive(si,n,starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_h_has_solution(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(full_length>=slack_length_help);

  while (len<full_length && !result)
  {
    if (branch_h_has_solution_in_n(si,len))
      result = true;

    len += 2;
  }

  result = result || branch_h_has_solution_in_n(si,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


/* Determine the starting side in a help branch in n
 * @param si slice index
 * @param n number of half-moves
 */
Side branch_h_starter_in_n(slice_index si, stip_length_type n)
{
  Side const branch_starter = slices[si].u.branch.starter;
  Side result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",branch_starter);

  result = (slices[si].u.branch.length%2==n%2
            ? branch_starter
            : advers(branch_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void branch_h_root_solve_in_n(slice_index si, stip_length_type n)
{
  Side const starter = branch_h_starter_in_n(si,n);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  branch_h_root_solve_in_n_recursive_nohash(si,n,starter);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_h_impose_starter(slice_index si, Side s)
{
  /* help play in N.5 -> change starter */
  Side next_starter = (slices[si].u.branch.length%2==1 ? advers(s) : s);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",s);

  TraceValue("%u\n",next_starter);

  slices[si].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,next_starter);

  TraceFunctionExit(__func__);
  TraceText("\n");
}
