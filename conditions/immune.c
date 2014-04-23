#include "conditions/immune.h"
#include "stipulation/pipe.h"
#include "solving/observation.h"
#include "debugging/trace.h"

circe_variant_type immune_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void immune_reset_variant(circe_variant_type *variant)
{
  circe_reset_variant(variant);

  variant->do_place_reborn = false;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict;
}

/* Initialise solving in Immune Chess
 * @param si identifies the root slice of the stipulation
 */
void immune_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  circe_initialise_solving(si,&immune_variant,STMove,STGenevaConsideringRebirth);

  stip_instrument_check_validation(si,
                                   nr_sides,
                                   STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
