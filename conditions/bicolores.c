#include "conditions/bicolores.h"

/* Try observing with both sides
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean bicolores_try_both_sides(slice_index si, validator_id evaluate)
{
  boolean result;

  if (is_square_observed_recursive(slices[si].next1,evaluate))
    result = true;
  else
  {
    trait[nbply] = advers(trait[nbply]);
    result = is_square_observed_recursive(slices[si].next1,evaluate);
    trait[nbply] = advers(trait[nbply]);
  }

  return result;
}

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si)
{
  stip_instrument_is_square_observed_testing(si,nr_sides,STBicoloresTryBothSides);
}
