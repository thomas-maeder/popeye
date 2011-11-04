#if !defined(OPTIMISATION_INTELLIGENT_PIN_BLACK_PIECE_H)
#define OPTIMISATION_INTELLIGENT_PIN_BLACK_PIECE_H

#include "py.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"

/* Pin a mobile black piece
 * @param dir_to_touble_maker direction from king to piece to be pinned
 */
void intelligent_pin_black_piece(square position_of_trouble_maker,
                                 void (*go_on)(void));

#endif
