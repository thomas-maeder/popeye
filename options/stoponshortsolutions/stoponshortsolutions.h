#if !defined(OPTIMISATIONS_STOPONSHORTSOLUTIONS_STOPONSHORTSOLUTIONS_H)
#define OPTIMISATIONS_STOPONSHORTSOLUTIONS_STOPONSHORTSOLUTIONS_H

#include "boolean.h"

/* This module provides functionality dealing with the filter slices
 * implement the stoponshortsolutions option and command line parameter. 
 * Slices of this type make sure that solving stops after the maximum
 * time has elapsed
 */

/* Inform the stoponshortsolutions module that a short solution has
 * been found
 */
void short_solution_found(void);

/* Reset the internal state to "no short solution found"
 */
void reset_short_solution_found_in_problem(void);

/* Instrument a stipulation with STStopOnShortSolutions*Filter slices
 */
void stip_insert_stoponshortsolutions_filters(void);

/* Has a short solution been found in the current problem?
 */
boolean has_short_solution_been_found_in_problem(void);

#endif
