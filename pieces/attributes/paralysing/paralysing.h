#if !defined(PIECES_PARALYSING_H)
#define PIECES_PARALYSING_H

#include "pystip.h"

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_paralysing_goal_filters(slice_index si);

#endif
