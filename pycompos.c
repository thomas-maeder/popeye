#include "pycompos.h"
#include "pyquodli.h"
#include "pysequen.h"
#include "pyrecipr.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyoutput.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

/* Determine whether the current position is stored in the hash table
 * and how.
 * @param si slice index
 * @param hb address of HashBuffer to hold the encoded current position
 * @param n number of moves until goal
 * @param result address where to write hash result (if any)
 * @return true iff the current position was found and *result was
 *              assigned
 */
static boolean composite_d_is_in_hash(slice_index si,
                                      HashBuffer *hb,
                                      stip_length_type n,
                                      boolean *hash_val)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  /* It is more likely that a position has no solution.           */
  /* Therefore let's check for "no solution" first.  TLi */
  if (inhash(si,DirNoSucc,n-slack_length_direct+1,hb))
  {
    TraceText("inhash(si,DirNoSucc,n,hb)\n");
    assert(!inhash(si,DirSucc,n-slack_length_direct,hb));
    *hash_val = false;
    result = true;
  }
  else if (inhash(si,DirSucc,n-slack_length_direct,hb))
  {
    TraceText("inhash(si,DirSucc,n-slack_length_direct,hb)\n");
    *hash_val = true;
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacking side wins at end of composite slice
 * @param attacker attacking side
 * @param si slice identifier
 * @return truee iff attacker wins
 */
static boolean composite_end_has_solution(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",slices[si].type);

  switch (slices[si].type)
  {
    case STQuodlibet:
      result = quodlibet_end_has_solution(si);
      break;

    case STReciprocal:
      result = reci_end_has_solution(si);
      break;

    case STSequence:
      result = sequence_end_has_solution(si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",si);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* TODO find out whether we can arrange the functions so that this
 * declaration is not necessary.
 */
static boolean composite_d_has_solution_in_n(slice_index si,
                                             stip_length_type n);

/* Count all non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * @return number of defender's non-trivial moves minus 1 (TODO: why?)
 */
static int count_non_trivial(slice_index si)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  int result = -1;

  genmove(defender);

  while (encore() && max_nr_nontrivial>=result)
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,defender)
        && !(min_length_nontrivial>0
             && composite_d_has_solution_in_n(si,min_length_nontrivial)))
      ++result;
    repcoup();
  }

  finply();

  return result;
}

typedef enum
{
  already_won,
  short_win,
  win,
  loss,
  short_loss,
  already_lost
} d_defender_win_type;

/* Determine whether the defending side wins in n (>1) in direct play.
 * @param defender defending side
 * @param si slice identifier
 * @return true iff defender wins
 */
static
d_defender_win_type composite_d_helper_does_defender_win(slice_index si,
                                                         stip_length_type n)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  boolean is_defender_immobile = true;
  boolean refutation_found = false;
  int ntcount = 0;
  d_defender_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  /* check whether `black' can reach a position that is already
  ** marked unsolvable for white in the hash table. */
  /* TODO should we? i.e. do it or remove comment */

  if (n>max_len_threat
	  && !echecc(nbply,defender)
	  && !composite_d_has_solution_in_n(si,max_len_threat))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%u\n",win);
	return win;
  }

  if (OptFlag[solflights] && has_too_many_flights(defender))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%u\n",win);
	return win;
  }

  /* Check whether defender has more non trivial moves than he is
	 allowed to have. The number of such moves allowed
	 (max_nr_nontrivial) is entered using the nontrivial option.
  */
  if (n>min_length_nontrivial)
  {
	ntcount = count_non_trivial(si);
	if (max_nr_nontrivial<ntcount)
    {
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",win);
	  return win;
    }
	else
	  max_nr_nontrivial -= ntcount;
  } /* nontrivial */

  move_generation_mode=
      n>1
      ? move_generation_mode_opti_per_side[defender]
      : move_generation_optimized_by_killer_move;
  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (!refutation_found && encore())
  {
	if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender))
	{
	  is_defender_immobile = false;
	  if (!composite_d_has_solution_in_n(si,n))
	  {
        TraceText("refutes\n");
		refutation_found = true;
		coupfort();
	  }
	}

	repcoup();
  }

  finply();

  if (n>min_length_nontrivial)
	max_nr_nontrivial += ntcount;

  result = refutation_found || is_defender_immobile ? win : loss;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param defender defending side (at move)
 * @param n number of moves until end state has to be reached, not
 *          including the attacker's move just played
 * @return true iff defender wins
 */
