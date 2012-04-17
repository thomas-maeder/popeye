#if !defined(PIECES_KAMIKAZE_H)
#define PIECES_KAMIKAZE_H

#include "stipulation/slice.h"

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_kamikaze_goal_filters(slice_index si);

#endif
