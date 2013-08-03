#if !defined(PIECES_WALKS_CHINESE_MAO_H)
#define PIECES_WALKS_CHINESE_MAO_H

/* This module implements chinese pawns */

#include "position/board.h"

/* Generate moves for a Mao
 */
void mao_generate_moves(void);

/* Generate moves for a Moa
 */
void moa_generate_moves(void);

/* Generate moves for a Moa Rider
 */
void moarider_generate_moves(void);

/* Generate moves for a Mao Rider
 */
void maorider_generate_moves(void);

/* Generate moves for a Mao Rider Lion
 */
void maoriderlion_generate_moves(void);

/* Generate moves for a Moa Rider Lion
 */
void moariderlion_generate_moves(void);

#endif
