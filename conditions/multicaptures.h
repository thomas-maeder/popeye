#if !defined(CONDITIONS_MULTICAPTURES_H)
#define CONDITIONS_MULTICAPTURES_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"

/* Implementation of condition MultiCaptures
 */

extern Side multicaptures_who;

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean multicaptures_initialise_check_detection(slice_index si,
                                                 Side side_in_check);

/* Continue validating an observation (or observer or observation geometry)
 * @param si identifies the slice with which to continue
 * @return true iff the observation is valid
 */
boolean multi_captures_count_checks(slice_index si);

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
