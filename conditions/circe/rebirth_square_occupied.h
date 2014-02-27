#if !defined(CONDITIONS_CIRCE_REBIRTH_SQUARE_OCCUPIED_H)
#define CONDITIONS_CIRCE_REBIRTH_SQUARE_OCCUPIED_H

/* This module supports dealing with the situation when a Circe rebirth square
 * is occupied
 */

#include "stipulation/stipulation.h"

/* Cause moves with Circe rebirth on an occupied square to be played without
 * rebirth
 * @param si entry slice into the solving machinery
 */
void circe_no_rebirth_on_occupied_square(slice_index si);

/* Cause moves with Circe rebirth on an occupied square to not be played
 * @param si entry slice into the solving machinery
 */
void circe_stop_rebirth_on_occupied_square(slice_index si);

/* Cause moves with Circe rebirth on an occupied square to assassinate
 * @param si entry slice into the solving machinery
 */
void circe_assassinate_on_occupied_square(slice_index si);

#endif
