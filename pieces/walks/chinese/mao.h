#if !defined(PIECES_WALKS_CHINESE_MAO_H)
#define PIECES_WALKS_CHINESE_MAO_H

/* This module implements chinese pawns */

#include "position/board.h"

/* Generate moves for a Mao
 * @param sq_departure common departure square of the generated moves
 */
void mao_generate_moves(square sq_departure);

/* Generate moves for a Moa
 * @param sq_departure common departure square of the generated moves
 */
void moa_generate_moves(square sq_departure);

/* Generate moves for a Moa Rider
 * @param sq_departure common departure square of the generated moves
 */
void moarider_generate_moves(square i);

/* Generate moves for a Mao Rider
 * @param sq_departure common departure square of the generated moves
 */
void maorider_generate_moves(square i);

/* Generate moves for a Mao Rider Lion
 * @param sq_departure common departure square of the generated moves
 */
void maoriderlion_generate_moves(square i);

/* Generate moves for a Moa Rider Lion
 * @param sq_departure common departure square of the generated moves
 */
void moariderlion_generate_moves(square i);

#endif
