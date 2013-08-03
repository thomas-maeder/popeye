#if !defined(PIECES_ANGLE_HOPPERS_H)
#define PIECES_ANGLE_HOPPERS_H

#include "pieces/walks/angle/angles.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "pyproc.h"

/* Is a particular square observed by a particular type of angle hopper?
 * @param kanf first vectors index (departure square to hurdle)
 * @param kend last vectors index (departure square to hurdle)
 * @param angle angle to take from hurdle to arrival squares
 * @param p type of piece
 */
boolean angle_hoppers_is_square_observed(vec_index_type kanf, vec_index_type kend,
                                         angle_t angle,
                                         PieNam p,
                                         evalfunction_t *evaluate);

/* Generated moves for an Elk
 */
void elk_generate_moves(void);

/* Generated moves for an Rook Moose
 */
void rook_moose_generate_moves(void);

/* Generated moves for an Bishop Moose
 */
void bishop_moose_generate_moves(void);

/* Generated moves for an Eagle
 */
void eagle_generate_moves(void);

/* Generated moves for a Rook Eagle
 */
void rook_eagle_generate_moves(void);

/* Generated moves for a Bishop Eagle
 */
void bishop_eagle_generate_moves(void);

/* Generated moves for a Sparrow
 */
void sparrow_generate_moves(void);

/* Generated moves for a Rook Sparrow
 */
void rook_sparrow_generate_moves(void);

/* Generated moves for a Bishop Sparrow
 */
void bishop_sparrow_generate_moves(void);

/* Generated moves for a Marguerite
 */
void marguerite_generate_moves(void);

#endif
