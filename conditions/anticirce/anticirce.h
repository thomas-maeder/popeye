#if !defined(CONDITIONS_ANTICIRCE_H)
#define CONDITIONS_ANTICIRCE_H

#include "pystip.h"

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_anticirce_goal_filters(slice_index si);

#endif
