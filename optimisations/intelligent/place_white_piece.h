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

void intelligent_place_promoted_white_rider(PieNam promotee_type,
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

void intelligent_place_white_piece(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void));

#endif
