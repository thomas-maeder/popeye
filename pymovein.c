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
  slices[result].starter = no_side;
  slices[result].u.pipe.next = next;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean move_inverter_root_make_setplay_slice(slice_index si,
                                              struct slice_traversal *st)
{
  boolean result;
  slice_index * const next_set_slice = st->param;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = traverse_slices(next,st);
  if (*next_set_slice==no_slice)
    result = false;
  else
  {
    Side const next_set_starter = slices[*next_set_slice].starter;
    *next_set_slice = alloc_move_inverter_slice(*next_set_slice);
    slices[*next_set_slice].starter = advers(next_set_starter);
    TraceEnumerator(Side,slices[*next_set_slice].starter,"\n");
    result = true;
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

  output_start_continuation_level();
  output_start_move_inverted_level();
  result = slice_root_solve(slices[si].u.pipe.next);
  output_end_move_inverted_level();
  output_end_continuation_level();

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

  result = slice_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean move_inverter_detect_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  stip_detect_starter_param_type * const param = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    Side next_starter;
    boolean const save_same_starter_as_root = param->same_starter_as_root;
    param->same_starter_as_root = !save_same_starter_as_root;
    result = slice_traverse_children(si,st);
    param->same_starter_as_root = save_same_starter_as_root;
    next_starter = slices[next].starter;
    if (next_starter!=no_side)
      slices[si].starter = (next_starter==no_side
                            ? no_side:
                            advers(next_starter));
  }
  else
  {
    result = true;
    param->who_decides = leaf_decides_on_starter;
  }

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
