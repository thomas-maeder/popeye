#include "pysequen.h"
#include "pystip.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param leaf leaf's slice index
 * @return true iff leaf is a priori unsolvable
 */
boolean sequence_end_is_unsolvable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = slice_is_unsolvable(slices[si].u.composite.op1);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void d_sequence_write_unsolvability(slice_index si)
{
  d_slice_write_unsolvability(slices[si].u.composite.op1);
}

/* Determine and write continuations at end of sequence slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of sequence slice
 */
void d_sequence_end_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].u.composite.op1);
  d_slice_solve_continuations(table,slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write defender's set play
 * @param leaf slice index
 */
void d_sequence_end_solve_setplay(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  d_slice_solve_setplay(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean d_sequence_end_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_solve_complete_set(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}


/* Continue solving at the end of a sequence slice
 * Unsolvability (e.g. because of a forced reflex move) has already
 * been dealt with.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param leaf slice index 
 */
void d_sequence_end_solve(boolean restartenabled, slice_index si)
{
  d_slice_solve(restartenabled,slices[si].u.composite.op1);
}

/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations) and
 * write the refutations (if any)
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 * @param is_try true iff what we are solving is a try
 */
void d_sequence_end_write_key_solve_postkey(int refutations,
                                            slice_index si,
                                            boolean is_try)
{
  d_slice_write_key_solve_postkey(refutations,
                                  slices[si].u.composite.op1,
                                  is_try);
}

/* Continue solving at the end of a sequence slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_sequence_end_solve(boolean restartenabled, slice_index si)
{
  return h_slice_solve(restartenabled,slices[si].u.composite.op1);
}

extern boolean hashing_suspended; /* TODO */

/* Continue solving series play at the end of a sequence slice
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean ser_sequence_end_solve(boolean restartenabled, slice_index si)
{
  boolean solution_found = false;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  solution_found = ser_slice_solve(restartenabled,
                                   slices[si].u.composite.op1);

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Determine whether the attacking side wins at the end of a sequence slice
 * @param si slice identifier
 * @return true iff attacker wins
 */
boolean d_sequence_end_does_attacker_win(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_does_attacker_win(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Find and write variations starting at end of sequence slice
 * @param len_threat length of threat (shorter variations are suppressed)
 * @param threats table containing threats (variations not defending
 *                against all threats are suppressed)
 * @param refutations table containing refutations (written at end)
 * @param si slice index
 */
void d_sequence_end_solve_variations(int len_threat,
                                     int threats,
                                     int refutations,
                                     slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",len_threat);
  TraceFunctionParam("%d\n",si);

  d_slice_solve_variations(len_threat,
                           threats,
                           refutations,
                           slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether the defending side wins in 0 (its final half
 * move) in direct play.
 * @param si slice identifier
 */
d_defender_win_type d_sequence_end_does_defender_win(slice_index si)
{
  d_defender_win_type result = win;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_does_defender_win(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has lost in direct play with his move
 * just played.
 * Assumes that there is no short win for the defending side.
 * @param si slice identifier
 * @return whether there is a short win or loss
 */
boolean d_sequence_end_has_defender_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_has_defender_lost(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the defender has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_sequence_end_has_defender_won(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_has_defender_won(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_sequence_end_has_attacker_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_has_attacker_lost(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_sequence_end_has_attacker_won(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_has_attacker_won(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Intialize starter field with the starting side if possible, and
 * no_side otherwise. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void sequence_init_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.composite.op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",is_duplex);

  slice_init_starter(op1,is_duplex);

  slices[si].starter = no_side;

  switch (slices[op1].type)
  {
    case STSequence:
      if (slices[op1].starter!=no_side)
        slices[si].starter = advers(slices[op1].starter);
      break;

    case STLeaf:
      if (slices[op1].starter==no_side)
      {
        /* op1 can't tell - let's tell him */
        slices[si].starter = is_duplex ? noir : blanc; /* not reci-h */
        slices[op1].starter = slices[si].starter;
      }
      else
        slices[si].starter = slices[op1].starter;
      break;

    default:
      slices[si].starter = slices[op1].starter;
      break;
  }

  TraceValue("%d\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}
