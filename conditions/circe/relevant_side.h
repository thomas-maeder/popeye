#if !defined(CONDITIONS_CIRCE_RELEVANT_SIDE_H)
#define CONDITIONS_CIRCE_RELEVANT_SIDE_H

/* This module deals with determining the side relevant for rebirth in Circe
 * variants
 */

#include "utilities/boolean.h"
#include "stipulation/stipulation.h"

typedef enum
{
  circe_relevant_side_overrider_none,
  circe_relevant_side_overrider_mirror,
  circe_relevant_side_overrider_rank
} circe_relevant_side_overrider_type;

struct circe_variant_type;

/* Override the relevant side overrider of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_relevant_side_overrider(struct circe_variant_type *variant,
                                               circe_relevant_side_overrider_type overrider);

/* Instrument the solving machinery with the logic for overriding the side
 * relevant for rebirth in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_relevant_side_overrider(slice_index si,
                                                      struct circe_variant_type const *variant,
                                                      slice_type interval_start);

#endif
