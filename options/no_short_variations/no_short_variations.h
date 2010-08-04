#if !defined(OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_H)
#define OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_H

#include "pystip.h"

/* This module provides functionality dealing with the filter slices
 * implement the noshortvariations option 
 */

/* Instrument a stipulation with STNoShortVariations slices
 * @param si identifies slice where to start
 */
void stip_insert_no_short_variations_filters(slice_index si);

#endif
