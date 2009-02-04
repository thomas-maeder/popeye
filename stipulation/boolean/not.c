#include "pynot.h"
#include "pyslice.h"
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

/* Is there no chance left for the starting side to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
void not_write_unsolvability(slice_index si)
{
  output_start_unsolvability_level();
  slice_solve(slices[si].u.not.op);
  output_end_unsolvability_level();
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean not_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

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

  /* Don't write anything, but return the correct value so that it can
   * be written to the hash table!
   */
  result = !slice_has_solution(slices[si].u.not.op);

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
  TraceText("\n");
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean not_has_starter_apriori_lost(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_starter_won(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean not_has_starter_reached_goal(slice_index si)
{
  boolean result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = !slice_has_starter_reached_goal(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean not_has_starter_won(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = !slice_has_starter_won(slices[si].u.not.op);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter not_detect_starter(slice_index si,
                                          boolean is_duplex,
                                          boolean same_side_as_root)
{
  who_decides_on_starter result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  result = slice_detect_starter(slices[si].u.not.op,
                                is_duplex,
                                same_side_as_root);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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

/* Spin off a set play slice
 * Assumes that slice_root_prepare_for_setplay(si) was invoked and
 * did not return no_slice
 * @param si slice index
 * @return set play slice spun off
 */
slice_index not_root_make_setplay_slice(slice_index si)
{
  slice_index result;
  slice_index op_set;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  op_set = slice_root_make_setplay_slice(slices[si].u.not.op);
  if (op_set==no_slice)
    result = no_slice;
  else
    result = alloc_not_slice(op_set);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write the solution of a slice
 * @param slice index
 * @return true iff >=1 move pair was found
 */
boolean not_root_solve(slice_index si)
{
  return true;
}
