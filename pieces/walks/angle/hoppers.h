#if !defined(PIECES_ANGLE_HOPPERS_H)
#define PIECES_ANGLE_HOPPERS_H

#include "pieces/walks/angle/angles.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "pyproc.h"

/* Is a particular square observed by a particular type of angle hopper?
 * @param sq_target the square
 * @param kanf first vectors index (departure square to hurdle)
 * @param kend last vectors index (departure square to hurdle)
 * @param angle angle to take from hurdle to arrival squares
 * @param p type of piece
 */
boolean angle_hoppers_is_square_observed(square sq_target,
                                         vec_index_type kanf, vec_index_type kend,
                                         angle_t angle,
                                         PieNam p,
                                         evalfunction_t *evaluate);

/* Generated moves for an Elk
 * @param sq_departure departure square of moves to be generated
 */
void elk_generate_moves(square sq_departure);

/* Generated moves for an Rook Moose
 * @param sq_departure departure square of moves to be generated
 */
void rook_moose_generate_moves(square sq_departure);

/* Generated moves for an Bishop Moose
 * @param sq_departure departure square of moves to be generated
 */
void bishop_moose_generate_moves(square sq_departure);

/* Generated moves for an Eagle
 * @param sq_departure departure square of moves to be generated
 */
void eagle_generate_moves(square sq_departure);

/* Generated moves for a Rook Eagle
 * @param sq_departure departure square of moves to be generated
 */
void rook_eagle_generate_moves(square sq_departure);

/* Generated moves for a Bishop Eagle
 * @param sq_departure departure square of moves to be generated
 */
void bishop_eagle_generate_moves(square sq_departure);

/* Generated moves for a Sparrow
 * @param sq_departure departure square of moves to be generated
 */
void sparrow_generate_moves(square sq_departure);

/* Generated moves for a Rook Sparrow
 * @param sq_departure departure square of moves to be generated
 */
void rook_sparrow_generate_moves(square sq_departure);

/* Generated moves for a Bishop Sparrow
 * @param sq_departure departure square of moves to be generated
 */
void bishop_sparrow_generate_moves(square sq_departure);

/* Generated moves for a Marguerite
 * @param sq_departure departure square of moves to be generated
 */
void marguerite_generate_moves(square sq_departure);

#endif