static d_defender_win_type composite_d_does_defender_win(slice_index si,
                                                         stip_length_type n)
{
  d_defender_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=slack_length_direct);

  TraceValue("%u\n",slices[si].u.composite.min_length);

  if (slice_end_has_starter_lost(si))
    result = already_won;
  else if ((slices[si].u.composite.length-n
            >slices[si].u.composite.min_length-slack_length_direct)
           && slice_end_has_starter_won(si))
    result = short_loss;
  else
    result = composite_d_helper_does_defender_win(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacking side wins in the middle of a
 * composite slice
 * @param attacker attacking side
 * @param si slice identifier
 */
static boolean composite_d_helper_has_solution(slice_index si,
                                               stip_length_type n)
{
  Side const attacker = slices[si].starter;
  boolean win_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  assert(n>slack_length_direct);

  genmove(attacker);

  while (!win_found && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker))
    {
      if (composite_d_does_defender_win(si,n-1)>=loss)
      {
        TraceValue("%u",n);
        TraceText(" wins\n");
        win_found = true;
        coupfort();
      }
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
  TraceValue("%u",n);
  TraceFunctionResult("%u\n",win_found);
  return win_found;
}

/* Determine whether attacker can end in n moves of direct play.
 * This is a recursive function.
 * @param n number of moves left until the end state has to be reached
 * @return true iff attacker can end in n moves
 */
static boolean composite_d_has_solution_in_n(slice_index si,
                                             stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (slice_end_has_non_starter_refuted(si))
    ; /* intentionally nothing */
  else
  {
    stip_length_type const moves_played = slices[si].u.composite.length-n;
    stip_length_type const
        min_moves_played = (slices[si].u.composite.min_length
                            -slack_length_direct);
    TraceValue("%u",moves_played);
    TraceValue("%u\n",min_moves_played);
    if (moves_played>=min_moves_played
        && composite_end_has_solution(si))
      result = true;
    else if (moves_played>min_moves_played
             && slice_has_non_starter_solved(si))
      result = true;
    else if (n>slack_length_direct)
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!composite_d_is_in_hash(si,&hb,n,&result))
      {
        stip_length_type i;
        stip_length_type n_min = slack_length_direct+1;

        if (min_moves_played>moves_played-slack_length_direct)
          n_min = min_moves_played-(moves_played-slack_length_direct);

        for (i = n_min; !result && i<=n; i++)
        {
          if (i-1>max_len_threat || i>min_length_nontrivial)
            i = n;

          result = composite_d_helper_has_solution(si,i);

          if (maxtime_status==MAXTIME_TIMEOUT)
            break;
        }
      }

      if (result)
        addtohash(si,DirSucc,n-slack_length_direct,&hb);
      else
        addtohash(si,DirNoSucc,n-slack_length_direct+1,&hb);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether attacker can win in direct play.
 * @param si slice index
 * @return true iff attacker can win
 */
static boolean composite_d_has_solution(slice_index si)
{
  stip_length_type const n = slices[si].u.composite.length;
  return composite_d_has_solution_in_n(si,n);
}

/* Determine whether a composite slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean composite_has_solution(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      result = composite_d_has_solution(si);
      break;

    case PHelp:
      /* TODO */
      break;

    case PSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find refutations after a move of the attacking side.
 * @param defender defending side
 * @param t table where to store refutations
 * @return 0  if the defending side has at >=1 final moves in reflex play
 *         max_nr_refutations+1 if
 *            if the defending side is immobile (it shouldn't be here!)
 *            if the defending side has more non-trivial moves than allowed
 *            if the defending king has more flights than allowed
 *            if there is no threat in <= the maximal number threat
 *               length as entered by the user
 *         number (0..max_nr_refutations) of refutations otherwise
 */
static int composite_d_find_refutations(int t, slice_index si)
{
  Side const defender = advers(slices[si].starter);
  boolean is_defender_immobile = true;
  int ntcount = 0;
  int result = 0;
  stip_length_type const n = slices[si].u.composite.length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n>max_len_threat
      && !echecc(nbply,defender)
      && !composite_d_has_solution_in_n(si,max_len_threat))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%u\n",result);
    return max_nr_refutations+1;
  }

  if (n>slack_length_direct+1
      && OptFlag[solflights] && has_too_many_flights(defender))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%u\n",result);
    return max_nr_refutations+1;
  }

  if (n>min_length_nontrivial)
  {
    ntcount = count_non_trivial(si);
    if (max_nr_nontrivial<ntcount)
    {
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",result);
      return max_nr_refutations+1;
    }
    else
      max_nr_nontrivial -= ntcount;
  }

  if (n>slack_length_direct+1)
    move_generation_mode= move_generation_mode_opti_per_side[defender];

  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore()
         && tablen(t)<=max_nr_refutations)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender))
    {
      is_defender_immobile = false;
      if (!composite_d_has_solution_in_n(si,n))
      {
        TraceText("refutes\n");
        pushtabsol(t);
      }
    }

    repcoup();
  }
  finply();

  if (n>min_length_nontrivial)
    max_nr_nontrivial += ntcount;

  result = is_defender_immobile ? max_nr_refutations+1 : tablen(t);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * We are at the end of a slice and delegate to the child slice(s)
 * @param si slice index
 */
