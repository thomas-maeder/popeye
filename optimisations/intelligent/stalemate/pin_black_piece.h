#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_PIN_BLACK_PIECE_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_PIN_BLACK_PIECE_H

#include "position/board.h"
#include "stipulation/stipulation.h"

/* Pin a mobile black piece
 * @param position_of_trouble_maker position of piece to be pinned
 */
void intelligent_stalemate_pin_black_piece(slice_index si,
                                           square position_of_trouble_maker);

#endif
