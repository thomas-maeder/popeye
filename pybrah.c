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
  slices[result].starter = no_side; 
  slices[result].u.pipe.u.branch.length = length;
  slices[result].u.pipe.u.branch.min_length = min_length;
  slices[result].u.pipe.next = fork;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Wrap the branch(es) where play starts with a root slice.
 * Set st->param to the newly created slice.
 * @param branch identifies the branch slice to be wrapped
 * @param st address of structure holding traversal state
 * @return true
 */
boolean branch_h_make_root_slice(slice_index branch, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  *root_slice = alloc_help_root_slice(branch);

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

  result = help_must_starter_resign(slices[si].u.pipe.next);
  
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

  slice_write_unsolvability(slices[si].u.pipe.next);
  
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
boolean branch_h_has_starter_apriori_lost(slice_index si)
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
boolean branch_h_has_starter_won(slice_index si)
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
boolean branch_h_has_starter_reached_goal(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_reached_goal(slices[si].u.pipe.next);

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

  result = slice_is_goal_reached(just_moved,slices[si].u.pipe.next);

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
            && !slice_must_starter_resign(slices[si].u.pipe.next));

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
  Side const starter = slices[si].starter;
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
      if (!slice_must_starter_resign_hashed(slices[si].u.pipe.next,starter)
          && help_solve_in_n(slices[si].u.pipe.next,n-1,next_side))
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
      if (!slice_must_starter_resign_hashed(slices[si].u.pipe.next,
                                            side_at_move)
          && help_solve_in_n(slices[si].u.pipe.next,n-1,next_side))
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

    if (periods_counter>=nr_periods)
      break;
  }
    
  ++MovesLeft[side_at_move];

  finply();

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
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;
  Side starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_help);

  starter = branch_h_starter_in_n(si,full_length);

  while (len<full_length && !result)
  {
    if (help_solve_in_n(slices[si].u.pipe.next,len,starter))
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
      if (!slice_must_starter_resign_hashed(slices[si].u.pipe.next,
                                            side_at_move)
          && help_solve_in_n(slices[si].u.pipe.next,n-1,next_side))
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

    if (periods_counter>=nr_periods)
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
  assert(slices[si].u.pipe.u.branch.length==slack_length_help+1);
  slice_solve_postkey(slices[si].u.pipe.next);
}

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_h_solve_continuations(table continuations, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;
  Side const starter = branch_h_starter_in_n(si,len);
  slice_index const next = slices[si].u.pipe.next;

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
      if (!slice_must_starter_resign_hashed(slices[si].u.pipe.next,
                                            side_at_move)
          && help_has_solution_in_n(slices[si].u.pipe.next,n-1,next_side))
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
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;
  Side const starter = branch_h_starter_in_n(si,len);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<full_length)
    if (help_has_solution_in_n(slices[si].u.pipe.next,len,starter))
    {
      result = true;
      break;
    }
    else
      len += 2;

  if (!result)
    result = help_has_solution_in_n(slices[si].u.pipe.next,
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
  Side const branch_starter = slices[si].starter;
  Side result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[si].type);
  assert(slices[si].type==STBranchHelp);

  TraceValue("%u\n",branch_starter);

  result = (slices[si].u.pipe.u.branch.length%2==n%2
            ? branch_starter
            : advers(branch_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_relevant_slice_branch_fork(slice_index si,
                                               slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(slices[si].u.pipe.u.branch_fork.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_relevant_slice_found(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  slice_index * const to_be_found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *to_be_found = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
  &find_relevant_slice_found,         /* STHelpRoot */
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
  boolean const even_length = slices[si].u.pipe.u.branch.length%2==0;
  slice_index const next = slices[si].u.pipe.next;
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
      if (slices[next].starter==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].starter = Black;
        slice_impose_starter(next,White);
      }
      else
        slices[si].starter = (even_length
                                       ? slices[next].starter
                                       : advers(slices[next].starter));
      break;
    }

    case STLeafSelf:
    {
      boolean const next_same_side_as_root = (even_length
                                              ? same_side_as_root
                                              : !same_side_as_root);
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slices[next].starter==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].starter = White;
        slice_impose_starter(next,White);
      }
      else
        slices[si].starter = (even_length
                                       ? slices[next].starter
                                       : advers(slices[next].starter));
      break;
    }

    case STLeafHelp:
    {
      boolean const next_same_side_as_root = (even_length
                                              ? same_side_as_root
                                              : !same_side_as_root);
      result = slice_detect_starter(next,next_same_side_as_root);
      if (slices[next].starter==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].starter = White;
        slice_impose_starter(next,White);
      }
      else
        slices[si].starter = (even_length
                                       ? slices[next].starter
                                       : advers(slices[next].starter));
      break;
    }

    default:
      result = slice_detect_starter(next,same_side_as_root);
      slices[si].starter = (even_length
                                     ? slices[next].starter
                                     : advers(slices[next].starter));
      break;
  }

  TraceValue("%u\n",slices[si].starter);

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
  Side next_starter = (slices[si].u.pipe.u.branch.length%2==1 ? advers(s) : s);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",s);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",next_starter);

  slices[si].starter = s;
  slice_impose_starter(slices[si].u.pipe.next,next_starter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/*************** root *****************/

slice_index alloc_help_root_slice(slice_index si)
{
  slice_index result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STBranchHelp);

  slices[result].type = STHelpRoot;
  slices[result].u.root_branch.short_length = branch_find_fork(si);
  slices[result].u.root_branch.full_length = si;
  slices[result].u.root_branch.length = slices[si].u.pipe.u.branch.length;
  slices[result].u.root_branch.min_length
      = slices[si].u.pipe.u.branch.min_length;
  slices[result].starter = slices[si].starter;
  TraceValue("%u",slices[result].starter);
  TraceValue("%u",slices[result].u.root_branch.full_length);
  TraceValue("%u\n",slices[result].u.root_branch.short_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a help pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
static void branch_h_shorten_help_pipe(slice_index pipe)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  --slices[pipe].u.pipe.u.branch.length;
  --slices[pipe].u.pipe.u.branch.min_length;
  if (slices[pipe].u.pipe.u.branch.min_length<slack_length_help)
    slices[pipe].u.pipe.u.branch.min_length += 2;
  slices[pipe].starter = advers(slices[pipe].starter);
  TraceValue("%u",slices[pipe].starter);
  TraceValue("%u",slices[pipe].u.pipe.u.branch.length);
  TraceValue("%u\n",slices[pipe].u.pipe.u.branch.min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten a help root branch by a half-move
 * @param root identifies root branch to be shortened
 */
static void branch_h_shorten_help_root(slice_index root)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  --slices[root].u.root_branch.length;
  --slices[root].u.root_branch.min_length;
  if (slices[root].u.root_branch.min_length<slack_length_help)
    slices[root].u.root_branch.min_length += 2;
  slices[root].starter = advers(slices[root].starter);
  TraceValue("%u",slices[root].starter);
  TraceValue("%u",slices[root].u.root_branch.length);
  TraceValue("%u\n",slices[root].u.root_branch.min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_h_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index const full_length = slices[si].u.root_branch.full_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STHelpRoot);
  assert(slices[si].u.root_branch.length>slack_length_help);

  if (slices[si].u.root_branch.length==slack_length_help+1)
  {
    slice_index const fork = branch_find_fork(si);
    result = slices[fork].u.pipe.u.branch_fork.towards_goal;
  }
  else
  {
    slice_index const full_length_copy = copy_slice(full_length);
    assert(slices[full_length].type==STBranchHelp);
    branch_h_shorten_help_pipe(full_length_copy);

    result = copy_slice(si);
    slices[result].u.root_branch.full_length = full_length_copy;
    branch_h_shorten_help_root(result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean traverse_and_shorten_help_pipe(slice_index pipe,
                                              slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  branch_h_shorten_help_pipe(pipe);
  result = slice_traverse_children(pipe,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean traverse_and_shorten_help_root(slice_index root,
                                              slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  branch_h_shorten_help_root(root);
  result = traverse_slices(slices[root].u.root_branch.full_length,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean traverse_and_shorten_branch_fork(slice_index fork,
                                                slice_traversal *st)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  slices[fork].starter = advers(slices[fork].starter);
  result = traverse_slices(slices[fork].u.pipe.next,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_shorteners[] =
{
  0,                                 /* STBranchDirect */
  0,                                 /* STBranchDirectDefender */
  &traverse_and_shorten_help_pipe,   /* STBranchHelp */
  0,                                 /* STBranchSeries */
  &traverse_and_shorten_branch_fork, /* STBranchFork */
  0,                                 /* STLeafDirect */
  0,                                 /* STLeafHelp */
  0,                                 /* STLeafSelf */
  0,                                 /* STLeafForced */
  0,                                 /* STReciprocal */
  0,                                 /* STQuodlibet */
  0,                                 /* STNot */
  0,                                 /* STMoveInverter */
  &traverse_and_shorten_help_root,   /* STHelpRoot */
  &traverse_and_shorten_help_pipe    /* STHelpHashed */
};

/* Shorten a help branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return - no_slice if not applicable (already shortened)
 *         - slice representing subsequent play if root has 1 half-move only
 *         - root (shortened) otherwise
 */
slice_index branch_h_root_shorten(slice_index root)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  assert(slices[root].type==STHelpRoot);

  if ((slices[root].u.root_branch.length-slack_length_help)%2==0)
    result = no_slice;
  else if (slices[root].u.root_branch.length==slack_length_help+1)
    result = branch_deallocate_to_fork(root);
  else
  {
    slice_traversal st;
    slice_traversal_init(&st,&slice_shorteners,0);
    traverse_slices(root,&st);
    result = root;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param root identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 * TODO can we get rid of this???
 */
who_decides_on_starter branch_h_root_detect_starter(slice_index root,
                                                    boolean same_side_as_root)
{
  who_decides_on_starter result;
  slice_index const full_length = slices[root].u.root_branch.full_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  result = slice_detect_starter(full_length,same_side_as_root);
  slices[root].starter = slices[full_length].starter;
  TraceValue("->%u\n",slices[root].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve full-length solutions in exactly n in help play at root level
 * @param root slice index
 * @param n number of half moves
 * @return true iff >=1 solution was found
 */
static boolean branch_h_root_solve_full_in_n(slice_index root,
                                             stip_length_type n)
{
  slice_index const full_length = slices[root].u.root_branch.full_length;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    result = Intelligent(full_length,n,n);
  else
    result = branch_h_root_solve_in_n(full_length,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve short solutions in exactly n in help play at root level.
 * @param root slice index
 * @param n number of half moves
 * @return true iff >=1 short solution was found
 */
static boolean branch_h_root_solve_short_in_n(slice_index root,
                                              stip_length_type n)
{
  slice_index const short_length = slices[root].u.root_branch.short_length;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u",slices[root].u.root_branch.full_length);
  TraceValue("%u\n",slices[root].u.root_branch.short_length);

  assert(n>=slack_length_help);

  if (isIntelligentModeActive)
    result = Intelligent(short_length,n,slices[root].u.root_branch.length);
  else
  {
    stip_length_type const
        full_length = slices[root].u.root_branch.full_length;
    Side const starter = branch_h_starter_in_n(full_length,n);

    /* we only display move numbers when looking for full length
     * solutions (incl. full length set play)
     */
    boolean const save_movenbr = OptFlag[movenbr];
    OptFlag[movenbr] = false;
    result = help_solve_in_n(short_length,n,starter);
    OptFlag[movenbr] = save_movenbr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a composite slice with help play at root level
 * @param root slice index
 * @return true iff >=1 solution was found
 */
boolean branch_h_root_solve(slice_index root)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  init_output(root);

  if (echecc(nbply,advers(slices[root].starter)))
    ErrorMsg(KingCapture);
  else
  {
    stip_length_type const full_length = slices[root].u.root_branch.length;
    stip_length_type len = (OptFlag[restart]
                            ? full_length
                            : slices[root].u.root_branch.min_length);

    TraceValue("%u",slices[root].u.root_branch.min_length);
    TraceValue("%u\n",slices[root].u.root_branch.length);

    assert(slices[root].u.root_branch.min_length>=slack_length_help);

    move_generation_mode = move_generation_not_optimized;

    FlagShortSolsReached = false;
    solutions = 0;

    while (len<full_length
           && !(OptFlag[stoponshort] && FlagShortSolsReached))
    {
      if (branch_h_root_solve_short_in_n(root,len))
      {
        FlagShortSolsReached = true;
        result = true;
      }

      len += 2;
    }

    if (FlagShortSolsReached && OptFlag[stoponshort])
      TraceText("aborting because of short solutions\n");
    else
      result = branch_h_root_solve_full_in_n(root,full_length);

    if (OptFlag[maxsols] && solutions>=maxsolutions)
      /* signal maximal number of solutions reached to outer world */
      FlagMaxSolsReached = true;
  }

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
boolean branch_h_root_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = help_must_starter_resign(slices[si].u.root_branch.full_length);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_h_root_impose_starter(slice_index si, Side s)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",s);
  TraceFunctionParamListEnd();

  slice_impose_starter(slices[si].u.root_branch.full_length,s);
  slices[si].starter = s;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_h_root_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = branch_h_has_solution(slices[si].u.root_branch.full_length);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
