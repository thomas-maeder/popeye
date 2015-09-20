#if !defined(STIPULATION_MODIFIER_H)
#define STIPULATION_MODIFIER_H

/* Utility functions for stipulation modifiers
 */

#include "stipulation/stipulation.h"

/* Instrument the machinery with a stipulation modifier slice
 * @param si where to start the instrumentation
 * @param type slice type of the stipulation modifier slice
 */
void stipulation_modifier_instrument(slice_index si, slice_type type);

/* Notifiy all stipulation modifier slices that the stipulation has changed
 * (i.e. that they have to do their modification work)
 * @param start identifies the slice where to start notifying
 * @param stipulation_root_hook entry slice into the new stipulation
 */
void stipulation_modifiers_notify(slice_index start,
                                  slice_index stipulation_root_hook);

/* Retrieve (from a stipulation modifier slice) the entry slice into the
 * stipulation to be modified
 * @param si identifies the stipulation modifier slice
 * @return the entry slice into the stipulation to be modified
 * @note resets the information about the stipulation to be modified
 */
slice_index stipulation_modifier_to_be_modified(slice_index si);

#endif
