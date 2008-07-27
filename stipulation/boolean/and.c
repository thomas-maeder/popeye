#include "pyrecipr.h"
#include "pyproc.h"
#include "pyleaf.h"
#include "pyint.h"
#include "pydata.h"
#include "trace.h"
#include "platform/maxtime.h"

/* Continue solving at the end of a reciprocal slice
 * @param side_at_move side at the move
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_reci_end_solve(couleur side_at_move, slice_index si)
{
  couleur other_side = advers(side_at_move);
  boolean found_solution = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d",op1);
  TraceFunctionParam("%d\n",op2);

  if (leaf_is_end_in_1_possible(side_at_move,op2))
  {
    genmove(side_at_move);

    if (side_at_move==blanc)
      WhMovesLeft--;
    else
      BlMovesLeft--;

    while (encore())
    {
      TraceCurrentMove();
      if (jouecoup()
          && !echecc(side_at_move)
          && h_leaf_h_solve_ending_move(other_side,op1))
        found_solution = true;

      repcoup();

      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    if (side_at_move==blanc)
      WhMovesLeft++;
    else
      BlMovesLeft++;

    finply();

    if (found_solution)
      h_leaf_h_solve_ending_move(side_at_move,op2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",found_solution);
  return found_solution;
} /* h_reci_end_solve */
