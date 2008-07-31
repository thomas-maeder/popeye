#include "pyrecipr.h"
#include "pyproc.h"
#include "pydata.h"
#include "pystip.h"
#include "pycompos.h"
#include "trace.h"

#include <assert.h>

extern boolean hashing_suspended; /* TODO */

/* Continue solving at the end of a reciprocal slice
 * @param side_at_move side at the move
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_reci_end_solve(Side side_at_move, slice_index si)
{
  boolean found_solution = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",side_at_move);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  hashing_suspended = true;

  if (slice_is_solvable(side_at_move,op2))
    found_solution = (slice_solve(side_at_move,op1)
                      && slice_solve(side_at_move,op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
} /* h_reci_end_solve */

/* Attempt to deremine which side is at the move
 * at the start of a slice.
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @return one of blanc, noir, no_side (the latter if we can't
 *         determine which side is at the move)
 */
Side reci_who_starts(slice_index si, boolean is_duplex)
{
  Side result = no_side;

  slice_index const op1 = slices[si].u.composite.op1;
  int const op1_result = composite_who_starts(op1,is_duplex);

  slice_index const op2 = slices[si].u.composite.op2;
  int const op2_result = composite_who_starts(op2,is_duplex);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",is_duplex);

  if (op1_result==no_side && slices[op1].type==STLeaf)
    result = op2_result;
  else if (op2_result==no_side && slices[op2].type==STLeaf)
    result = op1_result;
  else if (op1_result==op2_result)
    result = op1_result;
  else
    result = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}
