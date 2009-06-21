#include "pybrah.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "pyslice.h"
#include "pyhelp.h"
#include "pyhelpha.h"
#include "pybrafrk.h"
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
  slice_index hashed;
  slice_index fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  hashed = alloc_help_hashed_slice(length,min_length,result);
  fork = alloc_branch_fork_slice(hashed,next);

  slices[result].type = STBranchHelp; 
  slices[result].u.branch.starter = no_side; 
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;
  slices[result].u.branch.next = fork;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = help_must_starter_resign(slices[si].u.branch.next);
  
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
void branch_h_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_write_unsolvability(slices[si].u.branch.next);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(slices[si].u.branch.next);

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
boolean branch_h_has_starter_apriori_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_apriori_lost(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_won(slices[si].u.branch.next);

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
boolean branch_h_has_starter_reached_goal(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_reached_goal(slices[si].u.branch.next);

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
boolean branch_h_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_is_goal_reached(just_moved,slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();
  
  result = ((!isIntelligentModeActive || isGoalReachable())
            && !echecc(nbply,side_at_move)
            && !slice_must_starter_resign(slices[si].u.branch.next));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >=1 solution was found
 */
boolean branch_h_root_solve_in_n(slice_index si, stip_length_type n)
{
  Side const starter = slices[si].u.branch.starter;
  Side const next_side = advers(starter);
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  active_slice[nbply+1] = si;
  genmove(starter);
  
  --MovesLeft[starter];

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !(OptFlag[restart] && MoveNbr<RestartNbr)
        && move_filter(si,n,starter))
    {
      if (!slice_must_starter_resign_hashed(slices[si].u.branch.next,starter)
          && help_solve_in_n(slices[si].u.branch.next,n-1,next_side))
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

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }
    
  ++MovesLeft[starter];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
boolean branch_h_solve_in_n(slice_index si,
                            stip_length_type n,
                            Side side_at_move)

{
  boolean result = false;
  Side const next_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  --MovesLeft[side_at_move];

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && move_filter(si,n,side_at_move))
    {
      if (!slice_must_starter_resign_hashed(slices[si].u.branch.next,
                                            side_at_move)
          && help_solve_in_n(slices[si].u.branch.next,n-1,next_side))
        result = true;
    }

    repcoup();

    /* Stop solving if a given number of solutions was encountered */
    if (OptFlag[maxsols] && solutions>=maxsolutions)
    {
      TraceValue("%u",maxsolutions);
      TraceValue("%u",solutions);
      TraceText("aborting\n");
      break;
    }

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }
    
  ++MovesLeft[side_at_move];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve full-length solutions in exactly n in help play at root level
 * @param si slice index
 * @param n number of half moves
 * @return true iff >=1 solution was found
 */
static boolean branch_h_root_solve_full_in_n(slice_index si,
                                             stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    result = Intelligent(si,n,n);
  else
    result = branch_h_root_solve_in_n(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    result = Intelligent(slices[si].u.branch.next,
                         n,
                         slices[si].u.branch.length);
  else
  {
    /* we only display move numbers when looking for full length
     * solutions (incl. full length set play)
     */
    Side const starter = branch_h_starter_in_n(si,n);
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    result = help_solve_in_n(slices[si].u.branch.next,n,starter);
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
slice_index branch_h_root_make_setplay_slice(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.branch.length>slack_length_help);

  if (slices[si].u.branch.length==slack_length_help+1)
    result = branch_find_slice_behind_fork(si);
  else
  {
    result = copy_slice(si);
    --slices[result].u.branch.length;
    --slices[result].u.branch.min_length;
    if (slices[result].u.branch.min_length<slack_length_help)
      slices[result].u.branch.min_length += 2;
    slices[result].u.branch.starter = advers(slices[si].u.branch.starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_output(si);

  if (echecc(nbply,advers(slices[si].u.branch.starter)))
    ErrorMsg(KingCapture);
  else
  {
    stip_length_type const full_length = slices[si].u.branch.length;
    stip_length_type len = (OptFlag[restart]
                            ? full_length
                            : slices[si].u.branch.min_length);

    TraceValue("%u",slices[si].u.branch.min_length);
    TraceValue("%u\n",slices[si].u.branch.length);

    assert(slices[si].u.branch.min_length>=slack_length_help);

    move_generation_mode = move_generation_not_optimized;

    FlagShortSolsReached = false;
    solutions = 0;

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

    if (FlagShortSolsReached && OptFlag[stoponshort])
      TraceText("aborting because of short solutions\n");
    else
      result = branch_h_root_solve_full_in_n(si,full_length);

    if (OptFlag[maxsols] && solutions>=maxsolutions)
      /* signal maximal number of solutions reached to outer world */
      FlagMaxSolsReached = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  Side starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_help);

  starter = branch_h_starter_in_n(si,full_length);

  while (len<full_length && !result)
  {
    if (help_solve_in_n(slices[si].u.branch.next,len,starter))
    {
      result = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  result = result || help_solve_in_n(si,full_length,starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
void branch_h_solve_continuations_in_n(table continuations,
                                       slice_index si,
                                       stip_length_type n,
                                       Side side_at_move)
{
  Side const next_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  --MovesLeft[side_at_move];

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && move_filter(si,n,side_at_move))
    {
      if (!slice_must_starter_resign_hashed(slices[si].u.branch.next,
                                            side_at_move)
          && help_solve_in_n(slices[si].u.branch.next,n-1,next_side))
      {
        append_to_top_table();
        coupfort();
      }
    }

    repcoup();

    /* Stop solving if a given number of solutions was encountered */
    if (OptFlag[maxsols] && solutions>=maxsolutions)
    {
      TraceValue("%u",maxsolutions);
      TraceValue("%u",solutions);
      TraceText("aborting\n");
      break;
    }

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }
    
  ++MovesLeft[side_at_move];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  Side const starter = branch_h_starter_in_n(si,len);
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<full_length && !solution_found)
  {
    help_solve_continuations_in_n(continuations,next,len,starter);
    if (table_length(continuations)>0)
    {
      solution_found = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  if (!solution_found)
    help_solve_continuations_in_n(continuations,next,full_length,starter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_h_root_write_key(slice_index si, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  write_attack(type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_has_solution_in_n(slice_index si,
                                   stip_length_type n,
                                   Side side_at_move)
{
  Side const next_side = advers(side_at_move);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = si;
  genmove(side_at_move);
  
  --MovesLeft[side_at_move];

  while (encore() && !result)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && move_filter(si,n,side_at_move))
    {
      if (!slice_must_starter_resign_hashed(slices[si].u.branch.next,
                                            side_at_move)
          && help_has_solution_in_n(slices[si].u.branch.next,n-1,next_side))
        result = true;
    }

    repcoup();
  }
    
  ++MovesLeft[side_at_move];

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  Side const starter = branch_h_starter_in_n(si,len);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<full_length)
    if (help_has_solution_in_n(slices[si].u.branch.next,len,starter))
    {
      result = true;
      break;
    }
    else
      len += 2;

  if (!result)
    result = help_has_solution_in_n(slices[si].u.branch.next,
                                    full_length,
                                    starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",branch_starter);

  result = (slices[si].u.branch.length%2==n%2
            ? branch_starter
            : advers(branch_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void find_relevant_slice_branch_fork(slice_index si,
                                            slice_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  traverse_slices(slices[si].u.branch_fork.next_towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_relevant_slice_found(slice_index si, slice_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *result = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_operation const relevant_slice_finders[] =
{
  &find_relevant_slice_found,         /* STBranchDirect */
  0,                                  /* STBranchDirectDefender */
  &find_relevant_slice_found,         /* STBranchHelp */
  &find_relevant_slice_found,         /* STBranchSeries */
  &find_relevant_slice_branch_fork,   /* STBranchFork */
  &find_relevant_slice_found,         /* STLeafDirect */
  &find_relevant_slice_found,         /* STLeafHelp */
  &find_relevant_slice_found,         /* STLeafSelf */
  &find_relevant_slice_found,         /* STLeafForced */
  &find_relevant_slice_found,         /* STReciprocal */
  &find_relevant_slice_found,         /* STQuodlibet */
  &find_relevant_slice_found,         /* STNot */
  &find_relevant_slice_found,         /* STMoveInverter */
  &find_relevant_slice_found          /* STHelpHashed */
};

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
  slice_index next_relevant = no_slice;
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&relevant_slice_finders,&next_relevant);
  traverse_slices(next,&st);

  TraceValue("%u",next_relevant);
  TraceValue("%u",slices[next_relevant].type);
  TraceValue("%u\n",even_length);

  switch (slices[next_relevant].type)
  {
    case STLeafDirect:
    {
      boolean const next_same_side_as_root = (even_length
                                              ? same_side_as_root
                                              : !same_side_as_root);
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = Black;
        slice_impose_starter(next,White);
      }
      else
        slices[si].u.branch.starter = (even_length
                                       ? slice_get_starter(next)
                                       : advers(slice_get_starter(next)));
      break;
    }

    case STLeafSelf:
    {
      boolean const next_same_side_as_root = (even_length
                                              ? same_side_as_root
                                              : !same_side_as_root);
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = White;
        slice_impose_starter(next,White);
      }
      else
        slices[si].u.branch.starter = (even_length
                                       ? slice_get_starter(next)
                                       : advers(slice_get_starter(next)));
      break;
    }

    case STLeafHelp:
    {
      boolean const next_same_side_as_root = (even_length
                                              ? same_side_as_root
                                              : !same_side_as_root);
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = White;
        slice_impose_starter(next,White);
      }
      else
        slices[si].u.branch.starter = (even_length
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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",s);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",next_starter);

  slices[si].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,next_starter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return if the branch slice represents a half-move only, the slice
 *         representing the subsequent play; otherwise si
 */
slice_index branch_h_shorten(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length==slack_length_help+1)
    result = branch_deallocate_to_fork(si);
  else
  {
    --slices[si].u.branch.length;
    --slices[si].u.branch.min_length;
    if (slices[si].u.branch.min_length<slack_length_help)
      slices[si].u.branch.min_length += 2;
    TraceValue("->%u",slices[si].u.branch.length);
    TraceValue("->%u\n",slices[si].u.branch.min_length);
    result = si;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
