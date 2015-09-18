#if !defined(OPTIMISATION_INTELLIGENT_PIN_BLACK_PIECE_H)
#define OPTIMISATION_INTELLIGENT_PIN_BLACK_PIECE_H

#include "optimisations/intelligent/stalemate/immobilise_black.h"

/* Find out whether a black piece can be pinned
 * @param piece_pos position of piece to be pinned
 * @return direction of pin line from black king square to piece_pos
 *         0         otherwise
 */
int intelligent_is_black_piece_pinnable(square piece_pos);

/* Pin a pinnable black piece
 * @param piece_pos position of piece to be pinned
 * @param pin_dir direction of pin line from black king square via piece_pos
 * @param go_on how to go on
 * @pre pin_dir!=0
 * @pre the piece at piece_pos is pinnable along pin_dir
 */
void intelligent_pin_pinnable_black_piece(slice_index si,
                                          square piece_pos,
                                          int pin_dir,
                                          void (*go_on)(slice_index si));

/* Pin a mobile black piece
 * @param dir_to_touble_maker direction from king to piece to be pinned
 */
void intelligent_pin_black_piece(slice_index si,
                                 square position_of_trouble_maker,
                                 void (*go_on)(slice_index si));

#endif
