#if !defined(CONDITIONS_BICOLORES_H)
#define CONDITIONS_BICOLORES_H

/* This module implements Bicolores */

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"
#include "position/position.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean bicolores_check_tester_is_in_check(slice_index si, Side side_in_check);

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si);

#endif
