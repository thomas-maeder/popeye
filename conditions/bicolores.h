#if !defined(CONDITIONS_BICOLORES_H)
#define CONDITIONS_BICOLORES_H

/* This module implements Bicolores */

#include "stipulation/stipulation.h"
#include "solving/observation.h"

/* Try observing with both sides
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean bicolores_try_both_sides(slice_index si);

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si);

#endif
