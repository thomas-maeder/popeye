#include "options/movenumbers/restart_guard_intelligent.h"
#include "pydata.h"
#include "pymovenb.h"
#include "pypipe.h"
#include "pymsg.h"
#include "trace.h"
#include "optimisations/intelligent/moves_left.h"
#include "platform/maxtime.h"

#include <assert.h>

static unsigned long nr_potential_target_positions;

/* Allocate a STRestartGuardIntelligent slice.
 * @return allocated slice
 */
slice_index alloc_restart_guard_intelligent(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuardIntelligent);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_length_ruled_out_by_option_restart(stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (OptFlag[restart])
  {
    stip_length_type min_length = 2*get_restart_number();
    if ((n-slack_length_help)%2==1)
      --min_length;
    result = n-slack_length_help<min_length;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void print_nr_potential_target_positions(void)
{
  StdString("\n");
  sprintf(GlobalStr,"%lu %s %u+%u",
          nr_potential_target_positions,GetMsgString(PotentialMates),
          MovesLeft[White],MovesLeft[Black]);
  StdString(GlobalStr);
  if (!flag_regression)
  {
    StdString("  (");
    PrintTime();
    StdString(")");
  }
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_intelligent_help(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_length_ruled_out_by_option_restart(n))
    result = n+2;
  else
  {
    nr_potential_target_positions = 0;
    result = help(slices[si].u.pipe.next,n);
    if (!hasMaxtimeElapsed())
      print_nr_potential_target_positions();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Allocate a STIntelligentTargetCounter slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_target_counter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentTargetCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type intelligent_target_counter_help(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  ++nr_potential_target_positions;
  TraceValue("%u\n",nr_potential_target_positions);

  result = help(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
