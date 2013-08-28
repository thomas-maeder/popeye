#if !defined(SOLVING_CHECK_H)
#define SOLVING_CHECK_H

/* This module implements the foundations of the check detection machinery */

#include "utilities/boolean.h"
#include "position/position.h"
#include "stipulation/stipulation.h"

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean is_in_check_recursive(slice_index si, Side side_in_check);

/* Determine whether a side is in check
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean is_in_check(Side a);

/* Instrument check testing with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_check_testing(slice_index si, slice_type type);

#endif
