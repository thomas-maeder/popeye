#if !defined(PIECES_WALKS_RADIAL_H)
#define PIECES_WALKS_RADIAL_H

/* This module implements radial pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for a radial knight
 * @param sq_departure common departure square of the generated moves
 */
void radialknight_generate_moves(void);

/* Generate moves for a radial knight
 * @param sq_departure common departure square of the generated moves
 */
void treehopper_generate_moves(void);

/* Generate moves for a radial knight
 * @param sq_departure common departure square of the generated moves
 */
void greater_treehopper_generate_moves(void);

/* Generate moves for a radial knight
 * @param sq_departure common departure square of the generated moves
 */
void leafhopper_generate_moves(void);

/* Generate moves for a radial knight
 * @param sq_departure common departure square of the generated moves
 */
void greater_leafhopper_generate_moves(void);

#endif
