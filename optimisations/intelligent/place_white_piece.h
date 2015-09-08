#if !defined(OPTIMISATION_INTELLIGENT_PLACE_WHITE_PIECE_H)
#define OPTIMISATION_INTELLIGENT_PLACE_WHITE_PIECE_H

#include "pieces/pieces.h"
#include "position/position.h"

/* Place a white piece (other than the king) on an square deal with a possible
 * check to the black king
 * @note none of these functions clean up square placed_on on returning!
 * @note none of these functions take care of reserving the placed white mass
 */

void intelligent_place_unpromoted_white_pawn(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void));

void intelligent_place_promoted_white_rider(piece_walk_type promotee_type,
                                            unsigned int placed_index,
                                            square placed_on,
                                            void (*go_on)(void));

void intelligent_place_promoted_white_knight(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void));

void intelligent_place_promoted_white_pawn(unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(void));

void intelligent_place_white_queen(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void));

void intelligent_place_white_rider(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void));

void intelligent_place_white_knight(unsigned int placed_index,
                                    square placed_on,
                                    void (*go_on)(void));

/* Place a white piece
 * @param placed_index index of the piece in array white[]
 * @param placed_on where to place the white piece
 * @param go_on what to do with piece placed_index on square placed_on?
 * @note will leave placed_on occupied by the last piece tried
 */
void intelligent_place_white_piece(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void));

#endif
