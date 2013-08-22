#if !defined(CONDITIONS_MARSCIRCE_MARSCIRCE_H)
#define CONDITIONS_MARSCIRCE_MARSCIRCE_H

#include "solving/solve.h"
#include "solving/observation.h"
#include "position/position.h"

/* This module provides implements the condition Mars-Circe
 */

extern square (*marscirce_determine_rebirth_square)(PieNam, Flags, square, square, square, Side);

/* Generate non-capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 */
void marscirce_generate_non_captures(slice_index si,
                                     PieNam p,
                                     square sq_generate_from);

/* Generate capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 */
void marscirce_generate_captures(slice_index si,
                                 PieNam p,
                                 square sq_generate_from);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_moves_for_piece(slice_index si, PieNam p);

/* Validate an observation or observer by making sure it's the one that has just
 * been reborn
 * @param si identifies the validator slice
 */
boolean mars_enforce_observer(slice_index si);

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param pos_checking potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_is_square_observed_by(square pos_checking,
                                   square sq_rebirth,
                                   evalfunction_t *evaluate);

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @return true iff side is in check
 */
boolean marscirce_is_square_observed(slice_index si,
                                     evalfunction_t *evaluate);

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_marscirce(slice_index si);

#endif
