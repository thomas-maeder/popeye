#include "pyintslv.h"
#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>


/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 * @return true iff >= 1 solution was found
 */
boolean intelligent_solvable_root_solve_in_n(slice_index si,
                                             stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpRoot:
    case STHelpShortcut:
    case STHelpFork:
    case STHelpHashed:
    case STHelpMove:
    case STHelpMoveToGoal:
      result = help_solve_in_n(si,n)<=n;
      break;

    case STSeriesFork:
    case STSeriesShortcut:
    case STSeriesMove:
      result = series_solve_in_n(si,n)<=n;
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
