#include "conditions/bicolores.h"
#include "solving/machinery/solve.h"

/* Try observing with both sides
 * @param si identifies next slice
 * @note sets observation_result
 */
void bicolores_try_both_sides(slice_index si)
{
  is_square_observed_recursive(SLICE_NEXT1(si));

  if (!observation_result)
  {
    trait[nbply] = advers(trait[nbply]);
    is_square_observed_recursive(SLICE_NEXT1(si));
    trait[nbply] = advers(trait[nbply]);
  }
}

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si)
{
  stip_instrument_is_square_observed_testing(si,nr_sides,STBicoloresTryBothSides);
}
