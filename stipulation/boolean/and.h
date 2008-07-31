#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "boolean.h"
#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with reciprocal
 * (i.e. logical AND) stipulation slices.
 */

/* Continue solving at the end of a reciprocal slice
 * @param side_at_move side at the move
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_reci_end_solve(Side side_at_move, slice_index si);

/* Attempt to deremine which side is at the move
 * at the start of a slice.
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @return one of blanc, noir, no_side (the latter if we can't
 *         determine which side is at the move)
 */
Side reci_who_starts(slice_index si, boolean is_duplex);

#endif
