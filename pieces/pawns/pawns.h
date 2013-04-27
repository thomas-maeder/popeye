#if !defined(PIECES_PAWNS_PAWNS_H)
#define PIECES_PAWNS_PAWNS_H

#include "position/board.h"
#include "position/position.h"
#include "py.h"

/* generates move of a pawn of side camp on departure capturing a piece on
 * arrival
 */
void pawns_generate_capture_move(Side camp,
                                 square sq_departure,
                                 square sq_arrival);

/* generates moves of a pawn in direction dir where steps single steps are
 * possible.
 */
void pawns_generate_nocapture_moves(square sq_departure, numvec dir, int steps);

#endif
