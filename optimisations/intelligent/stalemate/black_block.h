#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_BLACK_BLOCK_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_BLACK_BLOCK_H

#include "position/position.h"
#include "stipulation/stipulation.h"

/* Place black blocks for mobile pieces (not the king)
 * @param to_be_blocked one ore more squares to be blocked
 * @param nr_to_be_blocked number of elements of to_be_blocked
 */
void intelligent_stalemate_black_block(slice_index si,
                                       square const to_be_blocked[8],
                                       unsigned int nr_to_be_blocked);

#endif
