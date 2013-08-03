#if !defined(PIECES_WALKS_HUNTERS_H)
#define PIECES_WALKS_HUNTERS_H

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

void hunter_generate_moves(PieNam walk);

void rook_hunter_generate_moves(void);
void bishop_hunter_generate_moves(void);

#endif
