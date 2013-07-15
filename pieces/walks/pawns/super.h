#if !defined(PIECES_PAWNS_SUPER_H)
#define PIECES_PAWNS_SUPER_H

#include "pyproc.h"

/* Generate moves for a Super Pawn
 * @param sq_departure common departure square of the generated moves
 */
void super_pawn_generate_moves(square sq_departure);

/* Generate moves for a Super-Berolina Pawn
 * @param sq_departure common departure square of the generated moves
 */
void super_berolina_pawn_generate_moves(square sq_departure);

#endif
