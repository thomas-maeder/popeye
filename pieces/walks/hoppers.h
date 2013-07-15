#if !defined(PIECES_WALKS_HOPPERS_H)
#define PIECES_WALKS_HOPPERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* This module implements hopper pieces.
 */

/* Generate moves for a hopper piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void hoppers_generate_moves(square sq_departure,
                            vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a leaper hopper piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_hoppers_generate_moves(square sq_departure,
                                   vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a double hopper piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void doublehopper_generate_moves(square sq_departure,
                                 vec_index_type vec_start,
                                 vec_index_type vec_end);

/* Generate moves for a contra-grasshopper piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void contra_grasshopper_generate_moves(square sq_departure,
                                       vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a grasshopper-n piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void grasshoppers_n_generate_moves(square sq_departure,
                                   vec_index_type kbeg, vec_index_type kend,
                                   unsigned int dist_hurdle_target);

/* Generate moves for an equihopper
 * @param sq_departure common departure square of the generated moves
 */
void equihopper_generate_moves(square sq_departure);

/* Generate moves for an nonstop-equihopper
 * @param sq_departure common departure square of the generated moves
 */
void nonstop_equihopper_generate_moves(square sq_departure);

/* Generate moves for an equistopper
 * @param sq_departure common departure square of the generated moves
 */
void equistopper_generate_moves(square sq_departure);

/* Generate moves for an nonstop-equistopper
 * @param sq_departure common departure square of the generated moves
 */
void nonstop_equistopper_generate_moves(square sq_departure);

/* Generate moves for an Orix
 * @param sq_departure common departure square of the generated moves
 */
void orix_generate_moves(square sq_departure);

/* Generate moves for an nonstop-Orix
 * @param sq_departure common departure square of the generated moves
 */
void nonstop_orix_generate_moves(square sq_departure);

#endif
