#include "pybrah.h"
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

/* Allocate a STBranchHelp slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_h_slice(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u\n",next);

  slices[result].type = STBranchHelp; 
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
boolean branch_h_must_starter_resign(slice_index si)
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
void branch_h_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.branch.next);
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_h_has_non_starter_solved(slice_index si)
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
boolean branch_h_has_starter_apriori_lost(slice_index si)
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
boolean branch_h_has_starter_won(slice_index si)
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
boolean branch_h_has_starter_reached_goal(slice_index si)
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
boolean branch_h_is_goal_reached(Side just_moved, slice_index si)
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

static boolean branch_h_solve_in_n_recursive(slice_index si,
                                             stip_length_type n,
                                             Side side_at_move);

/* Is the move just played playable in a help play solution?
 * @param si slice index
 * @param n number of half moves (including the move just played)
 * @param side_at_move side that has just played
 * @return true iff the move just played is playable
 */
static boolean move_filter(slice_index si, stip_length_type n, Side side_at_move)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",side_at_move);

  if ((!isIntelligentModeActive || isGoalReachable())
      && !echecc(nbply,side_at_move))
  {
    (*encode)();
    result = !slice_must_starter_resign(slices[si].u.branch.next);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

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
 * @return true iff >=1 solution was found
 */
static boolean branch_h_root_solve_in_n_recursive_nohash(slice_index si,
                                                         stip_length_type n,
                                                         Side side_at_move)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_root_solve(slices[si].u.branch.next);
  else
  {
    Side const next_side = advers(side_at_move);

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && move_filter(si,n,side_at_move)
          && branch_h_solve_in_n_recursive(si,n-1,next_side))
        result = true;

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
  TraceFunctionResult("%u\n",result);
  return result;
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
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_solve(slices[si].u.branch.next);
  else
  {
    Side const next_side = advers(side_at_move);

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && move_filter(si,n,side_at_move)
          && branch_h_solve_in_n_recursive(si,n-1,next_side))
        result = true;

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
  TraceFunctionResult("%u\n",result);
  return result;
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
 * @return true iff >=1 solution was found
 */
static boolean branch_h_root_solve_full_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",n);

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    result = Intelligent(si,n);
  else
    result = branch_h_root_solve_in_n(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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
    result = Intelligent(si,n);
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

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_h_root_make_setplay_slice(slice_index si)
{
  slice_index const next = slices[si].u.branch.next;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(slices[si].u.branch.length>slack_length_help);

  if (slices[si].u.branch.length==slack_length_help+1)
    result = next;
  else
  {
    result = copy_slice(si);
    slices[result].u.branch.length -= 1;
    slices[result].u.branch.min_length -= 1;
    if (slices[result].u.branch.min_length<slack_length_help)
      slices[result].u.branch.min_length += 2;
    slices[result].u.branch.starter = advers(slices[si].u.branch.starter);
    hash_slice_is_derived_from(result,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with help play at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_h_root_solve(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  init_output(si);

  if (echecc(nbply,advers(slices[si].u.branch.starter)))
  {
    TraceText("hallo\n");
    ErrorMsg(KingCapture);
  }
  else
  {
    stip_length_type const full_length = slices[si].u.branch.length;
    stip_length_type len = slices[si].u.branch.min_length;

    TraceValue("%u",slices[si].u.branch.min_length);
    TraceValue("%u\n",slices[si].u.branch.length);

    assert(slices[si].u.branch.min_length>=slack_length_help);

    move_generation_mode = move_generation_not_optimized;

    while (len<full_length
           && !(OptFlag[stoponshort] && FlagShortSolsReached))
    {
      if (branch_h_root_solve_short_in_n(si,len))
      {
        FlagShortSolsReached = true;
        result = true;
      }

      len += 2;
    }

    if (!(FlagShortSolsReached && OptFlag[stoponshort]))
      result = branch_h_root_solve_full_in_n(si,full_length);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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

  assert(full_length>=slack_length_help-1);

  if (len==slack_length_help-1)
  {
    slice_solve(slices[si].u.branch.next);
    len +=2;
  }

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
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
static
void branch_h_solve_continuations_in_n_recursive_nohash(table continuations,
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
    slice_solve_continuations(continuations,slices[si].u.branch.next);
  else
  {
    Side const next_side = advers(side_at_move);

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && move_filter(si,n,side_at_move)
          && branch_h_solve_in_n_recursive(si,n-1,next_side))
      {
        append_to_top_table();
        coupfort();
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
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
static
void branch_h_solve_continuations_in_n_recursive(table continuations,
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
    branch_h_solve_continuations_in_n_recursive_nohash(continuations,
                                                       si,
                                                       n,
                                                       side_at_move);
    if (table_length(continuations)==0)
      addtohash(si,hash_no_succ,n/2);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice.
 * @param continuations table where to add first moves
 * @param si identifies slice being solved
 * @param n number of half moves until end state has to be reached
 */
static void branch_h_solve_continuations_in_n(table continuations,
                                              slice_index si,
                                              stip_length_type n)
{
  Side const starter = branch_h_starter_in_n(si,n);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",starter);

  if (n==slices[si].u.branch.length)
    branch_h_solve_continuations_in_n_recursive_nohash(continuations,
                                                       si,
                                                       n,
                                                       starter);
  else
    branch_h_solve_continuations_in_n_recursive(continuations,si,n,starter);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write post key play
 * @param si slice index
 */
void branch_h_solve_postkey(slice_index si)
{
  assert(slices[si].u.branch.length==slack_length_help+1);
  slice_solve_postkey(slices[si].u.branch.next);
}

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_h_solve_continuations(table continuations, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.branch.length;
  stip_length_type len = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(full_length>=slack_length_help);

  while (len<full_length && !solution_found)
  {
    branch_h_solve_continuations_in_n(continuations,si,len);
    if (table_length(continuations)>0)
    {
      solution_found = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  if (!solution_found)
    branch_h_solve_continuations_in_n(continuations,si,full_length);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_h_root_write_key(slice_index si, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",type);

  write_attack(type);

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
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    result = slice_has_solution(slices[si].u.branch.next);
  else
  {
    Side const next_side = advers(side_at_move);

    active_slice[nbply+1] = si;
    genmove(side_at_move);
  
    if (side_at_move==Black)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && move_filter(si,n,side_at_move)
          && branch_h_has_solution_in_n_recursive(si,n-1,next_side))
        result = true;

      repcoup();
    }
    
    if (side_at_move==Black)
      BlMovesLeft++;
    else
      WhMovesLeft++;

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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
  boolean result = false;
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (!inhash(si,hash_no_succ,n/2))
  {
    if (branch_h_has_solution_in_n_recursive_nohash(si,n,side_at_move))
      result = true;
    else
      addtohash(si,hash_no_succ,n/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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
 * @return true iff >=1 solution was found
 */
boolean branch_h_root_solve_in_n(slice_index si, stip_length_type n)
{
  Side const starter = branch_h_starter_in_n(si,n);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  result = branch_h_root_solve_in_n_recursive_nohash(si,n,starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_h_detect_starter(slice_index si,
                                               boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;
  boolean const even_length = slices[si].u.branch.length%2==0;
  slice_index const next = slices[si].u.branch.next;
  slice_index next_relevant = next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",same_side_as_root);
  
  if (slices[next].type==STMoveInverter)
    next_relevant = slices[next].u.move_inverter.next;

  TraceValue("%u",next_relevant);
  TraceValue("%u\n",even_length);

  switch (slices[next_relevant].type)
  {
    case STLeafDirect:
    {
      boolean const next_same_side_as_root =
          (even_length
           ? same_side_as_root
           : !same_side_as_root);
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = Black;
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
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = White;
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
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = White;
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
      result = slice_detect_starter(next,same_side_as_root);
      slices[si].u.branch.starter = (even_length
                                     ? slice_get_starter(next)
                                     : advers(slice_get_starter(next)));
      break;
  }

  TraceValue("%u\n",slices[si].u.branch.starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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
