#if !defined(OPTIMISATION_INTELLIGENT_PLACE_BLACK_PIECE_H)
#define OPTIMISATION_INTELLIGENT_PLACE_BLACK_PIECE_H

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/position.h"

extern square const *where_to_start_placing_black_pieces;

/* Place a black piece on an square deal with a possible check to the white king
 * @note none of these functions clean up square placed_on on returning!
 * @note none of these functions take care of reserving the placed black mass
 */

void intelligent_place_promoted_black_rider(unsigned int placed_index,
                                            piece_walk_type promotee_type,
                                            square placed_on,
                                            void (*go_on)(void));

void intelligent_place_promoted_black_knight(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void));

void intelligent_place_promoted_black_pawn(unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(void));

void intelligent_place_unpromoted_black_pawn(unsigned int placed_index,
                                             square placed_on,
                                             void (*go_on)(void));

void intelligent_place_black_rider(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void));

void intelligent_place_black_knight(unsigned int placed_index,
                                    square placed_on,
                                    void (*go_on)(void));

/* Place a black piece
 * @param placed_index index of the piece in array black[]
 * @param placed_on where to place the black piece
 * @param go_on what to do with piece placed_index on square placed_on?
 * @note will leave placed_on occupied by the last piece tried
 */
void intelligent_place_black_piece(unsigned int placed_index,
                                   square placed_on,
                                   void (*go_on)(void));

void intelligent_place_pinned_promoted_black_rider(unsigned int placed_index,
                                                   piece_walk_type promotee_type,
                                                   square placed_on,
                                                   void (*go_on)(void));

void intelligent_place_pinned_promoted_black_knight(unsigned int placed_index,
                                                    square placed_on,
                                                    void (*go_on)(void));

void intelligent_place_pinned_promoted_black_pawn(unsigned int placed_index,
                                                  square placed_on,
                                                  void (*go_on)(void));

void intelligent_place_pinned_unpromoted_black_pawn(unsigned int placed_index,
                                                    square placed_on,
                                                    void (*go_on)(void));

void intelligent_place_pinned_black_rider(unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(void));

void intelligent_place_pinned_black_knight(unsigned int placed_index,
                                           square placed_on,
                                           void (*go_on)(void));

void intelligent_place_pinned_black_piece(unsigned int placed_index,
                                          square placed_on,
                                          void (*go_on)(void));

#endif
