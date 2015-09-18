#if !defined(OPTIONS_STOPONSHORTSOLUTIONS_STOPONSHORTSOLUTIONS_H)
#define OPTIONS_STOPONSHORTSOLUTIONS_STOPONSHORTSOLUTIONS_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module provides functionality dealing with the filter slices
 * implement the stoponshortsolutions option and command line parameter.
 * Slices of this type make sure that solving stops after the maximum
 * time has elapsed
 */

/* remember that a short solution has been found
 * @param si identifies the slice that remembers
 */
void short_solution_found(slice_index si);

/* Instrument the problem with a STStopOnShortSolutionsSolvingInstrumenter slice
 * @param si identifies the slice where to start instrumenting
 */
void stoponshortsolutions_problem_instrumenter_solve(slice_index si);

/* Instrument the twin with option stop on short solutions
 * @param si identifies the slice where to start instrumenting
 */
void stoponshortsolutions_instrument_twin(slice_index si);

/* Has a short solution been found in the current phase?
 */
boolean has_short_solution_been_found_in_phase(slice_index si);

/* Instrument help play
 * @param si identifies the slice where to start instrumenting
 */
void stoponshortsolutions_solving_instrumenter_solve(slice_index si);

/* Propagate our findings to the phase solving interruption machinery
 * @param si slice index
 */
void stoponshortsolutions_was_short_solution_found_solve(slice_index si);

#endif
