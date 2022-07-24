#if !defined(PIECES_WALKS_LEAPERS_H)
#define PIECES_WALKS_LEAPERS_H

/* This module implements leaper pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generate moves for a leaper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean leapers_check(vec_index_type b, vec_index_type c, validator_id evaluate);

boolean king_check(validator_id evaluate);
boolean knight_check(validator_id evaluate);
boolean alfil_check(validator_id evaluate);
boolean antilope_check(validator_id evaluate);
boolean bison_check(validator_id evaluate);
boolean bucephale_check(validator_id evaluate);
boolean okapi_check(validator_id evaluate);
boolean camel_check(validator_id evaluate);
boolean dabbaba_check(validator_id evaluate);
boolean fers_check(validator_id evaluate);
boolean girafe_check(validator_id evaluate);
boolean gnu_check(validator_id evaluate);
boolean leap15_check(validator_id evaluate);
boolean leap16_check(validator_id evaluate);
boolean leap24_check(validator_id evaluate);
boolean leap25_check(validator_id evaluate);
boolean leap35_check(validator_id evaluate);
boolean leap36_check(validator_id evaluate);
boolean leap37_check(validator_id evaluate);
boolean rccinq_check(validator_id evaluate);
boolean squirrel_check(validator_id evaluate);
boolean vizir_check(validator_id evaluate);
boolean zebra_check(validator_id evaluate);
boolean zebu_check(validator_id evaluate);

#endif
