#if !defined(CONDITIONS_TRANSMUTING_KINGS_TRANSMUTING_KINGS_H)
#define CONDITIONS_TRANSMUTING_KINGS_TRANSMUTING_KINGS_H

/* This module implements Transmuting Kings */

#include "pieces/pieces.h"
#include "position/position.h"
#include "stipulation/stipulation.h"
#include "solving/observation.h"

extern PieNam transmuting_kings_potential_transmutations[nr_sides][PieceCount];
extern boolean transmuting_kings_testing_transmutation[nr_sides];

/* Initialise the sequence of king transmuters
 * @param side for which side to initialise?
 */
void transmuting_kings_init_transmuters_sequence(Side side);

/* Generate moves of a potentially transmuting king
 * @param si identifies move generator slice
 * @return true iff the king is transmuting (which doesn't necessarily mean that
 *              any moves were generated!)
 */
boolean generate_moves_of_transmuting_king(slice_index si);

/* Determine whether the moving side's king is transmuting as a specific walk
 * @param p the piece
 */
boolean transmuting_kings_is_king_transmuting_as(PieNam walk);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void transmuting_kings_generate_moves_for_piece(slice_index si, PieNam p);

/* Inialise the observation machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 * @note invoked by transmuting_kings_initialise_observing()
 */
void transmuting_kings_initialise_observing(slice_index si, Side side);

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
boolean transmuting_king_is_square_observed(slice_index si, validator_id evaluate);

/* Find out if the royal piece is not transmuted (i.e. moves according to its
 * original walk)
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean transmuting_king_detect_non_transmutation(slice_index si, validator_id evaluate);

/* Make sure to behave correctly while detecting observations by
 * transmuting kings
 */
boolean transmuting_kings_enforce_observer_walk(slice_index si);

#endif
