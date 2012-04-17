#if !defined(OPTIONS_QUODLIBET_H)
#define OPTIONS_QUODLIBET_H

#include "stipulation/slice.h"

/* Transform a stipulation tree to a quodlibet.
 * @param si identifies slice where to start
 * @return true iff the stipulation could be transformed
 */
boolean transform_to_quodlibet(slice_index si);

#endif
