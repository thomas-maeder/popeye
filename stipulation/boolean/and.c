#include "pyrecipr.h"
#include "pystip.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>


/* Allocate a reciprocal slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_reciprocal_slice(slice_index op1, slice_index op2)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result].type = STReciprocal; 
  slices[result].u.reciprocal.op1 = op1;
  slices[result].u.reciprocal.op2 = op2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean reci_is_unsolvable(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = slice_is_unsolvable(op1) || slice_is_unsolvable(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean reci_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_solution(op1) && slice_has_solution(op2);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u\n",result);
  return result;
}

/* Determine whether a reciprocal slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean reci_has_non_starter_solved(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = (slice_has_non_starter_solved(op1)
            && slice_has_non_starter_solved(op2));

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
boolean reci_has_non_starter_refuted(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = (slice_end_has_non_starter_refuted(op1)
            || slice_end_has_non_starter_refuted(op2));

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
boolean reci_has_starter_lost(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = (slice_end_has_starter_lost(op1)
            || slice_end_has_starter_lost(op2));

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
boolean reci_has_starter_won(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = (slice_end_has_starter_won(op1)
            && slice_end_has_starter_won(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void reci_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u",slices[si].u.reciprocal.op1);
  TraceValue("%u\n",slices[si].u.reciprocal.op2);

  slice_write_unsolvability(slices[si].u.reciprocal.op1);
  slice_write_unsolvability(slices[si].u.reciprocal.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write variations from the end of a reciprocal slice.
 * @param si slice index
 */
void reci_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* TODO solve variation after variation */
  slice_solve_variations(slices[si].u.reciprocal.op1);
  slice_solve_variations(slices[si].u.reciprocal.op2);

  TraceFunctionExit(__func__);
}

/* Determine and write continuations at end of reciprocal slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void reci_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_solve_continuations(table,slices[si].u.reciprocal.op1);
  slice_solve_continuations(table,slices[si].u.reciprocal.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean reci_root_solve_setplay(slice_index si)
{
  boolean result1;
  boolean result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* TODO solve defense after defense */
  result1 = slice_root_solve_setplay(slices[si].u.reciprocal.op1);
  result2 = slice_root_solve_setplay(slices[si].u.reciprocal.op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n", result1 && result2);
  return result1 && result2;
}

/* Solve at root level at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
void reci_root_solve(slice_index si)
{
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  if (slice_is_solvable(op2))
  {
    slice_root_solve(op1);
    slice_root_solve(op2);
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the key just played, then solve the post key play (threats,
 * variations), starting at the end of a reciprocal slice.
 * @param si slice index
 * @param type type of attack
 */
void reci_root_write_key_solve_postkey(slice_index si,
                                             attack_type type)
{
  /* TODO does this make sense? */
  slice_root_write_key_solve_postkey(slices[si].u.reciprocal.op1,type);
  slice_root_write_key_solve_postkey(slices[si].u.reciprocal.op2,type);
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean reci_is_threat_refuted(slice_index si)
{
  return (slice_is_threat_refuted(slices[si].u.reciprocal.op1)
          || slice_is_threat_refuted(slices[si].u.reciprocal.op2));
}

/* Continue solving at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_solve(slice_index si)
{
  boolean found_solution = false;
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);
  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  found_solution = (slice_is_solvable(op2)
                    && slice_solve(op1)
                    && slice_solve(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void reci_detect_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.reciprocal.op1;
  slice_index const op2 = slices[si].u.reciprocal.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  slice_detect_starter(op1,is_duplex);
  slice_detect_starter(op2,is_duplex);

  if (slice_get_starter(op1)==no_side)
    /* op1 can't tell - let's tell him */
    slice_impose_starter(op1,slice_get_starter(op2));
  else if (slice_get_starter(op2)==no_side)
    /* op2 can't tell - let's tell him */
    slice_impose_starter(op2,slice_get_starter(op1));

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void reci_impose_starter(slice_index si, Side s)
{
  slice_impose_starter(slices[si].u.reciprocal.op1,s);
  slice_impose_starter(slices[si].u.reciprocal.op2,s);
}
