#if !defined(OPTIMISATION_INTELLIGENT_PLACE_WHITE_PIECE_H)
#define OPTIMISATION_INTELLIGENT_PLACE_WHITE_PIECE_H

#include "py.h"

/* Initialise the internal structures for fast detection of guards by newly
 * placed white pieces
 * @param black_king_pos position of black king
 */
void init_guard_dirs(square black_king_pos);

/* Place a white piece (other than the king) on an square deal with a possible
 * check to the black king
 * @note none of these functions clean up square placed_on on returning!
 * @note none of these functions take care of reserving the placed white mass
 */

void intelligent_place_unpromoted_white_pawn(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void));

void intelligent_place_promoted_white_rider(piece promotee_type,
                                            unsigned int placed_index,
                                            square placed_on,
                                            void (*go_on)(void));

void intelligent_place_promoted_white_knight(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void));

void intelligent_place_promoted_white_pawn(unsigned int placed_index,
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
