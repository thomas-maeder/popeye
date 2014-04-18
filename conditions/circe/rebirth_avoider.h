#if !defined(CONDITIONS_CIRCE_REBIRTH_AVOIDER_H)
#define CONDITIONS_CIRCE_REBIRTH_AVOIDER_H

/* Provide instrumentation of the solving machinery with Circe "rebirth avoider"
 * slices, i.e. slices that decide whether rebirth is going to take place.
 */

#include "stipulation/stipulation.h"

/* Instrument the move execution machinery with a type of "Circe rebirth
 * avoiders" (i.e. slices that may detour around Circe rebirth under a certain
 * condition; STCaptureFork is an example).
 * @param si identifies root slice of the solving machinery
 * @param interval_start start of the slices interval where to instrument
 * @param hook_type insertion is tried at each slice of this type
 * @param prototype copies of this are inserted
 * @param avoided_type type of proxy inserted on the "rebirth avoided" path
 * @param joint_type type of proxy where the two paths meet again
 * @note circe_insert_rebirth_avoider() assumes ownership of prototype
 */
void circe_insert_rebirth_avoider(slice_index si,
                                  slice_type interval_start,
                                  slice_type hook_type,
                                  slice_index prototype,
                                  slice_type avoided_type,
                                  slice_type joint_type);

#endif
