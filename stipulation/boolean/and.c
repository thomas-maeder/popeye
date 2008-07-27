#include "pyrecipr.h"
#include "pyproc.h"
#include "pydata.h"
#include "pystip.h"
#include "trace.h"

#include <assert.h>

/* Continue solving at the end of a reciprocal slice
 * @param side_at_move side at the move
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_reci_end_solve(couleur side_at_move, slice_index si)
{
  boolean found_solution = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",side_at_move);
  TraceFunctionParam("%d\n",si);
  TraceValue("%d",op1);
  TraceValue("%d\n",op2);

  if (slice_is_solvable(side_at_move,op2))
    found_solution = (slice_solve(side_at_move,op1)
                      && slice_solve(side_at_move,op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
} /* h_reci_end_solve */
