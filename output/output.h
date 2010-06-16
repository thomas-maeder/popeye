#if !defined(OUTPUT_OUTPUT_H)
#define OUTPUT_OUTPUT_H

#include "pystip.h"

/* Contains the stipulation slice that was active when the move at a
 * specific ply was played.
 */
extern slice_index active_slice[maxply];

#endif
