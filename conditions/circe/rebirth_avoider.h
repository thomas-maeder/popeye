#if !defined(CONDITIONS_CIRCE_REBIRTH_AVOIDER_H)
#define CONDITIONS_CIRCE_REBIRTH_AVOIDER_H

/* Provide instrumentation of the solving machinery with Circe "rebirth avoider"
 * slices, i.e. slices that decide whether rebirth is going to take place.
 */

#include "stipulation/stipulation.h"

/* Instrument a stipulation with a type of "Circe rebirth avoiders" (i.e.
 * slices that may detour around Circe rebirth under a certain condition;
 * STCaptureFork is an example).
 * @param si identifies root slice of stipulation
 * @param type type of Circe rebirth avoider
 * @param type type of proxy inserted on the "rebirth avoided" path
 */
void stip_insert_rebirth_avoider(slice_index si,
                                 slice_type type,
                                 slice_type avoided_type);

#endif
