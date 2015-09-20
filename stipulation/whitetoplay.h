#if !defined(STIPULATION_WHITETOPLAY_H)
#define STIPULATION_WHITETOPLAY_H

/* This module implements the option whitetoplay
 */

#include "stipulation/stipulation.h"

/* Apply the option whitetoplay to a recently constructed stipulation
 * @param si identifies the whitetoplay applier slice
 */
void white_to_play_stipulation_modifier_solve(slice_index si);

#endif
