#include "pyintslv.h"
#include "pyhelp.h"
#include "stipulation/series_play/play.h"
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
    case STHelpShortcut:
    case STBranchHelp:
    case STHelpHashed:
    case STHelpFork:
      help_solve_in_n(si,n);
      break;

    case STSeriesShortcut:
    case STSeriesMove:
    case STSeriesHashed:
    case STSeriesFork:
      series_solve_in_n(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
