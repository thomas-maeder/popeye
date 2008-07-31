#include "pycompos.h"
#include "pyleaf.h"
#include "pyquodli.h"
#include "pysequen.h"
#include "pyrecipr.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyint.h"
#include "pymsg.h"
#include "pyio.h"
#include "trace.h"
#include "platform/maxtime.h"

#include <assert.h>

extern boolean hashing_suspended; /* TODO */

/* Determine whether the current position is stored in the hash table
 * and how.
 * @param hb address of HashBuffer to hold the encoded current position
 * @param n number of moves until goal
 * @param result address where to write hash result (if any)
 * @return true iff the current position was found and *result was
 *              assigned
 */
static boolean d_composite_is_in_hash(HashBuffer *hb, int n, boolean *result)
{
  /* It is more likely that a position has no solution.           */
  /* Therefore let's check for "no solution" first.  TLi */
  (*encode)(hb);
  if (inhash(WhDirNoSucc,n,hb))
  {
    assert(hashing_suspended || !inhash(WhDirSucc,n,hb));
    *result = false;
    return true;
  } else if (inhash(WhDirSucc,n,hb))
  {
    *result = true;
    return true;
  }
  else
    return false;
}

/* Determine whether the defending side wins at the end of a sequence
 * in direct play.
 * @param defender defending side
 * @param si slice identifier
 * @return true iff defender wins
 */
