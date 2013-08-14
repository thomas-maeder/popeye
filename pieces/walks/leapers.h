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

boolean leapers_check(vec_index_type b, vec_index_type c, evalfunction_t *evaluate);

boolean king_check(evalfunction_t *evaluate);
boolean knight_check(evalfunction_t *evaluate);
boolean alfil_check(evalfunction_t *evaluate);
boolean antilope_check(evalfunction_t *evaluate);
boolean bison_check(evalfunction_t *evaluate);
boolean bucephale_check(evalfunction_t *evaluate);
boolean okapi_check(evalfunction_t *evaluate);
boolean camel_check(evalfunction_t *evaluate);
boolean dabbaba_check(evalfunction_t *evaluate);
boolean fers_check(evalfunction_t *evaluate);
boolean girafe_check(evalfunction_t *evaluate);
boolean gnu_check(evalfunction_t *evaluate);
boolean leap15_check(evalfunction_t *evaluate);
boolean leap16_check(evalfunction_t *evaluate);
boolean leap24_check(evalfunction_t *evaluate);
boolean leap25_check(evalfunction_t *evaluate);
boolean leap35_check(evalfunction_t *evaluate);
boolean leap36_check(evalfunction_t *evaluate);
boolean leap36_check(evalfunction_t *evaluate);
boolean leap37_check(evalfunction_t *evaluate);
boolean rccinq_check(evalfunction_t *evaluate);
boolean squirrel_check(evalfunction_t *evaluate);
boolean vizir_check(evalfunction_t *evaluate);
boolean zebra_check(evalfunction_t *evaluate);
boolean zebu_check(evalfunction_t *evaluate);

#endif
