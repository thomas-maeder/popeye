#if !defined(PIECES_WALKS_CLASSIFICATION_H)
#define PIECES_WALKS_CLASSIFICATION_H

#include "pieces/pieces.h"
#include "position/position.h"
#include "utilities/boolean.h"

boolean is_king(PieNam p);
boolean is_pawn(PieNam p);
boolean is_forwardpawn(PieNam p);
boolean is_locust(PieNam p);
boolean is_leaper(PieNam p);
boolean is_rider(PieNam p);
boolean is_chineserider(PieNam p);
boolean is_lion(PieNam p);
boolean is_simpledecomposedleaper(PieNam p);
boolean is_symmetricfairy(PieNam p);
boolean is_simplehopper(PieNam p);

#endif
