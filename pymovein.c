#include "pymovein.h"
#include "pyslice.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Allocate a move inverter slice.
 * @param next next slice
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_slice(slice_index next)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result].type = STMoveInverter; 
  slices[result].u.move_inverter.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean move_inverter_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  result = slice_must_starter_resign(slices[si].u.move_inverter.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a move inverter slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
void move_inverter_root_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_root_solve(slices[si].u.move_inverter.next);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  output_start_move_inverted_level();
  result = slice_solve(slices[si].u.move_inverter.next);
  output_end_move_inverted_level();

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
who_decides_on_starter
move_inverter_detect_starter(slice_index si,
                             boolean is_duplex,
                             boolean same_side_as_root)
{
  who_decides_on_starter result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_detect_starter(slices[si].u.move_inverter.next,
                                is_duplex,
                                !same_side_as_root);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side move_inverter_get_starter(slice_index si)
{
  Side result = no_side;
  slice_index const next = slices[si].u.move_inverter.next;
  Side next_starter;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  next_starter = slice_get_starter(next);
  if (next_starter!=no_side)
    result = advers(next_starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void move_inverter_impose_starter(slice_index si, Side side)
{  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_impose_starter(slices[si].u.move_inverter.next,advers(side));

  TraceFunctionExit(__func__);
  TraceText("\n");
}
