#if !defined(PIECES_WALKS_CLASSIFICATION_H)
#define PIECES_WALKS_CLASSIFICATION_H

#include "pieces/pieces.h"
#include "position/position.h"
#include "utilities/boolean.h"

boolean is_king(piece_walk_type p);
boolean is_pawn(piece_walk_type p);
boolean is_forwardpawn(piece_walk_type p);
boolean is_locust(piece_walk_type p);
boolean is_leaper(piece_walk_type p);
boolean is_rider(piece_walk_type p);
boolean is_chineserider(piece_walk_type p);
boolean is_lion(piece_walk_type p);
boolean is_simpledecomposedleaper(piece_walk_type p);
boolean is_symmetricfairy(piece_walk_type p);
boolean is_simplehopper(piece_walk_type p);

#endif
