#include "pysequen.h"
#include "pystip.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
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

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void sequence_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.composite.op1);
}

/* Determine and write continuations at end of sequence slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of sequence slice
 */
void sequence_end_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  TraceValue("%d\n",slices[si].u.composite.op1);
  d_slice_solve_continuations(table,slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 */
void sequence_root_end_solve_setplay(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  slice_root_solve_setplay(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean d_sequence_root_end_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_root_solve_complete_set(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}


/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void d_sequence_root_end_write_key_solve_postkey(slice_index si,
                                                 attack_type type)
{
  d_slice_root_write_key_solve_postkey(slices[si].u.composite.op1,type);
}

/* Solve at root level at the end of a sequence slice
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean sequence_root_end_solve(boolean restartenabled, slice_index si)
{
  return slice_root_solve(restartenabled,slices[si].u.composite.op1);
}

/* Continue solving at the end of a sequence slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean sequence_end_solve(slice_index si)
{
  return slice_solve(slices[si].u.composite.op1);
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
 * @param si slice index
 */
void d_sequence_end_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  d_slice_solve_variations(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
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
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_has_attacker_won(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Has the threat just played been defended by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_sequence_end_is_threat_refuted(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  result = d_slice_is_threat_refuted(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void sequence_detect_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.composite.op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",is_duplex);

  slice_detect_starter(op1,is_duplex);

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
        slices[si].starter = is_duplex ? Black : White; /* not reci-h */
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

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of slice
 */
void sequence_impose_starter(slice_index si, Side s)
{
  slice_index const op1 = slices[si].u.composite.op1;

  Side next_starter;

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      next_starter = s;
      break;

    case PHelp:
      /* help play in N.5 -> change starter */
      next_starter = (slices[si].u.composite.length%2==1 ? advers(s) : s);
      break;

    case PSeries:
      /* series sequence after series sequence == intro series
       * -> change starter */
      next_starter = (slices[op1].type==STSequence
                      && slices[op1].u.composite.play==PSeries
                      ? advers(s)
                      : s);
      break;

    default:
      assert(0);
      next_starter = s; /* avoid compiler warning */
      break;
  }

  slices[si].starter = s;
  slice_impose_starter(op1,next_starter);
}
