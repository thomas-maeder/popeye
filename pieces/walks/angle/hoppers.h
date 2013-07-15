#if !defined(PIECES_ANGLE_HOPPERS_H)
#define PIECES_ANGLE_HOPPERS_H

#include "pieces/walks/angle/angles.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "pyproc.h"

/* Generated moves for an angle hopper
 * @param sq_departure departure square of moves to be generated
 * @param kanf first vectors index (departure square to hurdle)
 * @param kend last vectors index (departure square to hurdle)
 * @param angle angle to take from hurdle to arrival squares
 * @param side side for which to generate moves
 */
void angle_hoppers_generate_moves(square sq_departure,
                                  vec_index_type kanf, vec_index_type kend,
                                  angle_t angle);

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

#endif
