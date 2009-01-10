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
  TraceFunctionParam("%u\n",si);

  result = slice_is_unsolvable(slices[si].u.composite.op1);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].u.composite.op1);
  slice_solve_continuations(table,slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean sequence_root_end_solve_setplay(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_root_solve_setplay(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean sequence_root_end_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_root_end_solve_complete_set(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void sequence_root_end_write_key_solve_postkey(slice_index si,
                                                 attack_type type)
{
  slice_root_write_key_solve_postkey(slices[si].u.composite.op1,type);
}

/* Solve at root level at the end of a sequence slice
 * @param si slice index
 */
void sequence_root_end_solve(slice_index si)
{
  slice_root_solve(slices[si].u.composite.op1);
}

/* Continue solving at the end of a sequence slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean sequence_end_solve(slice_index si)
{
  return slice_solve(slices[si].u.composite.op1);
}

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean sequence_end_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_solution(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write variations starting at end of sequence slice
 * @param si slice index
 */
void sequence_end_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_solve_variations(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether a sequence slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean sequence_end_has_non_starter_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_non_starter_solved(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the non-starter has refuted with his move just
 * played independently of the starter's possible play during the
 * current slice.
 * Example: in direct play, the defender has just captured that last
 * piece that could deliver mate.
 * @param si slice identifier
 * @return true iff the non-starter has refuted
 */
boolean sequence_end_has_non_starter_refuted(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_end_has_non_starter_refuted(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has lost with its move just
 * played independently of his possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean sequence_end_has_starter_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_end_has_starter_lost(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean sequence_end_has_starter_won(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_end_has_starter_won(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Has the threat just played been defended by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean sequence_end_is_threat_refuted(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_is_threat_refuted(slices[si].u.composite.op1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  slice_detect_starter(op1,is_duplex);

  slices[si].u.composite.starter = no_side;

  switch (slices[op1].type)
  {
    case STSequence:
      if (slice_get_starter(op1)!=no_side)
        slices[si].u.composite.starter = advers(slice_get_starter(op1));
      break;

    case STLeaf:
      if (slice_get_starter(op1)==no_side)
      {
        /* op1 can't tell - let's tell him */
        slices[si].u.composite.starter = is_duplex ? Black : White; /* not reci-h */
        slices[op1].u.leaf.starter = slices[si].u.composite.starter;
      }
      else
        slices[si].u.composite.starter = slice_get_starter(op1);
      break;

    default:
      slices[si].u.composite.starter = slice_get_starter(op1);
      break;
  }

  TraceValue("%u\n",slices[si].u.composite.starter);
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

  slices[si].u.composite.starter = s;
  slice_impose_starter(op1,next_starter);
}
