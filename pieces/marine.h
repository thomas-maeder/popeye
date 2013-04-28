#if !defined(PIECES_MARINE_H)
#define PIECES_MARINE_H

#include "position/board.h"
#include "position/position.h"
#include "py.h"

/* Generate moves for a marine rider
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_rider_generate_moves(Side side,
                                 square sq_departure,
                                 numvec kbeg, numvec kend);

/* Generate moves for a marine knight
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void marine_knight_generate_moves(Side side, square sq_departure);

/* Generate moves for a poseidon
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void poseidon_generate_moves(Side side, square sq_departure);

/* Generate moves for a marine pawn
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void marine_pawn_generate_moves(Side side, square sq_departure);

/* Generate moves for a marine ship (Triton + marine pawn)
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_ship_generate_moves(Side side,
                                square sq_departure,
                                numvec kbeg, numvec  kend);

#endif
