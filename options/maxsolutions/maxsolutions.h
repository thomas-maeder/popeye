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

/* Read the value of the maxsolutions option
 * @return true iff the value could be successfully read
 */
boolean read_max_solutions(char const *token);

/* Have we found the maxmimum allowed number of solutions since the
 * last invokation of reset_max_solutions()/read_max_solutions()?
 * @true iff we have found the maxmimum allowed number of solutions
 */
boolean max_solutions_reached(void);

/* Reset the number of found solutions
 */
void reset_nr_found_solutions_per_phase(void);

/* Increase the number of found solutions by 1
 */
void increase_nr_found_solutions(void);

/* Have we found the maximum allowed number of solutions since the
 * last invokation of reset_nr_found_solutions()?
 * @return true iff the allowed maximum number of solutions have been found
 */
boolean max_nr_solutions_found_in_phase(void);

/* Instrument a stipulation with STMaxSolutions*Filter slices
 * @param si identifies slice where to start
 */
void solving_insert_maxsolutions_filters(slice_index si);

#endif
