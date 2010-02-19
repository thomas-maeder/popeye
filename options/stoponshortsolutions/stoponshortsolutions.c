#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "trace.h"

static boolean short_solution_found_in_problem;

/* Inform the stoponshortsolutions module that a short solution has
 * been found
 */
void short_solution_found(void)
{
  short_solution_found_in_problem = true;
}

/* Reset the internal state to "no short solution found"
 */
void reset_short_solution_found_in_problem(void)
{
  short_solution_found_in_problem = false;
}

/* Has a short solution been found in the current problem?
 */
boolean has_short_solution_been_found_in_problem(void)
{
  return short_solution_found_in_problem;
}
