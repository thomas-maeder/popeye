#if !defined(STIPULATION_MOVE_INVERTER_H)
#define STIPULATION_MOVE_INVERTER_H

#include "stipulation/structure_traversal.h"

/* This module provides functionality dealing with move inverter slices
 */

/* Allocate a STMoveInverter slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_slice(void);

/* Allocate a STMoveInverterSetPlay slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_setplay_slice(void);


/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_inverter_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
