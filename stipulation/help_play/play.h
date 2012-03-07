#if !defined(STIPULATION_HELP_PLAY_PLAY_H)
#define STIPULATION_HELP_PLAY_PLAY_H

/* Interface for dynamically dispatching help play functions to slices
 * depending on the slice type
 */

#include "pystip.h"

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type help(slice_index si, stip_length_type n);

#endif
