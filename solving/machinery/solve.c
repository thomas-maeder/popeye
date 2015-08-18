#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#include "debugging/assert.h"

stip_length_type solve_nr_remaining = length_unspecified;
stip_length_type solve_result;

/* Detect whether solve_result indicates that solving has succeeded
 * @return true iff solving has succeeded
 */
boolean move_has_solved(void)
{
  return slack_length<=solve_result && solve_result<=MOVE_HAS_SOLVED_LENGTH();
}
