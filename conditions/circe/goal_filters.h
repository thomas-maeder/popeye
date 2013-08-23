#if !defined(CONDITIONS_CIRCE_H)
#define CONDITIONS_CIRCE_H

#include "stipulation/stipulation.h"

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_circe_goal_filters(slice_index si);

#endif
