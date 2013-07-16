#if !defined(PIECES_PAWNS_COMBINED_H)
#define PIECES_PAWNS_COMBINED_H

#include "position/board.h"

/* Generate moves for a Dragon
 * @param sq_departure common departure square of the generated moves
 */
void dragon_generate_moves(square sq_departure);

/* Generate moves for a Gryphon
 * @param sq_departure common departure square of the generated moves
 */
void gryphon_generate_moves(square sq_departure);

/* Generate moves for a Ship
 * @param sq_departure common departure square of the generated moves
 */
void ship_generate_moves(square sq_departure);

#endif
