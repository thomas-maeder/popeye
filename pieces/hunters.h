#if !defined(PIECES_HUNTERS_H)
#define PIECES_HUNTERS_H

#include "position/board.h"
#include "position/position.h"
#include "py.h"

typedef struct
{
    PieNam home;
    PieNam away;
} HunterType;

extern HunterType huntertypes[maxnrhuntertypes];
extern unsigned int nrhuntertypes;

PieNam hunter_make_type(PieNam away, PieNam home);

void hunter_generate_moves(square sq_departure, PieNam walk);

void rook_hunter_generate_moves(square sq_departure);
void bishop_hunter_generate_moves(square sq_departure);

#endif
