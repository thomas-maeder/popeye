#include "pybrad.h"
#include "pybranch.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "pyhash.h"
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
static boolean branch_d_is_in_hash(slice_index si,
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

/* Count all non-trivial moves of the defending side. Whether a
 * particular move is non-trivial is determined by user input.
 * @return number of defender's non-trivial moves minus 1 (TODO: why?)
 */
static int count_non_trivial(slice_index si)
{
  Side const attacker = slices[si].u.branch.starter;
  Side const defender = advers(attacker);
  int result = -1;

  genmove(defender);

  while (encore() && max_nr_nontrivial>=result)
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,defender)
        && !(min_length_nontrivial>0
             && branch_d_has_solution_in_n(si,min_length_nontrivial)))
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
d_defender_win_type branch_d_helper_does_defender_win(slice_index si,
                                                      stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
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
	  && !branch_d_has_solution_in_n(si,max_len_threat))
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
	  if (!branch_d_has_solution_in_n(si,n))
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
static d_defender_win_type branch_d_does_defender_win(slice_index si,
                                                      stip_length_type n)
{
  d_defender_win_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  assert(n>=slack_length_direct);

  TraceValue("%u\n",slices[si].u.branch.min_length);

  if (slice_end_has_starter_lost(si))
    result = already_won;
  else if ((slices[si].u.branch.length-n
            >slices[si].u.branch.min_length-slack_length_direct)
           && slice_end_has_starter_won(si))
    result = short_loss;
  else
    result = branch_d_helper_does_defender_win(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacking side wins in the middle of a
 * composite slice
 * @param attacker attacking side
 * @param si slice identifier
 */
static boolean branch_d_helper_has_solution(slice_index si,
                                            stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
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
      if (branch_d_does_defender_win(si,n-1)>=loss)
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
boolean branch_d_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (slice_end_has_non_starter_refuted(si))
    ; /* intentionally nothing */
  else
  {
    stip_length_type const moves_played = slices[si].u.branch.length-n;
    stip_length_type const
        min_moves_played = (slices[si].u.branch.min_length
                            -slack_length_direct);
    TraceValue("%u",moves_played);
    TraceValue("%u\n",min_moves_played);
    if (moves_played>=min_moves_played
        && slice_has_solution(slices[si].u.branch.next))
      result = true;
    else if (moves_played>min_moves_played
             && slice_has_non_starter_solved(si))
      result = true;
    else if (n>slack_length_direct)
    {
      HashBuffer hb;
      (*encode)(&hb);
      if (!branch_d_is_in_hash(si,&hb,n,&result))
      {
        stip_length_type i;
        stip_length_type n_min = slack_length_direct+1;

        if (min_moves_played>moves_played-slack_length_direct)
          n_min = min_moves_played-(moves_played-slack_length_direct);

        for (i = n_min; !result && i<=n; i++)
        {
          if (i-1>max_len_threat || i>min_length_nontrivial)
            i = n;

          result = branch_d_helper_has_solution(si,i);

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
static int branch_d_find_refutations(int t, slice_index si)
{
  Side const defender = advers(slices[si].u.branch.starter);
  boolean is_defender_immobile = true;
  int ntcount = 0;
  int result = 0;
  stip_length_type const n = slices[si].u.branch.length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n>max_len_threat
      && !echecc(nbply,defender)
      && !branch_d_has_solution_in_n(si,max_len_threat))
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
      if (!branch_d_has_solution_in_n(si,n))
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


/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean branch_d_is_threat_in_n_refuted(slice_index si, stip_length_type n)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_direct)
    result = slice_is_threat_refuted(slices[si].u.branch.next);
  else
    result = branch_d_does_defender_win(si,n-1)<=win;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the move just played by the defending side
 * defends against the threats.
 * @param n number of moves until end state has to be reached from now
 * @param threats table containing the threats
 * @return true iff the move just played defends against at least one
 *         of the threats
 */
static boolean branch_d_defends_against_threats(int threats,
                                                slice_index si,
                                                stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
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
        defense_found = branch_d_is_threat_in_n_refuted(si,n);
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

/* Write a variation in the try/solution/set play of a
 * direct/self/reflex stipulation. The move of the defending side that
 * starts the variation has already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * This is an indirectly recursive function.
 * @param n number of moves until end state has to be reached from now
 */
static void branch_d_write_variation(slice_index si, stip_length_type n)
{
  boolean isRefutation = true; /* until we prove otherwise */
  stip_length_type i;
  stip_length_type const min_len = (slices[si].u.branch.min_length>n
                                    ? n
                                    : slices[si].u.branch.min_length);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  write_defense();

  output_start_continuation_level();
  
  for (i = min_len; i<=n && isRefutation; i++)
  {
    int const continuations = alloctab();
    branch_d_solve_continuations_in_n(continuations,si,i);
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
static int branch_d_solve_threats(int threats,
                                  slice_index si,
                                  stip_length_type n)
{
  Side const defender = advers(slices[si].u.branch.starter);
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
      branch_d_solve_continuations_in_n(threats,si,i);
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
static void branch_d_root_solve_variations(int len_threat,
                                           int threats,
                                           int refutations,
                                           slice_index si,
                                           stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
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
          && branch_d_has_solution_in_n(si,n-2))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (len_threat>1
               && branch_d_has_solution_in_n(si,len_threat-1))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (slice_has_non_starter_solved(si))
        ; /* oops! */
      else if (!branch_d_defends_against_threats(threats,si,len_threat))
        ; /* move doesn't defend against threat */
      else
        branch_d_write_variation(si,n-1);
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
static void branch_d_solve_variations_in_n(int len_threat,
                                           int threats,
                                           slice_index si,
                                           stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;
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
          && branch_d_has_solution_in_n(si,n-2))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (len_threat>1
               && branch_d_has_solution_in_n(si,len_threat-1))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (slice_has_non_starter_solved(si))
        ; /* oops! short end. */
      else if (!branch_d_defends_against_threats(threats,si,len_threat))
        ; /* move doesn't defend against threat */
      else
        branch_d_write_variation(si,n-1);
    }

    repcoup();
  }

  finply();

  if (n-1>min_length_nontrivial)
    max_nr_nontrivial += ntcount;
  
  TraceFunctionExit(__func__);
}

/* Determine and write the threat and variations in direct
 * play after the move that has just been played in the current ply.
 * @param si slice index
 */
void branch_d_solve_variations(slice_index si)
{
  stip_length_type const n = slices[si].u.branch.length;
  stip_length_type const max_threat_length = (n-1>max_len_threat
                                              ? max_len_threat
                                              : n-1);
  int const threats = alloctab();
  branch_d_solve_variations_in_n(max_threat_length,threats,si,n);
  freetab();
}

/* Solve postkey play of a composite slice at root level.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param si slice index
 */
static void branch_d_root_solve_postkey(int refutations,
                                        slice_index si,
                                        stip_length_type n)
{
  int const threats = alloctab();
  int const len_threat = branch_d_solve_threats(threats,si,n);
  branch_d_root_solve_variations(len_threat,threats,refutations,si,n);
  freetab();
}

/* Solve postkey play of a composite slice.
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param si slice index
 */
static void branch_d_solve_postkey(slice_index si, stip_length_type n)
{
  int const threats = alloctab();
  int len_threat;

  output_start_postkey_level();
  len_threat = branch_d_solve_threats(threats,si,n);
  branch_d_solve_variations_in_n(len_threat,threats,si,n);
  freetab();
  output_end_postkey_level();
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
void branch_d_solve_continuations_in_n(int continuations,
                                       slice_index si,
                                       stip_length_type n)
{
  Side const attacker = slices[si].u.branch.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u\n",si);

  if (n==slack_length_direct)
    slice_solve_continuations(continuations,slices[si].u.branch.next);
  else
  {
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !echecc(nbply,attacker))
      {
        d_defender_win_type const
            defender_success = branch_d_does_defender_win(si,n-1);
        TraceValue("%u\n",defender_success);
        if (defender_success>=loss)
        {
          write_attack(no_goal,attack_regular);

          if (defender_success>=short_loss)
            slice_solve_variations(slices[si].u.branch.next);
          else
            branch_d_solve_postkey(si,n);

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

/* Determine and write set play in direct play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_d_root_solve_setplay(slice_index si)
{
  Side const defender = advers(slices[si].u.branch.starter);
  stip_length_type const n = slices[si].u.branch.length;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_setplay_level();

  if (n==slack_length_direct)
    result = slice_root_solve_setplay(slices[si].u.branch.next);
  else
  {
    int ntcount = 0;

    if (slice_root_end_solve_complete_set(slices[si].u.branch.next))
      result = true;
    else
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
        else if (branch_d_has_solution_in_n(si,n-1))
        {
          /* yipee - this solves! */
          branch_d_write_variation(si,n-1);
          result = true;
        }
      }

      repcoup();
    }

    finply();

    if (n-1>min_length_nontrivial)
      max_nr_nontrivial += ntcount;
  }

  output_end_setplay_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any).
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param type type of attack
 */
void branch_d_root_write_key_solve_postkey(int refutations,
                                           slice_index si,
                                           attack_type type)
{
  write_attack(no_goal,type);

  output_start_postkey_level();
  if (OptFlag[solvariantes])
    branch_d_root_solve_postkey(refutations,
                                si,
                                slices[si].u.branch.length);
  write_refutations(refutations);
  output_end_postkey_level();
}

/* Solve the postkey play only of a composite slice at root level.
 * @param si slice index
 * @param type type of attack
 */
static void branch_d_root_solve_postkeyonly(slice_index si,
                                            stip_length_type n)
{
  output_start_postkeyonly_level();

  if (n==slack_length_direct)
    slice_solve_variations(slices[si].u.branch.next);
  else if (n==slices[si].u.branch.min_length)
    branch_d_solve_postkey(si,n);
  else if (slice_end_has_starter_won(si))
    slice_solve_variations(slices[si].u.branch.next);
  else
    branch_d_solve_postkey(si,n);

  output_end_postkeyonly_level();
}

/* Determine and write the solutions and tries in the current position
 * in direct/self/reflex play.
 * @param si slice index
 */
static void branch_d_root_solve_real_play(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_continuation_level();

  if (slice_has_non_starter_solved(si))
    ;
  else if (slice_end_has_non_starter_refuted(si))
    slice_write_unsolvability(slices[si].u.branch.next);
  else if (slices[si].u.branch.length==slack_length_direct)
    slice_root_solve(slices[si].u.branch.next);
  else
  {
    Side const attacker = slices[si].u.branch.starter;
    genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !(OptFlag[restart] && MoveNbr<RestartNbr)
          && !echecc(nbply,attacker))
      {
        if (slices[si].u.branch.min_length==slack_length_direct
            && slice_end_has_starter_won(si))
          slice_root_write_key_solve_postkey(slices[si].u.branch.next,
                                             attack_key);
        else
        {
          int refutations = alloctab();
          int const nr_refutations =
              branch_d_find_refutations(refutations,si);
          TraceValue("%u\n",nr_refutations);
          if (nr_refutations<=max_nr_refutations)
          {
            attack_type const type = (tablen(refutations)>=1
                                      ? attack_try
                                      : attack_key);
            branch_d_root_write_key_solve_postkey(refutations,si,type);
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
  TraceText("\n");
}

/* Solve a composite direct slice at root level
 * @param si slice index
 */
void branch_d_root_solve(slice_index si)
{
  stip_length_type const n = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  init_output_mode(output_mode_tree);

  if (OptFlag[postkeyplay])
  {
    if (echecc(nbply,slices[si].u.branch.starter))
      ErrorMsg(SetAndCheck);
    else
      branch_d_root_solve_postkeyonly(si,n);
  }
  else
  {
    if (OptFlag[solapparent] && n>slack_length_direct)
    {
      if (echecc(nbply,slices[si].u.branch.starter))
        ErrorMsg(SetAndCheck);
      else
      {
        branch_d_root_solve_setplay(si);
        Message(NewLine);
      }
    }

    if (echecc(nbply,advers(slices[si].u.branch.starter)))
      ErrorMsg(KingCapture);
    else
      branch_d_root_solve_real_play(si);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_d_impose_starter(slice_index si, Side s)
{
  slices[si].u.branch.starter = s;
  slice_impose_starter(slices[si].u.branch.next,s);
}
