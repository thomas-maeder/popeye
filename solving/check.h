#if !defined(SOLVING_CHECK_H)
#define SOLVING_CHECK_H

/* This module implements the foundations of the check detection machinery */

#include "utilities/boolean.h"
#include "position/side.h"
#include "stipulation/stipulation.h"
#include "debugging/measure.h"

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

/* Tell the check detection machinery that a side may have no king */
void check_no_king_is_possible(void);

/* Tell the check detection machinery that a side may be in check even if it
 * doesn't have a king*/
void check_even_if_no_king(void);

/* Optimise the check machinery if possible
 * @param si identifies the root slice of the solving machinery
 */
void optimise_is_in_check(slice_index si);

/* Instrument the solving machinery so as to test for check playing moves
 * forward rather than tracking back from the king square
 * @param si identifies the root slice of the solving machinery
 */
void solving_test_check_playing_moves(slice_index si);

DECLARE_COUNTER(is_white_king_square_attacked);
DECLARE_COUNTER(is_black_king_square_attacked);

#endif
