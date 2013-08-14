#if !defined(CONDITIONS_TRANSMUTING_KINGS_TRANSMUTING_KINGS_H)
#define CONDITIONS_TRANSMUTING_KINGS_TRANSMUTING_KINGS_H

/* This module implements Transmuting Kings */

#include "position/position.h"
#include "stipulation/stipulation.h"
#include "solving/observation.h"

extern PieNam transmpieces[nr_sides][PieceCount];
extern boolean transmuting_kings_lock_recursion;

/* Initialise the sequence of king transmuters
 * @param side for which side to initialise?
 */
void init_transmuters_sequence(Side side);

/* Generate moves of a potentially transmuting king
 * @param si identifies move generator slice
 * @return true iff the king is transmuting (which doesn't necessarily mean that
 *              any moves were generated!)
 */
boolean generate_moves_of_transmuting_king(slice_index si);

/* Determine whether the moving side's king is transmuting as a specific piece
 * @param p the piece
 */
boolean is_king_transmuting_as(PieNam p, evalfunction_t *evaluate);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void transmuting_kings_generate_moves_for_piece(slice_index si, PieNam p);

/* Inialise the solving machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void transmuting_kings_initialise_solving(slice_index si, Side side);

/* Determine whether a square is observed be the side at the move according to
 * Transmuting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean transmuting_king_is_square_observed(slice_index si, evalfunction_t *evaluate);

/* Instrument the square observation machinery for a side with an alternative
 * slice dealting with observations by kings.
 * @param si identifies the root slice of the solving machinery
 * @param side side for which to instrument the square observation machinery
 * @param type type of slice to insert
 * @return the inserted slice's identifier
 * @note next2 of inserted slices will be set to the position behind the
 *       regular square observation by king handler
 */
slice_index instrument_alternative_is_square_observed_king_testing(slice_index si,
                                                            Side side,
                                                            slice_type type);

#endif
