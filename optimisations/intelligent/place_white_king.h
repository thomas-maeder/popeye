#if !defined(OPTIMISATION_INTELLIGENT_PLACE_WHITE_KING_H)
#define OPTIMISATION_INTELLIGENT_PLACE_WHITE_KING_H

#include "py.h"

/* Place the white king; intercept checks if necessary
 * @param place_on where to place the king
 * @param go_on what to do after having placed the king?
 */
void intelligent_place_white_king(square place_on, void (*go_on)(void));

#endif
