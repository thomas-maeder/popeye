#include "conditions/bicolores.h"
#include "solving/check.h"
#include "solving/ply.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean bicolores_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  boolean result;

  if (is_in_check_recursive(slices[si].next1,side_in_check))
    result = true;
  else
  {
    trait[nbply] = side_in_check;
    result = is_in_check_recursive(slices[si].next1,side_in_check);
  }

  return result;
}

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si)
{
  solving_instrument_check_testing(si,STBicoloresCheckTester);
}
