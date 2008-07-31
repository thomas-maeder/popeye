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

/* Intialize starter field with the starting side if possible, and
 * no_side otherwise. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void reci_init_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",is_duplex);

  composite_init_starter(op1,is_duplex);
  composite_init_starter(op2,is_duplex);

  slices[si].starter = no_side;

  if (slices[op1].starter==no_side && slices[op1].type==STLeaf)
  {
    /* op1 can't tell - let's tell him */
    slices[si].starter = slices[op2].starter;
    slices[op1].starter = slices[op2].starter;
  }
  else if (slices[op2].starter==no_side && slices[op2].type==STLeaf)
  {
    /* op2 can't tell - let's tell him */
    slices[si].starter = slices[op1].starter;
    slices[op2].starter = slices[op1].starter;
  }
  else if (slices[op1].starter==slices[op2].starter)
    slices[si].starter = slices[op1].starter;

  TraceValue("%d\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}
