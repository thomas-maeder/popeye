#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_DEAL_WITH_UNUSED_PIECES_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_DEAL_WITH_UNUSED_PIECES_H

#include "py.h"

extern square const *where_to_start_placing_unused_black_pieces;

void intelligent_stalemate_deal_with_unused_pieces(stip_length_type n);

#endif
