#if !defined(PIECES_WALKS_HUNTERS_H)
#define PIECES_WALKS_HUNTERS_H

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/position.h"
#include "solving/observation.h"

typedef struct
{
    PieNam home;
    PieNam away;
} HunterType;

extern HunterType huntertypes[maxnrhuntertypes];
extern unsigned int nrhuntertypes;

PieNam hunter_make_type(PieNam away, PieNam home);

void hunter_generate_moves(PieNam walk);
boolean hunter_check(evalfunction_t *evaluate);

void rook_hunter_generate_moves(void);
boolean rookhunter_check(evalfunction_t *evaluate);

void bishop_hunter_generate_moves(void);
boolean bishophunter_check(evalfunction_t *evaluate);

/* Make sure that the observer has the expected direction
 * @return true iff the observation is valid
 */
boolean hunter_enforce_observer_direction(slice_index si);

/* Initialise the solving machinery with hunter direction enforcement
 * @param root identifies the root slice of the solving machinery
 */
void solving_initialise_hunters(slice_index root);

#endif
