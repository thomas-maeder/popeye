#if !defined(OPTIMISATION_INTELLIGENT_PLACE_WHITE_KING_H)
#define OPTIMISATION_INTELLIGENT_PLACE_WHITE_KING_H

#include "position/position.h"
#include "stipulation/stipulation.h"

/* Place the white king; intercept checks if necessary
 * @param place_on where to place the king
 * @param go_on what to do after having placed the king?
 */
void intelligent_place_white_king(slice_index si,
                                  square place_on,
                                  void (*go_on)(slice_index si));

#endif
