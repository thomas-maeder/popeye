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
static boolean d_composite_is_in_hash(slice_index si,
                                      HashBuffer *hb,
                                      stip_length_type n,
                                      boolean *hash_val)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",n);

  /* It is more likely that a position has no solution.           */
  /* Therefore let's check for "no solution" first.  TLi */
  (*encode)(hb);
  if (inhash(si,DirNoSucc,n,hb))
  {
    TraceText("inhash(si,DirNoSucc,n,hb)\n");
    assert(!inhash(si,DirSucc,n-1,hb));
    *hash_val = false;
    result = true;
  }
  else if (inhash(si,DirSucc,n-1,hb))
  {
    TraceText("inhash(si,DirSucc,n-1,hb)\n");
    *hash_val = true;
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacking side wins at end of composite slice
 * @param attacker attacking side
 * @param si slice identifier
 * @return truee iff attacker wins
 */
static boolean d_composite_end_does_attacker_win(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",slices[si].type);

  switch (slices[si].type)
  {
    case STQuodlibet:
      result = d_quodlibet_end_does_attacker_win(si);
      break;

    case STReciprocal:
      result = d_reci_end_does_attacker_win(si);
      break;

    case STSequence:
      result = d_sequence_end_does_attacker_win(si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceValue("%d",si);
  TraceFunctionResult("%d\n",result);
  return result;
}

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
    if (jouecoup()
        && !echecc(defender)
        && !(min_length_nontrivial>0
             && d_composite_does_attacker_win(min_length_nontrivial,si)))
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
d_defender_win_type d_composite_middle_does_defender_win(stip_length_type n,
                                                         slice_index si)
{
  Side const attacker = slices[si].starter;
  Side const defender = advers(attacker);
  boolean is_defender_immobile = true;
  boolean refutation_found = false;
  int ntcount = 0;
  d_defender_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  /* check whether `black' can reach a position that is already
  ** marked unsolvable for white in the hash table. */
  /* TODO should we? i.e. do it or remove comment */

  if (n>max_len_threat
	  && !echecc(defender)
	  && !d_composite_does_attacker_win(max_len_threat,si))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",win);
	return win;
  }

  if (OptFlag[solflights] && has_too_many_flights(defender))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",win);
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
      TraceFunctionResult("%d\n",win);
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
    TraceCurrentMove();
	if (jouecoup()
        && !echecc(defender))
	{
	  is_defender_immobile = false;
	  if (!d_composite_does_attacker_win(n,si))
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
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param defender defending side (at move)
 * @param n number of moves until end state has to be reached
 * @return true iff defender wins
 */
static d_defender_win_type d_composite_does_defender_win(stip_length_type n,
                                                         slice_index si)
{
  d_defender_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  assert(n>=slack_length_direct);

  if (d_slice_has_attacker_lost(si))
    result = already_won;
  else if (!slices[si].u.composite.is_exact && d_slice_has_attacker_won(si))
    result = short_loss;
  else
    result = d_composite_middle_does_defender_win(n,si);

  TraceFunctionExit(__func__);
  TraceValue("%d",n);
  TraceValue("%d",si);
  TraceFunctionResult("%d\n",result);
  return result;
} /* d_composite_does_defender_win */

/* Determine whether the attacking side wins in the middle of a
 * composite slice
 * @param attacker attacking side
 * @param si slice identifier
 */
static boolean d_composite_middle_does_attacker_win(stip_length_type n,
                                                    slice_index si)
{
  Side const attacker = slices[si].starter;
  boolean win_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  assert(n>slack_length_direct);

  genmove(attacker);

  while (!win_found && encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !echecc(attacker))
    {
      if (d_composite_does_defender_win(n-1,si)>=loss)
      {
        TraceValue("%d",n);
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
  TraceValue("%d",n);
  TraceFunctionResult("%d\n",win_found);
  return win_found;
}

/* Determine whether attacker can end in n moves of direct play.
 * This is a recursive function.
 * @param n number of moves left until the end state has to be reached
 * @return true iff attacker can end in n moves
 */
boolean d_composite_does_attacker_win(stip_length_type n, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (d_slice_has_defender_won(si))
    ; /* intentionally nothing */
  else if (slices[si].u.composite.is_exact)
  {
    if (n==slack_length_direct)
      result = d_composite_end_does_attacker_win(si);
    else
    {
      HashBuffer hb;
      TraceText("slices[si].u.composite.is_exact\n");
      if (!d_composite_is_in_hash(si,&hb,n,&result))
      {
        TraceText("not in hash\n");
        result = d_composite_middle_does_attacker_win(n,si);
        if (result)
          addtohash(si,DirSucc,n-1,&hb);
        else
          addtohash(si,DirNoSucc,n,&hb);
      }
    }
  }
  else
  {
    if (d_composite_end_does_attacker_win(si))
      result = true;
    else if (d_slice_has_defender_lost(si))
      result = true;
    else if (n>slack_length_direct)
    {
      HashBuffer hb;
      if (!d_composite_is_in_hash(si,&hb,n,&result))
      {
        stip_length_type i;
        for (i = slack_length_direct+1; !result && i<=n; i++)
        {
          if (i-1>max_len_threat || i>min_length_nontrivial)
            i = n;

          result = d_composite_middle_does_attacker_win(i,si);

          if (maxtime_status==MAXTIME_TIMEOUT)
            break;
        }
      }

      if (result)
        addtohash(si,DirSucc,n-1,&hb);
      else
        addtohash(si,DirNoSucc,n,&hb);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionResult("%d\n",result);
  return result;
} /* d_composite_does_attacker_win */

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
static int d_composite_find_refutations(int t, slice_index si)
{
  Side const defender = advers(slices[si].starter);
  boolean is_defender_immobile = true;
  int ntcount = 0;
  int result = 0;
  stip_length_type const n = slices[si].u.composite.length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n>max_len_threat
      && !echecc(defender)
      && !d_composite_does_attacker_win(max_len_threat,si))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",result);
    return max_nr_refutations+1;
  }

  if (n>slack_length_direct+1
      && OptFlag[solflights] && has_too_many_flights(defender))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",result);
    return max_nr_refutations+1;
  }

  if (n>min_length_nontrivial)
  {
    ntcount = count_non_trivial(si);
    if (max_nr_nontrivial<ntcount)
    {
      TraceFunctionExit(__func__);
      TraceFunctionResult("%d\n",result);
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
    TraceCurrentMove();
    if (jouecoup()
        && !echecc(defender))
    {
      is_defender_immobile = false;
      if (!d_composite_does_attacker_win(n,si))
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
  TraceFunctionResult("%d\n",result);
  return result;
} /* d_composite_find_refutations */

void d_composite_end_solve_variations(int len_threat,
                                      int threats,
                                      int refutations,
                                      slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",slices[si].type);

  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_solve_variations(len_threat,threats,refutations,si);
      break;

    case STReciprocal:
      d_reci_end_solve_variations(len_threat,threats,refutations,si);
      break;

    case STSequence:
      d_sequence_end_solve_variations(len_threat,threats,refutations,si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Continue solving at the end of a composite slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
static boolean h_composite_end_solve(boolean restartenabled, slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      result = h_quodlibet_end_solve(si);
      break;

    case STReciprocal:
      result = h_reci_end_solve(si);
      break;

    case STSequence:
      result = h_sequence_end_solve(restartenabled,si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean h_composite_end_is_unsolvable(slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
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
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean h_composite_solve_recursive(Side side_at_move,
                                           stip_length_type n,
                                           boolean restartenabled,
                                           slice_index si);

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
 * @param restartenabled true iff option movenum is activated
 * @param si slice index of slice being solved
 * @return true iff >= 1 solution has been found
 */
static boolean h_composite_solve_recursive_nohash(Side side_at_move,
                                                  stip_length_type n,
                                                  boolean restartenabled,
                                                  slice_index si)

{
  boolean found_solution = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",side_at_move);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  assert(n>=slack_length_help);

  if (n==slack_length_help)
    found_solution = h_composite_end_solve(restartenabled,si);
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
      TraceCurrentMove();
      if (jouecoup()
          && (!isIntelligentModeActive || isGoalReachable())
          && !echecc(side_at_move)
          && !(restartenabled && MoveNbr<RestartNbr)
          && !h_composite_end_is_unsolvable(si)
          && h_composite_solve_recursive(next_side,n-1,false,si))
        found_solution = true;

      if (restartenabled)
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
  TraceFunctionResult("%d\n",found_solution);
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
 * @param restartenabled true iff option movenum is activated
 * @param si slice index of slice being solved
 * @return true iff >= 1 solution has been found
 */
static boolean h_composite_solve_recursive(Side side_at_move,
                                           stip_length_type n,
                                           boolean restartenabled,
                                           slice_index si)
{
  boolean found_solution = false;
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;
  HashBuffer hb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",side_at_move);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  (*encode)(&hb);
  if (!inhash(si,hash_no_succ,n/2,&hb))
  {
    if (h_composite_solve_recursive_nohash(side_at_move,n,restartenabled,si))
      found_solution = true;
    else
      addtohash(si,hash_no_succ,n/2,&hb);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
}

/* Determine and write the solution(s) in a help stipulation.
 * @param restartenabled true iff option movenum is activated
 * @param si identifies slice being solved
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean h_composite_solve(boolean restartenabled,
                          slice_index si,
                          stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",restartenabled);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",n);

  if (n==slices[si].u.composite.length)
    result = h_composite_solve_recursive_nohash(slices[si].starter,
                                                n,
                                                restartenabled,
                                                si);
  else
    result = h_composite_solve_recursive(slices[si].starter,
                                         n,
                                         restartenabled,
                                         si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean ser_composite_end_solve(boolean restartenabled,
                                       slice_index si)
{
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STSequence:
      solution_found = ser_sequence_end_solve(restartenabled,si);
      break;

    case STQuodlibet:
      solution_found = ser_quodlibet_end_solve(restartenabled,si);
      break;

    case STReciprocal:
      solution_found = ser_reci_end_solve(restartenabled,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Solve a composite slice with series play
 * @param n exact number of moves to reach the end state
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean ser_composite_exact_solve_recursive(stip_length_type n,
                                                   boolean restartenabled,
                                                   slice_index si)
{
  boolean solution_found = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n==slack_length_series)
    solution_found = ser_composite_end_solve(restartenabled,si);
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
        TraceCurrentMove();
        if (!jouecoup())
          TraceText("!jouecoup()\n");
        else if (echecc(series_side))
          TraceText("echecc(series_side)\n");
        else if (restartenabled && MoveNbr<RestartNbr)
          TraceText("restartenabled && MoveNbr<RestartNbr\n");
        else if (isIntelligentModeActive && !isGoalReachable())
          TraceText("isIntelligentModeActive && !isGoalReachable()\n");
        else if (echecc(other_side))
          TraceText("echecc(other_side)\n");
        else
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (inhash(si,SerNoSucc,n-1,&hb))
            TraceText("in hash\n");
          else if (ser_composite_exact_solve_recursive(n-1,false,si))
            solution_found = true;
          else
            addtohash(si,SerNoSucc,n-1,&hb);
        }

        if (restartenabled)
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
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
} /* ser_composite_exact_solve */

/* Solve a composite slice with series play
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_exact_solve(boolean restartenabled,
                                  slice_index si,
                                  stip_length_type n)
{
  return ser_composite_exact_solve_recursive(n,restartenabled,si);
}

/* Solve a composite slice with series play
 * @param n maximal number of moves to reach the end state
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean ser_composite_maximal_solve(stip_length_type n,
                                           boolean restartenabled,
                                           slice_index si)
{
  boolean solution_found;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  solution_found = ser_composite_end_solve(n==1 && restartenabled, si);

  if (n>slack_length_series)
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
        TraceCurrentMove();
        if (!jouecoup())
          TraceText("!jouecoup()\n");
        else if (echecc(series_side))
          TraceText("echecc(series_side)\n");
        else if (restartenabled && MoveNbr<RestartNbr)
          TraceText("restartenabled && MoveNbr<RestartNbr\n");
        else if (isIntelligentModeActive && !isGoalReachable())
          TraceText("isIntelligentModeActive && !isGoalReachable()\n");
        else if (echecc(other_side))
          TraceText("echecc(other_side)\n");
        else
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (inhash(si,SerNoSucc,n-1,&hb))
            TraceText("in hash\n");
          else if (ser_composite_maximal_solve(n-1,false,si))
            solution_found = true;
          else
            addtohash(si,SerNoSucc,n-1,&hb);
        }

        if (restartenabled)
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
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
} /* ser_composite_maximal_solve */

/* Solve a composite slice with series play
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_solve(boolean restartenabled,
                            slice_index si,
                            stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",n);

  if (slices[si].u.composite.is_exact)
    result = ser_composite_exact_solve(restartenabled,si,n);
  else
  {
    if (!slices[si].u.composite.is_exact)
    {
      stip_length_type i;
      for (i = 1; i<n; i++)
        if (ser_composite_exact_solve_recursive(i,false,si))
        {
          TraceText("solution found\n");
          result = true;
          if (OptFlag[stoponshort])
          {
            FlagShortSolsReached = true;
            break;
          }
        }
    }

    if (!FlagShortSolsReached
        && ser_composite_exact_solve(restartenabled,si,n))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Solve the composite slice with index 0 with series play
 * @param n number of moves until the slice's goal has to be reached
 *          (this may be shorter than the slice's length if we are
 *          searching for short solutions only)
 * @param restartenabled true iff option movenum is active
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_slice0_solve(stip_length_type n,
                                   boolean restartenabled)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",restartenabled);

  if (slices[0].u.composite.is_exact)
    result = ser_composite_exact_solve(restartenabled,0,n);
  else if (OptFlag[restart])
    result = ser_composite_maximal_solve(n,restartenabled,0);
  else
    result = ser_composite_solve(restartenabled,0,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
static boolean d_composite_end_is_threat_refuted(slice_index si)
{
  boolean result = true;

  switch (slices[si].type)
  {
    case STQuodlibet:
      result = d_quodlibet_end_is_threat_refuted(si);
      break;

    case STReciprocal:
      result = d_reci_end_is_threat_refuted(si);
      break;

    case STSequence:
      result = d_sequence_end_is_threat_refuted(si);
      break;

    default:
      assert(0);
  }

  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param n number of moves until end state has to be reached
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
static boolean d_composite_middle_is_threat_refuted(stip_length_type n,
                                                    slice_index si)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n==slack_length_direct)
    result = d_composite_end_is_threat_refuted(si);
  else
    result = d_composite_does_defender_win(n-1,si)<=win;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_composite_is_threat_refuted(slice_index si)
{
  return d_composite_middle_is_threat_refuted(slices[si].u.composite.length,
                                              si);
}

/* Determine whether the move just played by the defending side
 * defends against the threats.
 * @param n number of moves until end state has to be reached from now
 * @param threats table containing the threats
 * @return true iff the move just played defends against at least one
 *         of the threats
 */
static boolean d_composite_defends_against_threats(stip_length_type n,
                                                   int threats,
                                                   slice_index si)
{
  Side const attacker = slices[si].starter;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",tablen(threats));
  if (tablen(threats)>0)
  {
    int nr_successful_threats = 0;
    boolean defense_found = false;

    genmove(attacker);

    while (encore() && !defense_found)
    {
      TraceCurrentMove();
      if (jouecoup()
          && nowdanstab(threats)
          && !echecc(attacker))
      {
        TraceText("checking threat\n");
        defense_found = d_composite_middle_is_threat_refuted(n,si);
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
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Write a variation in the try/solution/set play of a
 * direct/self/reflex stipulation. The move of the defending side that
 * starts the variation has already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached from now
 */
static void d_composite_write_variation(stip_length_type n, slice_index si)
{
  boolean isRefutation = true; /* until we prove otherwise */
  stip_length_type i;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  write_defense(no_goal);
  marge+= 4;

  zugebene++;

  for (i = slices[si].u.composite.is_exact ? n : 1;
       i<=n && isRefutation;
       i++)
  {
    int const continuations = alloctab();
    d_composite_solve_continuations(i,continuations,si);
    isRefutation = tablen(continuations)==0;
    freetab();
  }

  if (isRefutation)
  {
    marge+= 2;
    Tabulate();
    Message(Refutation);
    marge-= 2;
  }

  zugebene--;

  marge-= 4;

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
 * @return the length of the shortest threat(s); 1 if there is no threat
 */
static int d_composite_middle_solve_threats(stip_length_type n,
                                            int threats,
                                            slice_index si)
{
  Side const defender = advers(slices[si].starter);
  int result = 1;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (OptFlag[nothreat] || echecc(defender))
    write_attack_conclusion(attack_without_zugzwang);
  else
  {
    stip_length_type max_threat_length = (n-1>max_len_threat
                                          ? max_len_threat
                                          : n-1);
    stip_length_type i;

    DrohFlag = true;

    marge+= 4;

    zugebene++;

    for (i = 1; i<=max_threat_length; i++)
    {
      d_composite_solve_continuations(i,threats,si);
      if (tablen(threats)>0)
      {
        result = i;
        break;
      }
    }

    zugebene--;

    marge-= 4;

    if (DrohFlag)
    {
      write_attack_conclusion(attack_with_zugzwang);
      DrohFlag = false;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
} /* d_composite_middle_solve_threats */

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
void d_composite_solve_variations(stip_length_type n,
                                  int len_threat,
                                  int threats,
                                  int refutations,
                                  slice_index si)
{
  Side const attacker = slices[si].starter;
  Side defender = advers(attacker);
  int ntcount = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d",len_threat);
  TraceFunctionParam("%d\n",si);

  if (n-1>min_length_nontrivial)
  {
    ntcount = count_non_trivial(si);
    max_nr_nontrivial -= ntcount;
  }

  genmove(defender);

  while(encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !echecc(defender)
        && !nowdanstab(refutations))
    {
      if (n>2 && OptFlag[noshort]
          && d_composite_does_attacker_win(n-2,si))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (len_threat>1
               && d_composite_does_attacker_win(len_threat-1,si))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (d_slice_has_defender_lost(si))
        ; /* oops! short end. NB: this can't happen if is_exact and
           * n is too large, because that would make the current move
           * a refutation */
      else if (!d_composite_defends_against_threats(len_threat, threats,si))
        ; /* move doesn't defend against threat */
      else
        d_composite_write_variation(n-1,si);
    }
    repcoup();
  }

  finply();

  if (n-1>min_length_nontrivial)
    max_nr_nontrivial += ntcount;
  
  TraceFunctionExit(__func__);
} /* d_composite_solve_variations */

static void d_composite_middle_solve_postkey(stip_length_type n,
                                             int refutations,
                                             slice_index si)
{
  int threats = alloctab();
  int len_threat = d_composite_middle_solve_threats(n,threats,si);
  d_composite_solve_variations(n,len_threat,threats,refutations,si);
  freetab();
}

/* Determine and write the end in direct/self/reflex play
 * (i.e. attacker's final move and possible play following it).
 * This is an indirectly recursive function.
 * @param t table where to store continuing moves (i.e. threats)
 */
static void d_composite_end_solve_continuations(int t, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_solve_continuations(t,si);
      break;

    case STReciprocal:
      d_reci_end_solve_continuations(t,si);
      break;

    case STSequence:
      d_sequence_end_solve_continuations(t,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the continuations in the current position in
 * direct/self/reflex play (i.e. attacker's moves winning after a
 * defender's move that refuted the threat).
 * This is an indirectly recursive function.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param continuations table where to store continuing moves (i.e. threats)
 */
void d_composite_solve_continuations(stip_length_type n,
                                     int continuations,
                                     slice_index si)
{
  Side const attacker = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n==slack_length_direct)
    d_composite_end_solve_continuations(continuations,si);
  else
  {
    genmove(attacker);

    while (encore())
    {
      TraceCurrentMove();
      if (jouecoup()
          && !echecc(attacker))
      {
        d_defender_win_type const defender_success =
            d_composite_does_defender_win(n-1,si);
        TraceValue("%d\n",defender_success);
        if (defender_success>=loss)
        {
          write_attack(no_goal,attack_regular);

          marge+= 4;
          if (!slices[si].u.composite.is_exact
              && defender_success>=short_loss)
          {
            d_composite_end_solve_variations(0,alloctab(),alloctab(),si);
            freetab();
            freetab();
          }
          else
          {
            d_composite_middle_solve_postkey(n,alloctab(),si);
            freetab();
          }
          marge-= 4;

          pushtabsol(continuations);
        }
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
} /* d_composite_solve_continuations */

static void d_composite_end_solve_setplay(slice_index si)
{
  switch (slices[si].type)
  {
    case STSequence:
      d_sequence_end_solve_setplay(si);
      break;

    case STReciprocal:
      d_reci_end_solve_setplay(si);
      break;

    case STQuodlibet:
      d_quodlibet_end_solve_setplay(si);
      break;

    default:
      assert(0);
      break;
  }
}

/* Determine and write set play of a direct/self/reflex stipulation
 * @param si slice index
 */
void d_composite_solve_setplay(slice_index si)
{
  Side const defender = advers(slices[si].starter);
  stip_length_type const n = slices[si].u.composite.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  if (n==slack_length_direct)
    d_composite_end_solve_setplay(si);
  else
  {
    int ntcount = 0;

    if (!d_slice_solve_complete_set(si))
      StdString("\n");

    if (n-1>min_length_nontrivial)
    {
      ntcount = count_non_trivial(si);
      max_nr_nontrivial -= ntcount;
    }

    genmove(defender);

    while(encore())
    {
      TraceCurrentMove();
      if (jouecoup()
          && !echecc(defender))
      {
        if (d_slice_has_defender_lost(si))
          ; /* oops */
        else if (d_composite_does_attacker_win(n-1,si))
          /* yipee - this solves! */
          d_composite_write_variation(n-1,si);
      }

      repcoup();
    }

    finply();

    if (n-1>min_length_nontrivial)
      max_nr_nontrivial += ntcount;
  }

  TraceFunctionExit(__func__);
}

static void d_composite_end_solve(boolean restartenabled, slice_index si)
{
  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_solve(restartenabled,si);
      break;

    case STReciprocal:
      d_reci_end_solve(restartenabled,si);
      break;

    case STSequence:
      d_sequence_end_solve(restartenabled,si);
      break;

    default:
      assert(0);
      break;
  }
}

static void d_composite_end_write_key_solve_postkey(int refutations,
                                                    slice_index si,
                                                    attack_type type)
{
  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_write_key_solve_postkey(refutations,si,type);
      break;

    case STReciprocal:
      d_reci_end_write_key_solve_postkey(refutations,si,type);
      break;

    case STSequence:
      d_sequence_end_write_key_solve_postkey(refutations,si,type);
      break;

    default:
      assert(0);
      break;
  }
}

void d_composite_write_key_solve_postkey(int refutations,
                                         slice_index si,
                                         attack_type type)
{
  write_attack(no_goal,type);

  marge+= 4;

  if (OptFlag[solvariantes])
    d_composite_middle_solve_postkey(slices[si].u.composite.length,
                                     refutations,
                                     si);
  else
    write_attack_conclusion(attack_without_zugzwang);

  write_refutations(refutations);

  marge-= 4;
}

void d_composite_solve_postkey(stip_length_type n, slice_index si)
{
  if (n==slack_length_direct)
  {
    d_composite_end_solve_variations(0,alloctab(),alloctab(),si);
    freetab();
    freetab();
  }
  else if (slices[si].u.composite.is_exact)
  {
    d_composite_middle_solve_postkey(n,alloctab(),si);
    freetab();
  }
  else if (d_slice_has_attacker_won(si))
  {
    d_composite_end_solve_variations(0,alloctab(),alloctab(),si);
    freetab();
    freetab();
  }
  else
  {
    d_composite_middle_solve_postkey(n,alloctab(),si);
    freetab();
  }
}

/* Determine and write tries and solutios in a "regular"
 * direct/self/reflex stipulation.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @return true iff >=1 solution was found and written
 */
static void d_composite_middle_solve(boolean restartenabled, slice_index si)
{
  Side const attacker = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  genmove(attacker);

  while (encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !(restartenabled && MoveNbr<RestartNbr)
        && !echecc(attacker))
    {
      if (!slices[si].u.composite.is_exact
          && d_slice_has_attacker_won(si))
      {
        int refutations = alloctab();
        d_composite_end_write_key_solve_postkey(refutations,
                                                si,
                                                attack_key);
        freetab();
      }
      else
      {
        int refutations = alloctab();
        int const nr_refutations = d_composite_find_refutations(refutations,
                                                                si);
        TraceValue("%d\n",nr_refutations);
        if (nr_refutations<=max_nr_refutations)
        {
          attack_type const type = (tablen(refutations)>=1
                                    ? attack_try
                                    : attack_key);
          d_composite_write_key_solve_postkey(refutations,si,type);
        }

        freetab();
      }
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  TraceFunctionExit(__func__);
}

/* Determine and write the solutions and tries in the current position
 * in direct/self/reflex play.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 */
void d_composite_solve(boolean restartenabled, slice_index si)
{
  if (d_slice_has_defender_lost(si))
    ;
  else if (d_slice_has_defender_won(si))
    d_slice_write_unsolvability(slices[si].u.composite.op1);
  else if (slices[si].u.composite.length==1)
    d_composite_end_solve(restartenabled,si);
  else
    d_composite_middle_solve(restartenabled,si);
} /* d_composite_solve */
