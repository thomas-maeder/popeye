#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_UNINTERCEPTABLE_CHECK_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_UNINTERCEPTABLE_CHECK_H

#include "position/position.h"

vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                              vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk);

vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked);

vec_index_type is_square_attacked_by_uninterceptable(Side side_under_attack, square sq_attacked);

#endif
