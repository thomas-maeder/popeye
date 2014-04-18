#include "conditions/circe/relevant_side.h"
#include "conditions/circe/circe.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Override the relevant side overrider of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_relevant_side_overrider(struct circe_variant_type *variant,
                                               circe_relevant_side_overrider_type overrider)
{
  boolean result;

  if (variant->relevant_side_overrider==circe_relevant_side_overrider_none)
  {
    variant->relevant_side_overrider = overrider;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Instrument the solving machinery with the logic for overriding the side
 * relevant for rebirth in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_relevant_side_overrider(slice_index si,
                                                      struct circe_variant_type const *variant,
                                                      slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",interval_start);
  TraceFunctionParamListEnd();

  switch (variant->relevant_side_overrider)
  {
    case circe_relevant_side_overrider_none:
      break;

    case circe_relevant_side_overrider_mirror:
      circe_instrument_solving(si,
                               interval_start,
                               STCirceDeterminingRebirth,
                               alloc_pipe(STMirrorCirceOverrideRelevantSide));
      break;

    case circe_relevant_side_overrider_rank:
      circe_instrument_solving(si,
                               interval_start,
                               STCirceDeterminingRebirth,
                               alloc_pipe(STRankCirceOverrideRelevantSide));
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
