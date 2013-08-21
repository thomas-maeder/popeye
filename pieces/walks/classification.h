#if !defined(PIECES_WALKS_CLASSIFICATION_H)
#define PIECES_WALKS_CLASSIFICATION_H

#include "position/position.h"
#include "utilities/boolean.h"

boolean is_king(PieNam p);
boolean is_pawn(PieNam p);
boolean is_forwardpawn(PieNam p);
boolean is_locust(PieNam p);

#endif
