#if !defined(OPTIMISATIONS_MAXSOLUTIONS_MAXSOLUTIONS_H)
#define OPTIMISATIONS_MAXSOLUTIONS_MAXSOLUTIONS_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module provides functionality dealing with the filter slices
 * implement the maxsolutions option
 */

/* Reset the value of the maxsolutions option
 */
void maxsolutions_resetter_solve(slice_index si);

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 */
void maxsolutions_solving_instrumenter_solve(slice_index si);

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 */
void maxsolutions_problem_instrumenter_solve(slice_index si);

/* Instrument the current problem with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param max_nr_solutions_per_phase
 */
void maxsolutions_instrument_problem(slice_index si,
                                     unsigned int max_nr_solutions_per_phase);

/* Reset the number of found solutions
 */
void reset_nr_found_solutions_per_phase(void);

/* Increase the number of found solutions by 1
 */
void increase_nr_found_solutions(slice_index interruption);

/* Have we found the maximum allowed number of solutions since the
 * last invokation of reset_nr_found_solutions()?
 * @return true iff the allowed maximum number of solutions have been found
 */
boolean max_nr_solutions_found_in_phase(void);

#endif