static void composite_end_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",slices[si].type);

  switch (slices[si].type)
  {
    case STQuodlibet:
      quodlibet_end_solve_variations(si);
      break;

    case STReciprocal:
      reci_end_solve_variations(si);
      break;

    case STSequence:
      sequence_end_solve_variations(si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve at root level at the end of a composite slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
static boolean composite_root_end_solve(slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      result = quodlibet_root_end_solve(si);
      break;

    case STReciprocal:
      result = reci_root_end_solve(si);
      break;

    case STSequence:
      result = sequence_root_end_solve(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Continue solving at the end of a composite slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
static boolean composite_end_solve(slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      result = quodlibet_end_solve(si);
      break;

    case STReciprocal:
      result = reci_end_solve(si);
      break;

    case STSequence:
      result = sequence_end_solve(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * We are at the end of a slice and delegate to the child slice(s)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
static boolean composite_end_is_unsolvable(slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STReciprocal:
      result = reci_end_is_unsolvable(si);
      break;

    case STQuodlibet:
      result = quodlibet_end_is_unsolvable(si);
      break;

    case STSequence:
      result = sequence_end_is_unsolvable(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

static boolean composite_h_exact_solve_recursive(slice_index si,
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
 * @return true iff >= 1 solution has been found
 */
static boolean
composite_h_root_exact_solve_recursive_nohash(slice_index si,
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
    found_solution = composite_root_end_solve(si);
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
          && !composite_end_is_unsolvable(si)
          && composite_h_exact_solve_recursive(si,n-1,next_side))
        found_solution = true;

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
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
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
static boolean composite_h_exact_solve_recursive_nohash(slice_index si,
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
    found_solution = composite_end_solve(si);
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
          && !composite_end_is_unsolvable(si)
          && composite_h_exact_solve_recursive(si,n-1,next_side))
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

/* Determine and write the solution(s) in a help stipulation at root level.
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
static boolean composite_h_root_exact_solve_recursive(slice_index si,
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
    if (composite_h_root_exact_solve_recursive_nohash(si,n,side_at_move))
      found_solution = true;
    else
      addtohash(si,hash_no_succ,n/2,&hb);
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
static boolean composite_h_exact_solve_recursive(slice_index si,
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
    if (composite_h_exact_solve_recursive_nohash(si,n,side_at_move))
      found_solution = true;
    else
      addtohash(si,hash_no_succ,n/2,&hb);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Determine and write the solution(s) in a help stipulation at root level.
 * @param si identifies slice being solved
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
static boolean composite_h_root_exact_solve(slice_index si,
                                            stip_length_type n,
                                            Side starter)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",starter);

  if (n==slices[si].u.composite.length)
    result = composite_h_root_exact_solve_recursive_nohash(si,n,starter);
  else
    result = composite_h_root_exact_solve_recursive(si,n,starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the solution(s) in a help stipulation.
 * @param si identifies slice being solved
 * @return true iff >= 1 solution was found
 */
static boolean composite_h_exact_solve(slice_index si)
{
  boolean result;
  stip_length_type const n = slices[si].u.composite.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",n);

  if (n==slices[si].u.composite.length)
    result = composite_h_exact_solve_recursive_nohash(si,
                                                      n,
                                                      slices[si].starter);
  else
    result = composite_h_exact_solve_recursive(si,n,slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with series play
 * @param n exact number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean composite_ser_exact_solve_recursive(slice_index si,
                                                   stip_length_type n)
{
  boolean solution_found = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_series)
    solution_found = composite_end_solve(si);
  else
  {
    Side const series_side = slices[si].starter;
    Side other_side = advers(series_side);

    if (!slice_is_unsolvable(si))
    {
      genmove(series_side);
      active_slice[nbply] = si;

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
        else if (echecc(nbply,other_side))
          TraceText("echecc(nbply,other_side)\n");
        else
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (inhash(si,SerNoSucc,n-slack_length_series,&hb))
            TraceText("in hash\n");
          else if (composite_ser_exact_solve_recursive(si,n-1))
            solution_found = true;
          else
            addtohash(si,SerNoSucc,n-slack_length_series,&hb);
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
 * @param n exact number of moves to reach the end state
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean
composite_ser_root_exact_solve_recursive(slice_index si, stip_length_type n)
{
  boolean solution_found = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_series)
    solution_found = composite_root_end_solve(si);
  else
  {
    Side const series_side = slices[si].starter;
    Side const other_side = advers(series_side);

    if (!slice_is_unsolvable(si))
    {
      genmove(series_side);
      active_slice[nbply] = si;

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
        else if (echecc(nbply,other_side))
          TraceText("echecc(nbply,other_side)\n");
        else
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (inhash(si,SerNoSucc,n-slack_length_series,&hb))
            TraceText("in hash\n");
          else if (composite_ser_exact_solve_recursive(si,n-1))
            solution_found = true;
          else
            addtohash(si,SerNoSucc,n-slack_length_series,&hb);
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
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
}

/* Determine and write set play of a direct/self/reflex stipulation
 * We are at the end of a slice and delegate to the child slice(s)
 * @param si slice index
 */
static boolean composite_root_end_solve_setplay(slice_index si)
{
  boolean result = false;

  switch (slices[si].type)
  {
    case STSequence:
      result = sequence_root_end_solve_setplay(si);
      break;

    case STReciprocal:
      result = reci_root_end_solve_setplay(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_end_solve_setplay(si);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Solve a series play problem at root level in exactly N moves
 * (N>=2); decide whether to solve in intelligent mode or not. 
 * @param si slice index
 * @param n number of half moves
 * @return true iff >= 1 solution was found
 */
static boolean
composite_ser_root_exact_solve_intelligent_or_not(slice_index si,
                                                  stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=1);

  if (isIntelligentModeActive)
    result = Intelligent(n,slices[si].starter);
  else
  {
    /* we only display move numbers when looking for full length
     * solutions
     */
    boolean const save_movenbr = OptFlag[movenbr];
    if (n<slices[si].u.composite.length)
      OptFlag[movenbr] = false;
    result = composite_ser_root_exact_solve_recursive(si,n);
    OptFlag[movenbr] = save_movenbr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with series play at root level
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean composite_ser_root_solve(slice_index si)
{
  boolean result = false;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",starter);
  
  init_output_mode(output_mode_line);

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
      output_start_setplay_level();
      composite_root_end_solve_setplay(si);
      output_end_setplay_level();
      isIntelligentModeActive = save_isIntelligentModeActive;
    }

    Message(NewLine); /* TODO use higher level output function */
  }

  solutions= 0;    /* reset after set play */
  FlagShortSolsReached= false;

  if (echecc(nbply,advers(starter)))
    ErrorMsg(KingCapture);
  else
  {
    boolean full_length_solved = false;
    stip_length_type const full_length = slices[si].u.composite.length;

    stip_length_type len;
    for (len = slices[si].u.composite.min_length;
         len<full_length && !(OptFlag[stoponshort] && FlagShortSolsReached);
         len++)
      if (composite_ser_root_exact_solve_intelligent_or_not(si,len))
        FlagShortSolsReached = true;

    if (!(FlagShortSolsReached && OptFlag[stoponshort]))
      full_length_solved =
          composite_ser_root_exact_solve_intelligent_or_not(si,full_length);

    result = FlagShortSolsReached || full_length_solved;
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
/*boolean composite_ser_root_solve(slice_index si,
                                 stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",n);

  if (n==slices[si].u.composite.min_length)
    result = composite_ser_root_exact_solve_recursive(si,n);
  else if (OptFlag[restart])
    result = composite_ser_root_maximal_solve(n,si);
  else
    result = composite_ser_root_solve(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}*/

/* Solve a composite slice at root level.
 * This is the interface for intelligent mode.
 * @param si slice index
 * @param n exact number of moves until the slice's goal has to be
 *          reached (this may be shorter than the slice's length if
 *          we are searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean composite_root_exact_solve(slice_index si,
                                   stip_length_type n,
                                   Side starter)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",starter);

  assert(isIntelligentModeActive);

  switch (slices[si].u.composite.play)
  {
    case PHelp:
      result = composite_h_root_exact_solve(si,n,starter);
      break;

    case PSeries:
      /* TODO what to do with starter? */
      result = composite_ser_root_exact_solve_recursive(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with series play
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean composite_ser_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.composite.length;
  stip_length_type i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].u.composite.length);

  for (i = slices[si].u.composite.min_length; i<n; i++)
    if (composite_ser_exact_solve_recursive(si,i))
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
      && composite_ser_exact_solve_recursive(si,n))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the solutions in the current position.
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean composite_solve(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      /* TODO */
      break;

    case PHelp:
      /* TODO implement composite_h_solve() */
      result = composite_h_exact_solve(si);
      break;

    case PSeries:
      result = composite_ser_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
static boolean composite_end_is_threat_refuted(slice_index si)
{
  boolean result = true;

  switch (slices[si].type)
  {
    case STQuodlibet:
      result = quodlibet_end_is_threat_refuted(si);
      break;

    case STReciprocal:
      result = reci_end_is_threat_refuted(si);
      break;

    case STSequence:
      result = sequence_end_is_threat_refuted(si);
      break;

    default:
      assert(0);
  }

  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
static boolean composite_d_is_threat_in_n_refuted(slice_index si,
                                                  stip_length_type n)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_direct)
    result = composite_end_is_threat_refuted(si);
  else
    result = composite_d_does_defender_win(si,n-1)<=win;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
static boolean composite_d_is_threat_refuted(slice_index si)
{
  stip_length_type const n = slices[si].u.composite.length;
  return composite_d_is_threat_in_n_refuted(si,n);
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean composite_is_threat_refuted(slice_index si)
{
  boolean result = false;

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      result = composite_d_is_threat_refuted(si);
      break;

    case PHelp:
      /* TODO */
      break;

    case PSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Determine whether the move just played by the defending side
 * defends against the threats.
 * @param n number of moves until end state has to be reached from now
 * @param threats table containing the threats
 * @return true iff the move just played defends against at least one
 *         of the threats
 */
static boolean composite_d_defends_against_threats(int threats,
                                                   slice_index si,
                                                   stip_length_type n)
{
  Side const attacker = slices[si].starter;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",tablen(threats));
  if (tablen(threats)>0)
  {
    int nr_successful_threats = 0;
    boolean defense_found = false;

    genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && nowdanstab(threats)
          && !echecc(nbply,attacker))
      {
        TraceText("checking threat\n");
        defense_found = composite_d_is_threat_in_n_refuted(si,n);
        if (defense_found)
        {
          TraceText("defended\n");
          coupfort();
        }
        else
          nr_successful_threats++;
      }

      repcoup();
    }

    finply();

    /* this happens if we have found a defense or some threats can no
     * longer be played after defender's defense. */
    result = nr_successful_threats<tablen(threats);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* TODO can we rearrange the functions so that this declaration is no
 * necessary? */
static void composite_d_solve_continuations_in_n(int continuations,
                                                 slice_index si,
                                                 stip_length_type n);

/* Write a variation in the try/solution/set play of a
 * direct/self/reflex stipulation. The move of the defending side that
 * starts the variation has already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached from now
 */
static void composite_d_write_variation(slice_index si, stip_length_type n)
{
  boolean isRefutation = true; /* until we prove otherwise */
  stip_length_type i;
  stip_length_type const min_len = (slices[si].u.composite.min_length>n
                                    ? n
                                    : slices[si].u.composite.min_length);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  write_defense();

  output_start_continuation_level();
  
  for (i = min_len; i<=n && isRefutation; i++)
  {
    int const continuations = alloctab();
    composite_d_solve_continuations_in_n(continuations,si,i);
    isRefutation = tablen(continuations)==0;
    freetab();
  }

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the threats in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param threats table where to add threats
 * @return the length of the shortest threat(s); 1 if there is no threat
 */
static int composite_d_solve_threats(int threats,
                                     slice_index si,
                                     stip_length_type n)
{
  Side const defender = advers(slices[si].starter);
  int result = 1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (!(OptFlag[nothreat] || echecc(nbply,defender)))
  {
    stip_length_type max_threat_length = (n-1>max_len_threat
                                          ? max_len_threat
                                          : n-1);
    stip_length_type i;

    output_start_threat_level();

    for (i = 1; i<=max_threat_length; i++)
    {
      composite_d_solve_continuations_in_n(threats,si,i);
      if (tablen(threats)>0)
      {
        result = i;
        break;
      }
    }

    output_end_threat_level();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param refutations table containing refutations after move just
 *                    played
 */
static void composite_d_root_solve_variations(int len_threat,
                                              int threats,
                                              int refutations,
                                              slice_index si,
                                              stip_length_type n)
{
  Side const attacker = slices[si].starter;
  Side defender = advers(attacker);
  int ntcount = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u\n",si);

  if (n-1>min_length_nontrivial)
  {
    ntcount = count_non_trivial(si);
    max_nr_nontrivial -= ntcount;
  }

  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender)
        && !nowdanstab(refutations))
    {
      if (n>2 && OptFlag[noshort]
          && composite_d_has_solution_in_n(si,n-2))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (len_threat>1
               && composite_d_has_solution_in_n(si,len_threat-1))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (slice_has_non_starter_solved(si))
        ; /* oops! */
      else if (!composite_d_defends_against_threats(threats,si,len_threat))
        ; /* move doesn't defend against threat */
      else
        composite_d_write_variation(si,n-1);
    }

    repcoup();
  }

  finply();

  if (n-1>min_length_nontrivial)
    max_nr_nontrivial += ntcount;
  
  TraceFunctionExit(__func__);
}

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param len_threat length of threats
 * @param threats table containing threats
 */
static void composite_d_solve_variations_in_n(int len_threat,
                                              int threats,
                                              slice_index si,
                                              stip_length_type n)
{
  Side const attacker = slices[si].starter;
  Side defender = advers(attacker);
  int ntcount = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u\n",si);

  if (n-1>min_length_nontrivial)
  {
    ntcount = count_non_trivial(si);
    max_nr_nontrivial -= ntcount;
  }

  genmove(defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,defender))
    {
      if (n>2 && OptFlag[noshort]
          && composite_d_has_solution_in_n(si,n-2))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (len_threat>1
               && composite_d_has_solution_in_n(si,len_threat-1))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (slice_has_non_starter_solved(si))
        ; /* oops! short end. */
      else if (!composite_d_defends_against_threats(threats,si,len_threat))
        ; /* move doesn't defend against threat */
      else
        composite_d_write_variation(si,n-1);
    }

    repcoup();
  }

  finply();

  if (n-1>min_length_nontrivial)
    max_nr_nontrivial += ntcount;
  
  TraceFunctionExit(__func__);
}

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * @param si slice index
 */
static void composite_d_solve_variations(slice_index si)
{
  stip_length_type const n = slices[si].u.composite.length;
  stip_length_type const max_threat_length = (n-1>max_len_threat
                                              ? max_len_threat
                                              : n-1);
  int const threats = alloctab();
  composite_d_solve_variations_in_n(max_threat_length,threats,si,n);
  freetab();
}

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * @param si slice index
 */
void composite_solve_variations(slice_index si)
{
  switch (slices[si].u.composite.play)
  {
    case PDirect:
      composite_d_solve_variations(si);
      break;

    case PHelp:
      /* TODO */
      break;

    case PSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }
}

/* Solve postkey play of a composite slice at root level.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param si slice index
 */
static void composite_d_root_solve_postkey(int refutations,
                                           slice_index si,
                                           stip_length_type n)
{
  int const threats = alloctab();
  int const len_threat = composite_d_solve_threats(threats,si,n);
  composite_d_root_solve_variations(len_threat,threats,refutations,si,n);
  freetab();
}

/* Solve postkey play of a composite slice.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param si slice index
 */
static void composite_d_solve_postkey(slice_index si, stip_length_type n)
{
  int const threats = alloctab();
  int len_threat;

  output_start_postkey_level();
  len_threat = composite_d_solve_threats(threats,si,n);
  composite_d_solve_variations_in_n(len_threat,threats,si,n);
  freetab();
  output_end_postkey_level();
}

/* Determine and write the continuations in direct/self/reflex play
 * (i.e. attacker's final move and possible play following it).
 * This is an indirectly recursive function.
 * @param t table where to store continuing moves (i.e. threats)
 */
static void composite_end_solve_continuations(int t, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      quodlibet_end_solve_continuations(t,si);
      break;

    case STReciprocal:
      reci_end_solve_continuations(t,si);
      break;

    case STSequence:
      sequence_end_solve_continuations(t,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * This is an indirectly recursive function.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
static void composite_d_solve_continuations_in_n(int continuations,
                                                 slice_index si,
                                                 stip_length_type n)
{
  Side const attacker = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_direct)
    composite_end_solve_continuations(continuations,si);
  else
  {
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !echecc(nbply,attacker))
      {
        d_defender_win_type const
            defender_success = composite_d_does_defender_win(si,n-1);
        TraceValue("%u\n",defender_success);
        if (defender_success>=loss)
        {
          write_attack(no_goal,attack_regular);

          if (defender_success>=short_loss)
            composite_end_solve_variations(si);
          else
            composite_d_solve_postkey(si,n);

          pushtabsol(continuations);
        }
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
static void composite_d_solve_continuations(int continuations,
                                            slice_index si)
{
  stip_length_type const n = slices[si].u.composite.length;
  composite_d_solve_continuations_in_n(continuations,si,n);
}

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
void composite_solve_continuations(int continuations, slice_index si)
{
  switch (slices[si].u.composite.play)
  {
    case PDirect:
      composite_d_solve_continuations(continuations,si);
      break;

    case PHelp:
      /* TODO */
      break;

    case PSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }
}

/* Determine and write set play in direct play
 * @param si slice index
 */
static boolean composite_d_root_solve_setplay(slice_index si)
{
  Side const defender = advers(slices[si].starter);
  stip_length_type const n = slices[si].u.composite.length;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_direct)
    result = composite_root_end_solve_setplay(si);
  else
  {
    int ntcount = 0;

    if (!slice_root_end_solve_complete_set(si))
      StdString("\n");

    if (n-1>min_length_nontrivial)
    {
      ntcount = count_non_trivial(si);
      max_nr_nontrivial -= ntcount;
    }

    genmove(defender);

    while(encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !echecc(nbply,defender))
      {
        if (slice_has_non_starter_solved(si))
          ; /* oops */
        else if (composite_d_has_solution_in_n(si,n-1))
        {
          /* yipee - this solves! */
          composite_d_write_variation(si,n-1);
          result = true;
        }
      }

      repcoup();
    }

    finply();

    if (n-1>min_length_nontrivial)
      max_nr_nontrivial += ntcount;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a help play problem at root level in exactly N half moves
 * (N>=2); decide whether to solve in intelligent mode or not. 
 * @param si slice index
 * @param n number of half moves
 * @param starter starting side
 * @return true iff >= 1 solution was found
 */
static boolean
composite_h_root_exact_solve_intelligent_or_not(slice_index si,
                                                stip_length_type n,
                                                Side starter)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",starter);

  assert(n>=2);

  if (isIntelligentModeActive)
    result = Intelligent(n,starter);
  else
  {
    /* we only display move numbers when looking for full length
     * solutions (incl. full length set play)
     */
    boolean const save_movenbr = OptFlag[movenbr];
    if (n<slices[si].u.composite.length-1)
      OptFlag[movenbr] = false;
    result = composite_h_root_exact_solve(si,n,starter);
    OptFlag[movenbr] = save_movenbr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write set play in help play
 * @param si slice index
 */
static boolean composite_h_root_solve_setplay(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (echecc(nbply,slices[si].starter))
    ErrorMsg(KingCapture);
  else
  {
    Side const starter = advers(slices[si].starter);
    stip_length_type const length = slices[si].u.composite.length-1;

    if (length%2==1)
      result = composite_root_end_solve_setplay(si);

    if (length>1)
    {
      stip_length_type len;
      for (len = slices[si].u.composite.min_length+1;
           !result && len<length;
           len += 2)
        result = composite_h_root_exact_solve_intelligent_or_not(si,
                                                                 len,
                                                                 starter);
      result = (result
                || composite_h_root_exact_solve_intelligent_or_not(si,
                                                                   length,
                                                                   starter));
    }
  }

  Message(NewLine); /* TODO invoke higher-level output function */

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write set play
 * @param si slice index
 */
boolean composite_root_solve_setplay(slice_index si)
{
  boolean result = false;

  output_start_setplay_level();

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      result = composite_d_root_solve_setplay(si);
      break;

    case PHelp:
      result = composite_h_root_solve_setplay(si);
      break;

    case PSeries:
      /* TODO implement composite_ser_root_solve_setplay() */
      break;
  }

  output_end_setplay_level();

  return result;
}

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any).
 * We are at the end of the slice and delegate to the child slice(s).
 * @param si slice index
 * @param type type of attack
 */
static void composite_root_end_write_key_solve_postkey(slice_index si,
                                                       attack_type type)
{
  switch (slices[si].type)
  {
    case STQuodlibet:
      quodlibet_root_end_write_key_solve_postkey(si,type);
      break;

    case STReciprocal:
      reci_root_end_write_key_solve_postkey(si,type);
      break;

    case STSequence:
      sequence_root_end_write_key_solve_postkey(si,type);
      break;

    default:
      assert(0);
      break;
  }
}

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any).
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param type type of attack
 */
static void composite_d_root_write_key_solve_postkey(int refutations,
                                                     slice_index si,
                                                     attack_type type)
{
  write_attack(no_goal,type);

  output_start_postkey_level();
  if (OptFlag[solvariantes])
    composite_d_root_solve_postkey(refutations,
                                   si,
                                   slices[si].u.composite.length);
  write_refutations(refutations);
  output_end_postkey_level();
}

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any).
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param type type of attack
 */
void composite_root_write_key_solve_postkey(int refutations,
                                            slice_index si,
                                            attack_type type)
{
  switch (slices[si].u.composite.play)
  {
    case PDirect:
      composite_d_root_write_key_solve_postkey(refutations,si,type);
      break;

    case PHelp:
      /* TODO */
      break;

    case PSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }
}

/* Solve the postkey play only of a composite slice at root level.
 * @param si slice index
 * @param type type of attack
 */
static void composite_d_root_solve_postkeyonly(slice_index si,
                                               stip_length_type n)
{
  output_start_postkeyonly_level();

  if (n==slack_length_direct)
    composite_end_solve_variations(si);
  else if (n==slices[si].u.composite.min_length)
    composite_d_solve_postkey(si,n);
  else if (slice_end_has_starter_won(si))
    composite_end_solve_variations(si);
  else
    composite_d_solve_postkey(si,n);

  output_end_postkeyonly_level();
}

/* Determine and write the solutions and tries in the current position
 * in direct/self/reflex play.
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean composite_d_root_solve_real_play(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_continuation_level();

  if (slice_has_non_starter_solved(si))
    result = true;
  else if (slice_end_has_non_starter_refuted(si))
    slice_write_unsolvability(slices[si].u.composite.op1);
  else if (slices[si].u.composite.length==slack_length_direct)
    result = composite_root_end_solve(si);
  else
  {
    Side const attacker = slices[si].starter;
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && !echecc(nbply,attacker))
      {
        if (slices[si].u.composite.min_length==slack_length_direct
            && slice_end_has_starter_won(si))
          composite_root_end_write_key_solve_postkey(si,attack_key);
        else
        {
          int refutations = alloctab();
          int const nr_refutations =
              composite_d_find_refutations(refutations,si);
          TraceValue("%u\n",nr_refutations);
          if (nr_refutations<=max_nr_refutations)
          {
            attack_type const type = (tablen(refutations)>=1
                                      ? attack_try
                                      : attack_key);
            composite_root_write_key_solve_postkey(refutations,si,type);

            if (nr_refutations==0)
              result = true;
          }

          freetab();
        }
      }

      if (OptFlag[movenbr])
        IncrementMoveNbr();

      repcoup();

      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    finply();
  }

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Delegate solving the complete set to the child slice(s)
 * @param si parent slice index
 * @return true iff complete set play was found
 */
static boolean composite_root_end_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",slices[si].type);

  switch (slices[si].type)
  {
    case STQuodlibet:
      result = quodlibet_root_end_solve_complete_set(si);
      break;

    case STReciprocal:
      /* TODO */
      break;

    case STSequence:
      result = sequence_root_end_solve_complete_set(si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",si);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice with help play at root level
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean composite_h_root_solve(slice_index si)
{
  boolean result = false;
  Side const starter = slices[si].starter;
  stip_length_type const full_length = slices[si].u.composite.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  init_output_mode(output_mode_line);

  move_generation_mode = move_generation_not_optimized;

  if (OptFlag[solapparent] && !OptFlag[restart])
  {
    output_start_setplay_level();
    if (composite_h_root_solve_setplay(si))
      result = true;
    output_end_setplay_level();
  }

  solutions = 0;    /* reset after set play */
  FlagShortSolsReached= false;

  if (echecc(nbply,advers(starter)))
    ErrorMsg(KingCapture);
  else if (full_length==slack_length_help-1)
    result = composite_root_end_solve_setplay(si);
  else
  {
    boolean full_length_solved = false;
    stip_length_type len = slices[si].u.composite.min_length;

    if (len==slack_length_help-1)
    {
      FlagShortSolsReached = composite_root_end_solve_complete_set(si);
      len +=2;
    }

    while (len<full_length
           && !(OptFlag[stoponshort] && FlagShortSolsReached))
    {
      if (composite_h_root_exact_solve_intelligent_or_not(si,len,starter))
        FlagShortSolsReached = true;
      len += 2;
    }

    if (!(FlagShortSolsReached && OptFlag[stoponshort]))
      full_length_solved =
          composite_h_root_exact_solve_intelligent_or_not(si,
                                                          full_length,
                                                          starter);

    result = FlagShortSolsReached || full_length_solved;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


/* Solve a composite direct slice at root level
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean composite_d_root_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.composite.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  init_output_mode(output_mode_tree);

  if (OptFlag[postkeyplay])
    
  {
    if (echecc(nbply,slices[si].starter))
      ErrorMsg(SetAndCheck);
    else
    {
      composite_d_root_solve_postkeyonly(si,n);
      result = true;
    }
  }
  else
  {
    if (OptFlag[solapparent] && n>slack_length_direct)
    {
      if (echecc(nbply,slices[si].starter))
        ErrorMsg(SetAndCheck);
      else
      {
        output_start_setplay_level();
        composite_d_root_solve_setplay(si);
        output_end_setplay_level();
        Message(NewLine);
      }
    }

    if (echecc(nbply,advers(slices[si].starter)))
      ErrorMsg(KingCapture);
    else
      result = composite_d_root_solve_real_play(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a composite slice at root level
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
/* TODO why do we have to return a value? */
boolean composite_root_solve(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      result = composite_d_root_solve(si);
      break;

    case PHelp:
      result = composite_h_root_solve(si);
      break;

    case PSeries:
      result = composite_ser_root_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
