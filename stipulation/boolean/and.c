#include "pyrecipr.h"
#include "pyproc.h"
#include "pydata.h"
#include "pystip.h"
#include "pycompos.h"
#include "trace.h"

#include <assert.h>

extern boolean hashing_suspended; /* TODO */


/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean reci_end_is_unsolvable(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  result = slice_is_unsolvable(op1) || slice_is_unsolvable(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defending side wins at the end of reciprocal
 * in direct play. 
 * @param si slice identifier
 */
d_defender_win_type d_reci_end_does_defender_win(slice_index si)
{
  d_defender_win_type result = loss;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  if (d_slice_does_defender_win(slices[si].u.composite.op1)<=win
      || d_slice_does_defender_win(slices[si].u.composite.op2)<=win)
    result = win;

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker wins at the end of a reciprocal slice
 * @param si slice index
 * @param parent_is_exact true iff parent of slice si has exact length
 * @return true iff attacker wins
 */
boolean d_reci_end_does_attacker_win(slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;
  boolean result;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  /* avoid conflict in hash table between op1 and op2 */
  /* TODO use disjoint "hash slots" to avoid this conflict while
   * hashing both in op1 and op2 */
  hashing_suspended = true;

  result = (d_slice_does_attacker_win(op1)
            && d_slice_does_attacker_win(op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%d\n",result);
  return result;
}

/* Determine whether the defender has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly lost
 */
boolean d_reci_end_has_defender_lost(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  result = d_slice_has_defender_lost(op1) && d_slice_has_defender_lost(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_reci_end_has_defender_won(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  result = d_slice_has_defender_won(op1) || d_slice_has_defender_won(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_reci_end_has_attacker_lost(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  result = d_slice_has_attacker_lost(op1) || d_slice_has_attacker_lost(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_reci_end_has_attacker_won(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  result = d_slice_has_attacker_won(op1) && d_slice_has_attacker_won(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void d_reci_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d",slices[si].u.composite.op1);
  TraceValue("%d\n",slices[si].u.composite.op2);

  d_slice_write_unsolvability(slices[si].u.composite.op1);
  d_slice_write_unsolvability(slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write variations from the end of a reciprocal slice.
 * @param len_threat length of threat (shorter variations are suppressed) 
 * @param threats table containing threats (variations not defending
 *                against all threats are suppressed)
 * @param refutations table containing refutations (written at end)
 * @param si slice index
 */
void d_reci_end_solve_variations(int len_threat,
                                 int threats,
                                 int refutations,
                                 slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",len_threat);
  TraceFunctionParam("%d\n",si);

  /* TODO solve variation after variation */
  d_slice_solve_variations(len_threat,
                           threats,
                           refutations,
                           slices[si].u.composite.op1);
  d_slice_solve_variations(len_threat,
                           threats,
                           refutations,
                           slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
}

/* Determine and write continuations at end of reciprocal slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void d_reci_end_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  d_slice_solve_continuations(table,slices[si].u.composite.op1);
  d_slice_solve_continuations(table,slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write defender's set play
 * @param si slice index
 */
void d_reci_end_solve_setplay(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  /* TODO solve defense after defense */
  d_slice_solve_setplay(slices[si].u.composite.op1);
  d_slice_solve_setplay(slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write solutions starting at the end of a reciprocal
 * direct/self/reflex stipulation. 
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_reci_end_solve(boolean restartenabled, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  /* TODO does this makes sense? */
  if (slice_is_unsolvable(op1))
    d_slice_write_unsolvability(op1);
  else if (slice_is_unsolvable(op2))
    d_slice_write_unsolvability(op2);
  else
  {
    d_slice_solve(restartenabled,op1);
    d_slice_solve(restartenabled,op2);
  }
}

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any), starting at the end
 * of a reciprocal slice.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_reci_end_write_key_solve_postkey(int refutations,
                                        slice_index si,
                                        boolean is_try)
{
  /* TODO does this makes sense? */
  d_slice_write_key_solve_postkey(refutations,
                                  slices[si].u.composite.op1,
                                  is_try);
  d_slice_write_key_solve_postkey(refutations,
                                  slices[si].u.composite.op2,
                                  is_try);
}

/* Continue solving at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_reci_end_solve(slice_index si)
{
  boolean found_solution = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  hashing_suspended = true;

  found_solution = (slice_is_solvable(op2)
                    && h_slice_solve(false,op1)
                    && h_slice_solve(false,op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
} /* h_reci_end_solve */

/* Continue solving series play at the end of a reciprocal slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean ser_reci_end_solve(boolean restartenabled, slice_index si)
{
  boolean solution_found = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  solution_found = (slice_is_solvable(op2)
                    && ser_slice_solve(false,op1)
                    && ser_slice_solve(false,op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void reci_detect_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",is_duplex);

  slice_detect_starter(op1,is_duplex);
  slice_detect_starter(op2,is_duplex);

  slices[si].starter = no_side;

  if (slices[op1].starter==no_side && slices[op1].type==STLeaf)
  {
    /* op1 can't tell - let's tell him */
    slices[si].starter = slices[op2].starter;
    slices[op1].starter = slices[op2].starter;
  }
  else if (slices[op2].starter==no_side && slices[op2].type==STLeaf)
  {
    /* op2 can't tell - let's tell him */
    slices[si].starter = slices[op1].starter;
    slices[op2].starter = slices[op1].starter;
  }
  else if (slices[op1].starter==slices[op2].starter)
    slices[si].starter = slices[op1].starter;

  TraceValue("%d\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of leaf
 */
void reci_impose_starter(slice_index si, Side s)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  Side const next_starter = (slices[si].u.composite.play==PHelp
                             && slices[si].u.composite.length%2==1
                             ? advers(s)
                             : s);

  slices[si].starter = s;
  slice_impose_starter(op1,next_starter);
  slice_impose_starter(op2,next_starter);
}
