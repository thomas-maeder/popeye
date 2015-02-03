#include "conditions/antikings.h"
#include "solving/check.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean antikings_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  return !is_in_check_recursive(SLICE_NEXT1(si),side_in_check);
}

/* Instrument the solving machinery with Antikings
 * @param si root slice of the solving machinery
 */
void antikings_initalise_solving(slice_index si)
{
  solving_instrument_check_testing(si,STAntikingsCheckTester);
}
