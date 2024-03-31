#if !defined(CONDITIONS_MULTICAPTURES_H)
#define CONDITIONS_MULTICAPTURES_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"

/* Implementation of condition MultiCaptures
 */

extern boolean multicaptures_who[nr_sides];

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean multicaptures_check_test_initialize(slice_index si,
                                            Side side_attacked);

/* Filter out single captures
 * @param identifies generator slice
 */
void multicaptures_filter_singlecaptures(slice_index si);

/* Inialise the solving machinery with MultiCaptures
 * @param si identifies root slice of solving machinery
 * @param side for who - pass nr_sides for both sides
 */
void multicaptures_initialise_solving(slice_index si);

#endif