static
d_composite_win_type d_composite_end_does_defender_win(Side defender,
                                                       slice_index si)
{
  d_composite_win_type result = win;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",defender);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STQuodlibet:
      result = d_quodlibet_end_does_defender_win(defender,si);
      break;

    case STSequence:
      result = d_sequence_end_does_defender_win(defender,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker has lost with his last move in
 * direct play. 
 * Assumes that he has not won with his last move.
 * @param defender defending side
 * @param si slice identifier
 * @return true iff attacker has lost
 */
boolean d_composite_has_attacker_lost(Side defender, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",defender);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[op1].type);
  switch (slices[op1].type)
  {
    case STLeaf:
      result = d_leaf_has_attacker_lost(defender,op1);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_attacker_lost(defender,op1);
      break;

    default:
      assert(0);
      break;
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
static boolean d_composite_end_does_attacker_win(Side attacker,
                                                 slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      result = d_quodlibet_end_does_attacker_win(attacker,si);
      break;

    case STSequence:
      result = d_sequence_end_does_attacker_win(attacker,si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceValue("%d",si);
  TraceFunctionResult("%d\n",result);
  return result;
}

boolean d_composite_does_attacker_win(Side attacker,
                                      int n,
                                      slice_index si);

/* Count all non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * @param defender defending side (i.e.side for which to count
 *                 non-trivial moves)
 * @return number of defender's non-trivial moves minus 1 (TODO: why?)
 */
static int count_non_trivial(Side defender, slice_index si)
{
  Side attacker = advers(defender);
  int result = -1;

  genmove(defender);

  while (encore() && max_nr_nontrivial>=result)
  {
    if (jouecoup()
        && !echecc(defender)
        && !(min_length_nontrivial>0
             && d_composite_does_attacker_win(attacker,
                                              min_length_nontrivial,
                                              si)))
      ++result;
    repcoup();
  }

  finply();

  return result;
}

/* Determine whether the defending side wins in n (>1) in direct play.
 * @param defender defending side
 * @param si slice identifier
 * @return true iff defender wins
 */
static
d_composite_win_type d_composite_middle_does_defender_win(Side defender,
                                                          int n,
                                                          slice_index si)
{
  Side const attacker = advers(defender);
  boolean is_defender_immobile = true;
  boolean refutation_found = false;
  int ntcount = 0;
  d_composite_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  /* check whether `black' can reach a position that is already
  ** marked unsolvable for white in the hash table. */
  /* TODO should we? i.e. do it or remove comment */

  if (n>max_len_threat
	  && !echecc(defender)
	  && !d_composite_does_attacker_win(attacker,max_len_threat,si))
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
	ntcount = count_non_trivial(defender,si);
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
	  if (!d_composite_does_attacker_win(attacker,n,si))
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

/* Determine whether the attacker has won the composite slice with his
 * last move in direct play. 
 * @param defender defending side
 * @param si slice identifier
 * @return true iff attacker has won
 */
boolean d_composite_has_attacker_won(Side defender, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",defender);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",slices[op1].type);

  switch (slices[op1].type)
  {
    case STLeaf:
      result = d_leaf_has_attacker_won(defender,op1);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_attacker_won(defender,op1);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender wins in a self/reflex stipulation in
 * n.
 * @param defender defending side (at move)
 * @param n number of moves until end state has to be reached
 * @return true iff defender wins
 */
d_composite_win_type d_composite_does_defender_win(Side defender,
                                                   int n,
                                                   slice_index si)
{
  d_composite_win_type result;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d",defender);
  TraceFunctionParam("%d\n",si);
  if (d_composite_has_attacker_lost(defender,si))
    result = already_won;
  else if (n==0)
  {
    if (d_composite_has_attacker_won(defender,si))
      result = short_loss;
    else
      result = d_composite_end_does_defender_win(defender,si);
  }
  else if (slices[si].u.composite.is_exact)
    result = d_composite_middle_does_defender_win(defender,n,si);
  else if (d_composite_has_attacker_won(defender,si))
    result = short_loss;
  else
    result = d_composite_middle_does_defender_win(defender,n,si);

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
static boolean d_composite_middle_does_attacker_win(Side attacker,
                                                    int n,
                                                    slice_index si)
{
  Side const defender = advers(attacker);
  boolean win_found = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  genmove(attacker);

  while (!win_found && encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !echecc(attacker))
    {
      if (d_composite_does_defender_win(defender,n-1,si)>=loss)
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

/* Determine whether the defender has won the direct play sequence
 * with his move just played.
 * @param attacker attacking side
 * @param si slice identifier
 * @return whether there is a short win or loss
 */
boolean d_composite_has_defender_won(Side attacker, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",attacker);
  TraceFunctionParam("%d\n",si);

  switch (slices[op1].type)
  {
    case STLeaf:
      result = d_leaf_is_unsolvable(attacker,op1);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_defender_won(attacker,op1);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has lost in direct play with his move
 * just played.
 * Assumes that there is no short win for the defending side.
 * @param attacker attacking side
 * @param si slice identifier
 * @return whether there is a short win or loss
 */
boolean d_composite_has_defender_lost(Side attacker, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",attacker);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      result = d_leaf_has_defender_lost(attacker,si);
      break;

    case STSequence:
      result = d_sequence_end_has_defender_lost(attacker,si);
      break;

    case STQuodlibet:
      result = d_quodlibet_end_has_defender_lost(attacker,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether attacker can end in n moves of direct play.
 * This is a recursive function.
 * @param attacker attacking side (i.e. side attempting to reach the
 * end)
 * @param n number of moves left until the end state has to be reached
 * @return true iff attacker can end in n moves
 */
boolean d_composite_does_attacker_win(Side attacker,
                                      int n,
                                      slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",attacker);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (d_composite_has_defender_won(attacker,si))
    ; /* intentionally nothing */
  else if (n==1)
  {
    TraceText("n==1\n");
    if (d_composite_has_defender_lost(attacker,si))
      result = true;
    else
      result = d_composite_end_does_attacker_win(attacker,si);
  }
  else if (slices[si].u.composite.is_exact)
  {
    HashBuffer hb;
    TraceText("slices[si].u.composite.is_exact\n");
    if (!d_composite_is_in_hash(&hb,n,&result))
    {
      TraceText("not in hash\n");
      result = d_composite_middle_does_attacker_win(attacker,n,si);
      addtohash(result ? WhDirSucc : WhDirNoSucc, n, &hb);
    }
  }
  else if (d_composite_has_defender_lost(attacker,si))
    result = true;
  else
  {
    HashBuffer hb;
    if (!d_composite_is_in_hash(&hb,n,&result))
    {
      TraceText("not in hash\n");
      if (d_composite_end_does_attacker_win(attacker,si))
        result = true;
      else
      {
        int i;
        for (i = 2; !result && i<=n; i++)
        {
          if (i-1>max_len_threat || i>min_length_nontrivial)
            i = n;

          result = d_composite_middle_does_attacker_win(attacker,i,si);

          if (maxtime_status==MAXTIME_TIMEOUT)
            break;
        }
      }

      addtohash(result ? WhDirSucc : WhDirNoSucc, n, &hb);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionResult("%d\n",result);
  return result;
} /* d_composite_does_attacker_win */

/* Find refutations after a move of the attacking side.
 * @param defender defending side
 * @param n number of moves until end state has to be reached,
 *          not including the move just played
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
static int d_composite_find_refutations(Side defender,
                                        int n,
                                        int t,
                                        slice_index si)
{
  Side attacker = advers(defender);
  boolean is_defender_immobile = true;
  int ntcount = 0;
  int result = 0;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n>max_len_threat
      && !echecc(defender)
      && !d_composite_does_attacker_win(attacker,max_len_threat,si))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",result);
    return max_nr_refutations+1;
  }

  if (n>2 && OptFlag[solflights] && has_too_many_flights(defender))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%d\n",result);
    return max_nr_refutations+1;
  }

  if (n>min_length_nontrivial)
  {
    ntcount = count_non_trivial(defender,si);
    if (max_nr_nontrivial<ntcount)
    {
      TraceFunctionExit(__func__);
      TraceFunctionResult("%d\n",result);
      return max_nr_refutations+1;
    }
    else
      max_nr_nontrivial -= ntcount;
  }

  if (n>2)
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
      if (!d_composite_does_attacker_win(attacker,n,si))
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

void d_composite_end_solve_variations(Side attacker,
                                      int len_threat,
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
      d_quodlibet_end_solve_variations(attacker,
                                       len_threat,
                                       threats,
                                       refutations,
                                       si);
      break;

    case STSequence:
      d_sequence_end_solve_variations(attacker,
                                      len_threat,
                                      threats,
                                      refutations,
                                      si);
      break;

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Continue solving at the end of a composite slice
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
static boolean h_composite_end_solve(Side side_at_move,
                                     hashwhat no_succ_hash_category,
                                     boolean restartenabled,
                                     slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",side_at_move);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STReciprocal:
      result = h_reci_end_solve(side_at_move,si);
      break;

    case STSequence:
      result = h_sequence_end_solve(side_at_move,
                                    no_succ_hash_category,
                                    restartenabled,
                                    si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine and write the solution(s) in a help stipulation.
 *
 * This is a recursive function.
 * Recursion works with decreasing parameter n; recursion stops at
 * n==2 (e.g. h#1). For solving help play problems in 0.5, call
 * h_leaf_h_solve_ending_move() on the leaf instead.
 *
 * @param side_at_move side at the move
 * @param n number of half moves until end state has to be reached
 * @param restartenabled true iff option movenum is activated
 */
boolean h_composite_solve(Side side_at_move,
                          int n,
                          boolean restartenabled,
                          slice_index si)
{
  boolean found_solution = false;
  hashwhat next_no_succ = side_at_move==blanc ? BlHelpNoSucc : WhHelpNoSucc;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",side_at_move);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  assert(n>=2);

  if (OptFlag[keepmating] && !is_a_mating_piece_left(maincamp))
    TraceText("!is_a_mating_piece_left(maincamp)");
  else if (n==2)
    found_solution = h_composite_end_solve(side_at_move,
                                           next_no_succ,
                                           restartenabled,
                                           si);
  else
  {
    Side next_side = advers(side_at_move);

    genmove(side_at_move);
    active_slice[nbply] = si;
  
    if (side_at_move==noir)
      BlMovesLeft--;
    else
      WhMovesLeft--;

    while (encore())
    {
      TraceCurrentMove();
      if (jouecoup()
          && (!OptFlag[intelligent] || MatePossible())
          && !echecc(side_at_move)
          && !(restartenabled && MoveNbr<RestartNbr))
      {
        if (flag_hashall)
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (!inhash(next_no_succ,n-1,&hb))
          {
            if (h_composite_solve(next_side,n-1,False,si))
              found_solution = true;
            else
              addtohash(next_no_succ,n-1,&hb);
          }
        } else
          if (h_composite_solve(next_side,n-1,False,si))
            found_solution = true;
      }

      if (restartenabled)
        IncrementMoveNbr();

      repcoup();

      /* Stop solving if a given number of solutions was encountered */
      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || maxtime_status==MAXTIME_TIMEOUT)
        break;
    }
    
    if (side_at_move==noir)
      BlMovesLeft++;
    else
      WhMovesLeft++;

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
} /* h_composite_solve */

static boolean ser_composite_end_is_unsolvable(Side series_side,
                                               slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",series_side);
  TraceFunctionParam("%d\n",si);

  switch (slices[op1].type)
  {
    case STLeaf:
      /* TODO this doesn't look right */
      if (d_leaf_is_unsolvable(series_side,op1))
      {
        TraceText("unsolvable\n");
        result = true;
      }
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean ser_composite_end_solve(Side series_side,
                                       boolean restartenabled,
                                       slice_index si)
{
  boolean solution_found = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",series_side);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STSequence:
      solution_found = ser_sequence_end_solve(series_side,restartenabled,si);
      break;

    case STReciprocal:
      solution_found = h_reci_end_solve(series_side,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Solve a composite clide with series play
 * @param series_side side doing the series
 * @param n exact number of moves to reach the end state
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_exact_solve(Side series_side,
                                  int n,
                                  boolean restartenabled,
                                  slice_index si)
{
  boolean solution_found = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",series_side);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n==1)
    solution_found = ser_composite_end_solve(series_side,restartenabled,si);
  else
  {
    Side other_side = advers(series_side);

    if (!ser_composite_end_is_unsolvable(series_side,si))
    {
      genmove(series_side);
      active_slice[nbply] = si;

      if (series_side==blanc)
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
        else if (OptFlag[intelligent] && !MatePossible())
          TraceText("OptFlag[intelligent] && !MatePossible()\n");
        else if (echecc(other_side))
          TraceText("echecc(other_side)\n");
        else
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (inhash(SerNoSucc,n,&hb))
            TraceText("in hash\n");
          else if (ser_composite_exact_solve(series_side,n-1,False,si))
            solution_found = true;
          else
            addtohash(SerNoSucc,n,&hb);
        }

        if (restartenabled)
          IncrementMoveNbr();

        repcoup();

        if ((OptFlag[maxsols] && solutions>=maxsolutions)
            || maxtime_status==MAXTIME_TIMEOUT)
          break;
      }

      if (series_side==blanc)
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

/* Solve a composite clide with series play
 * @param series_side side doing the series
 * @param n maximal number of moves to reach the end state
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
static boolean ser_composite_maximal_solve(Side series_side,
                                           int n,
                                           boolean restartenabled,
                                           slice_index si)
{
  boolean solution_found;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",series_side);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  solution_found = ser_composite_end_solve(series_side,restartenabled,si);

  if (n>1)
  {
    Side other_side = advers(series_side);

    if (!ser_composite_end_is_unsolvable(series_side,si))
    {
      genmove(series_side);
      active_slice[nbply] = si;

      if (series_side==blanc)
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
        else if (OptFlag[intelligent] && !MatePossible())
          TraceText("OptFlag[intelligent] && !MatePossible()\n");
        else if (echecc(other_side))
          TraceText("echecc(other_side)\n");
        else
        {
          HashBuffer hb;
          (*encode)(&hb);
          if (inhash(SerNoSucc,n,&hb))
            TraceText("in hash\n");
          else if (ser_composite_maximal_solve(series_side,n-1,False,si))
            solution_found = true;
          else
            addtohash(SerNoSucc,n,&hb);
        }

        if (restartenabled)
          IncrementMoveNbr();

        repcoup();

        if ((OptFlag[maxsols] && solutions>=maxsolutions)
            || maxtime_status==MAXTIME_TIMEOUT)
          break;
      }

      if (series_side==blanc)
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

/* Solve a composite clide with series play
 * @param series_side side doing the series
 * @param restartenabled true iff option movenum is active
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_solve(Side series_side,
                            boolean restartenabled,
                            slice_index si)
{
  boolean result = false;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",series_side);
  TraceFunctionParam("%d\n",si);

  if (slices[si].u.composite.is_exact)
    result = ser_composite_exact_solve(series_side,
                                       slices[si].u.composite.length,
                                       restartenabled,
                                       si);
  else
  {
    int i;
    TraceFunctionParam("%d\n",slices[si].u.composite.length);
    for (i = 1; i<slices[si].u.composite.length; i++)
      if (ser_composite_exact_solve(series_side,i,false,si))
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
        && ser_composite_exact_solve(series_side,
                                     slices[si].u.composite.length,
                                     restartenabled,
                                     si))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Solve the composite slice with index 0 with series play
 * @param series_side side doing the series
 * @param restartenabled true iff option movenum is active
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_slice0_solve(Side series_side,
                                   int n,
                                   boolean restartenabled)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",series_side);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",restartenabled);

  if (OptFlag[restart])
    result = ser_composite_maximal_solve(series_side,n,restartenabled,0);
  else
    result = ser_composite_solve(series_side,restartenabled,0);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the move just played by the defending side
 * defends against the threats.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached from now
 * @param threats table containing the threats
 * @return true iff the move just played defends against at least one
 *         of the threats
 */
static boolean d_composite_defends_against_threats(Side attacker,
                                                   int n,
                                                   int threats,
                                                   slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",tablen(threats));
  if (tablen(threats)>0)
  {
    int zaehler = 0;
    boolean defense_found = false;
    Side defender = advers(attacker);

    genmove(attacker);

    while (encore() && !defense_found)
    {
      TraceCurrentMove();
      if (jouecoup()
          && nowdanstab(threats)
          && !echecc(attacker))
      {
        TraceText("checking threat\n");
        defense_found = d_composite_does_defender_win(defender,n-1,si)<=win;
        if (defense_found)
        {
          TraceText("defended\n");
          coupfort();
        }
        else
          zaehler++;
      }

      repcoup();
    }

    finply();

    /* this happens if we have found a defense or some threats can no
     * longer be played after defender's defense. */
    result = zaehler<tablen(threats);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

void d_composite_solve_continuations(Side attacker,
                                     int n,
                                     int t,
                                     slice_index si);

/* Write a variation in the try/solution/set play of a
 * direct/self/reflex stipulation. The move of the defending side that
 * starts the variation has already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * This is an indirectly recursive function.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached from now
 */
static void d_composite_write_variation(Side attacker, int n, slice_index si)
{
  boolean isRefutation = true; /* until we prove otherwise */
  int i;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  d_write_defense(no_goal);
  marge+= 4;

  zugebene++;

  for (i = slices[si].u.composite.is_exact ? n : 1;
       i<=n && isRefutation;
       i++)
  {
    int const mats = alloctab();
    d_composite_solve_continuations(attacker,i,mats,si);
    isRefutation = tablen(mats)==0;
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
}

/* Determine and write the threats in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param attacker attacking side (i.e. side that has just played)
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @return the length of the shortest threat(s); 1 if there is no threat
 */
static int d_composite_middle_solve_threats(Side attacker,
                                            int n,
                                            int threats,
                                            slice_index si)
{
  Side defender = advers(attacker);
  int result = 1;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (OptFlag[nothreat] || echecc(defender))
    StdString("\n");
  else
  {
    /* TODO exact? */
    int max_threat_length = n-1>max_len_threat ? max_len_threat : n-1;
    int i;

    DrohFlag = true;

    marge+= 4;

    zugebene++;

    for (i = 1; i<=max_threat_length; i++)
    {
      d_composite_solve_continuations(attacker,i,threats,si);
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
      Message(Zugzwang);
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
 * @param attacker attacking side (i.e. side that has just played)
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param refutations table containing refutations after move just
 *                    played
 */
void d_composite_solve_variations(Side attacker,
                                  int n,
                                  int len_threat,
                                  int threats,
                                  int refutations,
                                  slice_index si)
{
  Side defender = advers(attacker);
  int ntcount = 0;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d",len_threat);
  TraceFunctionParam("%d\n",si);

  if (n-1>min_length_nontrivial)
  {
    ntcount = count_non_trivial(defender,si);
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
          && d_composite_does_attacker_win(attacker,n-2,si))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (len_threat>1
               && d_composite_does_attacker_win(attacker,len_threat-1,si))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (d_composite_has_defender_lost(attacker,si)) /* TODO exact */
        ; /* oops! short end */
      else if (!d_composite_defends_against_threats(attacker,
                                                    len_threat,
                                                    threats,
                                                    si))
        ; /* move doesn't defend against threat */
      else
        d_composite_write_variation(attacker,n-1,si);
    }
    repcoup();
  }

  finply();

  if (n-1>min_length_nontrivial)
    max_nr_nontrivial += ntcount;
  
  TraceFunctionExit(__func__);
} /* d_composite_solve_variations */

void d_composite_middle_solve_postkey(Side attacker,
                                      int n,
                                      int refutations,
                                      slice_index si)
{
  int threats = alloctab();
  int len_threat = d_composite_middle_solve_threats(attacker,
                                                    n,
                                                    threats,
                                                    si);
  d_composite_solve_variations(attacker,
                               n,
                               len_threat,
                               threats,
                               refutations,
                               si);
  freetab();
}

/* Determine and write the end in direct/self/reflex play
 * (i.e. attacker's final move and possible play following it).
 * This is an indirectly recursive function.
 * @param attacker attacking side
 * @param t table where to store continuing moves (i.e. threats)
 */
static void d_composite_end_solve_continuations(Side attacker,
                                                int t,
                                                slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].type);
  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_solve_continuations(attacker,t,si);
      break;

    case STSequence:
      d_sequence_end_solve_continuations(attacker,t,si);
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
 * @param t table where to store continuing moves (i.e. threats)
 */
void d_composite_solve_continuations(Side attacker,
                                     int n,
                                     int t,
                                     slice_index si)
{
  Side defender = advers(attacker);
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n==1)
    d_composite_end_solve_continuations(attacker,t,si);
  else
  {
    genmove(attacker);

    while (encore())
    {
      TraceCurrentMove();
      if (jouecoup()
          && !echecc(attacker))
      {
        d_composite_win_type const defender_success =
            d_composite_does_defender_win(defender,n-1,si);
        TraceValue("%d\n",defender_success);
        if (defender_success>=loss)
        {
          d_write_attack(no_goal);

          marge+= 4;
          if (!slices[si].u.composite.is_exact
              && defender_success>=short_loss)
          {
            d_composite_end_solve_variations(defender,
                                             0,
                                             alloctab(),
                                             alloctab(),
                                             si);
            freetab();
            freetab();
          }
          else
          {
            d_composite_middle_solve_postkey(attacker,n,alloctab(),si);
            freetab();
          }
          marge-= 4;

          pushtabsol(t);
        }
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
} /* d_composite_solve_continuations */

static void d_composite_end_solve_setplay(Side defender, slice_index si)
{
  switch (slices[si].type)
  {
    case STSequence:
      d_sequence_end_solve_setplay(defender,si);
      break;

    case STQuodlibet:
      d_quodlibet_end_solve_setplay(defender,si);
      break;

    default:
      assert(0);
      break;
  }
}

static boolean d_composite_end_solve_complete_set(Side defender,
                                                  slice_index si)
{
  switch (slices[si].type)
  {
    case STSequence:
      return d_sequence_end_solve_complete_set(defender,si);

    case STQuodlibet:
      return d_quodlibet_end_solve_complete_set(defender,si);

    default:
      assert(0);
      return false;
  }
}

/* Determine and write set play of a direct/self/reflex stipulation
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached,
 *          including the virtual key move
 */
void d_composite_solve_setplay(Side attacker, int n, slice_index si)
{
  Side const defender = advers(attacker);
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);

  if (n==1)
    d_composite_end_solve_setplay(defender,si);
  else
  {
    int ntcount = 0;

    if (!d_composite_end_solve_complete_set(defender,si))
      StdString("\n");

    if (n-1>min_length_nontrivial)
    {
      ntcount = count_non_trivial(defender,si);
      max_nr_nontrivial -= ntcount;
    }

    genmove(defender);

    while(encore())
    {
      TraceCurrentMove();
      if (jouecoup()
          && !echecc(defender))
      {
        /* TODO exact? */
        if (d_composite_has_defender_lost(attacker,si))
          ; /* oops */
        else if (d_composite_does_attacker_win(attacker,n-1,si))
          /* yipee - this solves! */
          d_composite_write_variation(attacker,n-1,si);
      }

      repcoup();
    }

    finply();

    if (n-1>min_length_nontrivial)
      max_nr_nontrivial += ntcount;
  }

  TraceFunctionExit(__func__);
} /* d_composite_solve_setplay */

static void d_composite_end_solve(Side attacker,
                                  boolean restartenabled,
                                  slice_index si)
{
  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_solve(attacker,restartenabled,si);
      break;

    case STSequence:
      d_sequence_end_solve(attacker,restartenabled,si);
      break;

    default:
      assert(0);
      break;
  }
}

static void d_composite_end_write_key_solve_postkey(Side attacker,
                                                    int refutations,
                                                    slice_index si,
                                                    boolean is_try)
{
  switch (slices[si].type)
  {
    case STQuodlibet:
      d_quodlibet_end_write_key_solve_postkey(attacker,
                                              refutations,
                                              si,
                                              is_try);
      break;

    case STSequence:
      d_sequence_end_write_key_solve_postkey(attacker,
                                             refutations,
                                             si,
                                             is_try);
      break;

    default:
      assert(0);
      break;
  }
}

void d_composite_write_key_solve_postkey(Side attacker,
                                         int n,
                                         int refutations,
                                         slice_index si,
                                         boolean is_try)
{
  d_write_key(no_goal,is_try);

  marge+= 4;

  if (OptFlag[solvariantes])
    d_composite_middle_solve_postkey(attacker,n,refutations,si);
  else
    Message(NewLine);

  d_write_refutations(refutations);

  marge-= 4;
}

void d_composite_solve_postkey(Side attacker, int n, slice_index si)
{
  Side const defender = advers(attacker);

  if (n==1)
  {
    d_composite_end_solve_variations(attacker,0,alloctab(),alloctab(),si);
    freetab();
    freetab();
  }
  else if (slices[si].u.composite.is_exact)
  {
    d_composite_middle_solve_postkey(attacker,n,alloctab(),si);
    freetab();
  }
  else if (d_composite_has_attacker_won(defender,si))
  {
    d_composite_end_solve_variations(attacker,0,alloctab(),alloctab(),si);
    freetab();
    freetab();
  }
  else
  {
    d_composite_middle_solve_postkey(attacker,n,alloctab(),si);
    freetab();
  }
}

/* Determine and write tries and solutios in a "regular"
 * direct/self/reflex stipulation.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @return true iff >=1 solution was found and written
 */
static void d_composite_middle_solve(Side attacker,
                                     int n,
                                     boolean restartenabled,
                                     slice_index si)
{
  Side const defender = advers(attacker);
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",n);
  TraceFunctionParam("%d\n",si);
  assert(n>1);

  genmove(attacker);

  while (encore())
  {
    TraceCurrentMove();
    if (jouecoup()
        && !(restartenabled && MoveNbr<RestartNbr)
        && !echecc(attacker))
    {
      if (!slices[si].u.composite.is_exact
          && d_composite_has_attacker_won(defender,si))
      {
        int refutations = alloctab();
        boolean const is_try = false;
        d_composite_end_write_key_solve_postkey(attacker,
                                                refutations,
                                                si,
                                                is_try);
        freetab();
      }
      else
      {
        int refutations = alloctab();
        int const nr_refutations = d_composite_find_refutations(defender,
                                                                n-1,
                                                                refutations,
                                                                si);
        TraceValue("%d\n",nr_refutations);
        if (nr_refutations<=max_nr_refutations)
        {
          boolean const is_try = tablen(refutations)>=1;
          d_composite_write_key_solve_postkey(attacker,
                                              n,
                                              refutations,
                                              si,
                                              is_try);
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
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 */
void d_composite_solve(Side attacker,
                       int n,
                       boolean restartenabled,
                       slice_index si)
{
  zugebene = 1;

  if (d_composite_has_defender_lost(attacker,si))
    ; /* TODO  - write this? */
  else if (d_composite_has_defender_won(attacker,si))
    ; /* TODO if attacker has to deliver reflex mate, write it? */
  else if (n==1)
    d_composite_end_solve(attacker,restartenabled,si);
  else
    d_composite_middle_solve(attacker,n,restartenabled,si);

  zugebene = 0;
} /* d_composite_solve */

/* Intialize starter field with the starting side if possible, and
 * no_side otherwise. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void composite_init_starter(slice_index si, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[si].type)
  {
    case STLeaf:
      leaf_init_starter(si,is_duplex);
      break;

    case STSequence:
      sequence_init_starter(si,is_duplex);
      break;

    case STReciprocal:
      reci_init_starter(si,is_duplex);
      break;

    case STQuodlibet:
      quodlibet_init_starter(si,is_duplex);
      break;

    default:
      assert(0);
      break;
  }

  if (slices[si].type!=STLeaf
      && slices[si].u.composite.play==PHelp
      && slices[si].u.composite.length%2 == 1)
  {
    if (slices[si].starter==no_side)
      slices[si].starter = no_side;
    else
      slices[si].starter = advers(slices[si].starter);
  }

  TraceValue("%d\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}
