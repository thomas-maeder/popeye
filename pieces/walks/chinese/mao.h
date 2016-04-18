#if !defined(PIECES_WALKS_CHINESE_MAO_H)
#define PIECES_WALKS_CHINESE_MAO_H

/* This module implements chinese pawns */

#include "position/board.h"
#include "solving/observation.h"

/* Determine the vector to the intermediate square given the vector to the
 * arrival square.
 */
numvec mao_intermediate_vector(numvec to_arrival);
numvec moa_intermediate_vector(numvec to_arrival);

/* Generate moves for a Mao
 */
void mao_generate_moves(void);

boolean mao_check(validator_id evaluate);

/* Generate moves for a Moa
 */
void moa_generate_moves(void);

boolean moa_check(validator_id evaluate);

/* Generate moves for a Mao Rider
 */
void maorider_generate_moves(void);

boolean maorider_check(validator_id evaluate);

/* Generate moves for a Moa Rider
 */
void moarider_generate_moves(void);

boolean moarider_check(validator_id evaluate);

/* Generate moves for a Mao Rider Lion
 */
void maoriderlion_generate_moves(void);

boolean maoriderlion_check(validator_id evaluate);

/* Generate moves for a Moa Rider Lion
 */
void moariderlion_generate_moves(void);

boolean moariderlion_check(validator_id evaluate);

#endif
