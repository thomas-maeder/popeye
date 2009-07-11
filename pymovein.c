#include "pymovein.h"
#include "pyslice.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "pydata.h"
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
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  slices[result].type = STMoveInverter; 
  slices[result].u.pipe.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * Tests do not rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff starter must resign
 */
boolean move_inverter_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = slice_must_starter_resign(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @param just_moved side that has just moved
 * @return true iff no chance is left
 */
boolean move_inverter_must_starter_resign_hashed(slice_index si, Side just_moved)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",just_moved);
  TraceFunctionParamListEnd();

  result = slice_must_starter_resign_hashed(slices[si].u.pipe.next,
                                            advers(just_moved));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index move_inverter_root_make_setplay_slice(slice_index si)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index result;
  slice_index next_set_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  next_set_slice = slice_root_make_setplay_slice(next);
  if (next_set_slice==no_slice)
    result = no_slice;
  else
  {
    result = alloc_move_inverter_slice(next_set_slice);
    slices[result].starter = advers(slices[next_set_slice].starter);
    TraceValue("%u\n",slices[result].starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a move inverter slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_move_inverted_level();
  result = slice_root_solve(slices[si].u.pipe.next);
  output_end_move_inverted_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void move_inverter_root_solve_in_n(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_move_inverted_level();
  slice_root_solve_in_n(slices[si].u.pipe.next,n);
  output_end_move_inverted_level();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (echecc(nbply,slices[si].starter))
  {
    TraceText("illegal check\n");
    result = false;
  }
  else
    result = slice_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter
move_inverter_detect_starter(slice_index si,
                             boolean same_side_as_root)
{
  who_decides_on_starter result;
  slice_index const next = slices[si].u.pipe.next;
  Side next_starter;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  result = slice_detect_starter(next,!same_side_as_root);

  next_starter = slices[next].starter;
  if (next_starter==no_side)
    slices[si].starter = no_side;
  else
    slices[si].starter = advers(next_starter);
  TraceValue("->%u\n",slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side move_inverter_get_starter(slice_index si)
{
  Side result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void move_inverter_impose_starter(slice_index si, Side side)
{  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].starter = side;
  slice_impose_starter(slices[si].u.pipe.next,advers(side));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
