#if !defined(PIECES_WALKS_MARINE_H)
#define PIECES_WALKS_MARINE_H

#include "pieces/walks/vectors.h"
#include "position/board.h"
#include "position/position.h"
#include "py.h"

/* Generate moves for a marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_rider_generate_moves(vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a marine knight
 */
void marine_knight_generate_moves(void);

/* Generate moves for a poseidon
 */
void poseidon_generate_moves(void);

/* Generate moves for a marine pawn
 */
void marine_pawn_generate_moves(void);

/* Generate moves for a marine ship (Triton + marine pawn)
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_ship_generate_moves(vec_index_type kbeg, vec_index_type  kend);

#endif
