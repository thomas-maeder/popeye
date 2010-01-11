#include "pyintslv.h"
#include "pybrah.h"
#include "pybraser.h"
#include "pyquodli.h"
#include "pymovein.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>


/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void intelligent_solvable_root_solve_in_n(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpRoot:
      help_root_solve_in_n(si,n);
      break;

    case STSeriesRoot:
      series_root_solve_in_n(si,n);
      break;

    case STBranchSeries:
    case STSeriesHashed:
    case STBranchFork:
      series_solve_in_n(si,n);
      break;

    case STQuodlibet:
      quodlibet_root_solve_in_n(si,n);
      break;

    case STMoveInverter:
      move_inverter_root_solve_in_n(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
