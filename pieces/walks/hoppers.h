#if !defined(PIECES_WALKS_HOPPERS_H)
#define PIECES_WALKS_HOPPERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* This module implements hopper pieces.
 */

/* Generate moves for a hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a leaper hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a double hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void doublehopper_generate_moves(vec_index_type vec_start,
                                 vec_index_type vec_end);

/* Generate moves for a contra-grasshopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void contra_grasshopper_generate_moves(vec_index_type kbeg, vec_index_type kend);

/* Generate moves for a grasshopper-n piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void grasshoppers_n_generate_moves(vec_index_type kbeg, vec_index_type kend,
                                   unsigned int dist_hurdle_target);

/* Generate moves for an equihopper
 */
void equihopper_generate_moves(void);

/* Generate moves for an nonstop-equihopper
 */
void nonstop_equihopper_generate_moves(void);

/* Generate moves for an equistopper
 */
void equistopper_generate_moves(void);

/* Generate moves for an nonstop-equistopper
 */
void nonstop_equistopper_generate_moves(void);

/* Generate moves for an Orix
 */
void orix_generate_moves(void);

/* Generate moves for an nonstop-Orix
 */
void nonstop_orix_generate_moves(void);

#endif
