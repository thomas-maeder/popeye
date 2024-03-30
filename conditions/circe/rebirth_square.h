#if !defined(CONDITIONS_CIRCE_REBIRTH_SQUARE_H)
#define CONDITIONS_CIRCE_REBIRTH_SQUARE_H

/* This module deals with the rebirth square of Circe variants in general.
 * The effective methods for determining and adapting the rebirth square(s)
 * are implemented in various other modules.
 */

#include "utilities/boolean.h"
#include "stipulation/slice_type.h"
#include "stipulation/stipulation.h"

typedef enum
{
  circe_determine_rebirth_square_from_pas,
  circe_determine_rebirth_square_frischauf,
  circe_determine_rebirth_square_symmetry,
  circe_determine_rebirth_square_vertical_symmetry,
  circe_determine_rebirth_square_horizontal_symmetry,
  circe_determine_rebirth_square_diagram,
  circe_determine_rebirth_square_pwc,
  circe_determine_rebirth_square_rank,
  circe_determine_rebirth_square_file,
  circe_determine_rebirth_square_equipollents,
  circe_determine_rebirth_square_cage,
  circe_determine_rebirth_square_antipodes,
  circe_determine_rebirth_square_super,
  circe_determine_rebirth_square_take_and_make,
  circe_determine_rebirth_square_last_capture
} circe_determine_rebirth_square_type;

typedef enum
{
  circe_rebirth_square_adapter_none,
  circe_rebirth_square_adapter_diametral,
  circe_rebirth_square_adapter_verticalmirror,
  circe_rebirth_square_adapter_rank,
  circe_rebirth_square_adapter_frischauf
} circe_rebirth_square_adapter_type;

struct circe_variant_type;

/* Instrument the solving machinery with the logic for determining the rebirth
 * square in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_rebirth_square(slice_index si,
                                             struct circe_variant_type const *variant,
                                             slice_type interval_start);

/* Override the method for determining the rebirth square of a Circe variant object
 * @param adapter the overrider
 * @return true if it hasn't been overridden yet
 */
boolean circe_override_determine_rebirth_square(struct circe_variant_type *variant,
                                                circe_determine_rebirth_square_type determine);


/* Override the rebirth square adapter of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_rebirth_square_adapter(struct circe_variant_type *variant,
                                              circe_rebirth_square_adapter_type adapter);

#endif
