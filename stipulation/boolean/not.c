#include "pynot.h"
#include "pystip.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a not slice.
 * @param op operand
 * @return index of allocated slice
 */
slice_index alloc_not_slice(slice_index op)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",op);

  assert(op!=no_slice);

  slices[result].type = STNot;
  slices[result].u.not.op = op;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void not_write_unsolvability(slice_index si)
{
  output_start_unsolvability_level();
  slice_solve(slices[si].u.not.op);
  output_end_unsolvability_level();
}

/* Determine whether there is >= 1 solution
 * @param si slice index
 * @return true iff there is >=1 solution
 */
boolean not_is_solvable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = !slice_is_solvable(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect a priori unsolvability (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean not_is_unsolvable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* *_is_unsolvable() and *_has_solution() seem to be
   * complimentary operations - eliminate one of them?
   */
  result = slice_has_solution(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the solution
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean not_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* We have to return the correct value or a wrong value will be
   * written to the hash table!
   */
  result = !slice_has_solution(slices[si].u.not.op);

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
boolean not_has_non_starter_refuted(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* *_has_non_starter_refuted() and *_has_solution() seem to be
   * complimentary operations - eliminate one of them?
   */
  result = slice_has_solution(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean not_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = !slice_has_solution(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of slice
 */
void not_solve_continuations(int continuations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceFunctionExit(__func__);
  TraceText("%n");
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si slice index
 * @return true iff the threat is refuted
 */
boolean not_is_threat_refuted(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* TODO *_end_has_starter_won() and *_is_threat_refuted() seem to be
   * complimentary operations - eliminate one of them? */
  result = slice_end_has_starter_won(slices[si].u.not.op);

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
boolean not_has_starter_lost(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* TODO *_has_starter_lost() and *_end_has_starter_won() seem to be
   * complimentary operations - eliminate one of them? */
  result = slice_end_has_starter_won(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void not_detect_starter(slice_index si, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  slice_detect_starter(slices[si].u.not.op,is_duplex);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void not_impose_starter(slice_index si, Side s)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",s);

  slice_impose_starter(slices[si].u.not.op,s);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write the solution of a slice
 * @param slice index
 * @return true iff >=1 move pair was found
 */
boolean not_root_solve(slice_index si)
{
  return true;
}
