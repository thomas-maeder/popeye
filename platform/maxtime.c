#include "maxtime_impl.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "options/interruption.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <limits.h>

static maxtime_type maxTimeCommandLine = no_time_set;

static maxtime_type maxTimeOption = no_time_set;

/* number of seconds passed since timer started */
sig_atomic_t volatile periods_counter = 0;

/* number of seconds after which solving is aborted */
sig_atomic_t volatile nr_periods = INT_MAX;

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void setCommandlineMaxtime(maxtime_type commandlineValue)
{
  maxTimeCommandLine = commandlineValue;
}

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void resetOptionMaxtime(void)
{
  maxTimeOption = no_time_set;
}

boolean isMaxtimeSet(void)
{
  return maxTimeOption!=no_time_set || maxTimeCommandLine!=no_time_set;
}

/* Propagate our findings to STProblemSolvingInterrupted
 * @param si identifies the slice where to start instrumenting
 */
void maxtime_propagator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (hasMaxtimeElapsed())
    phase_solving_remember_interruption(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param maxtime
 */
void maxtime_instrument_solving(slice_index si, maxtime_type maxtime)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",maxtime);
  TraceFunctionParamListEnd();

  maxTimeOption = maxtime;

  {
    slice_index const interruption = branch_find_slice(STPhaseSolvingInterrupted,
                                                       si,
                                                       stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STMaxTimePropagator);
    SLICE_NEXT2(prototype) = interruption;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean dealWithMaxtime(void)
{
  boolean result;

  /* If a maximal time is indicated both on the command line and as
   * an option, use the smaller value.
   */
  if (maxTimeCommandLine==no_time_set)
    result = setMaxtimeTimer(maxTimeOption);
  else if (maxTimeOption==no_time_set)
    result = setMaxtimeTimer(maxTimeCommandLine);
  else if (maxTimeCommandLine<maxTimeOption)
    result = setMaxtimeTimer(maxTimeCommandLine);
  else
    result = setMaxtimeTimer(maxTimeOption);

  return result;
}

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean hasMaxtimeElapsed(void)
{
  return periods_counter>=nr_periods;
}
