#if !defined(OPTIONS_QUODLIBET_H)
#define OPTIONS_QUODLIBET_H

#include "stipulation/stipulation.h"

/* Transform a stipulation tree to a quodlibet.
 * @param si identifies slice where to start
 */
void quodlibet_stipulation_modifier_solve(slice_index si);

#endif
