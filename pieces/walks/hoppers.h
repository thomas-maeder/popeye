#if !defined(PIECES_WALKS_HOPPERS_H)
#define PIECES_WALKS_HOPPERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"
#include "solving/move_generator.h"

/* This module implements hopper pieces.
 */

typedef struct
{
    vec_index_type idx_dir;
    square sq_hurdle;
} hoppper_moves_auxiliary_type;

extern hoppper_moves_auxiliary_type hoppper_moves_auxiliary[toppile+1];

/* push current move, all while remembering the current hurdle and move direction
 * @param idx_dir index into vec
 * @param sq_hurdle hurdle for the move being pushed
 */
void hoppers_push_move(vec_index_type idx_dir, square sq_hurdle);
void hoppers_push_capture(vec_index_type idx_dir, square sq_hurdle);

/* Clear hurdle information in the moves from the current move of ply nbply
 * down to (but not including) some base move.
 * To be used while generating moves for walks that can do both moves using a
 * hurdle and moves that don't
 * @param base index of the base move
 */
void hoppers_clear_hurdles(numecoup base);

/* Generate moves for a hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void rider_hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean rider_hoppers_check(vec_index_type kanf, vec_index_type kend,
                            validator_id evaluate);

boolean grasshopper_check(validator_id evaluate);
boolean rookhopper_check(validator_id evaluate);
boolean bishopper_check(validator_id evaluate);
boolean nightrider_hopper_check(validator_id evaluate);
boolean camelrider_hopper_check(validator_id evaluate);
boolean gnurider_hopper_check(validator_id evaluate);
boolean zebrarider_hopper_check(validator_id evaluate);

/* Generate moves for a leaper hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_hoppers_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean kinghopper_check(validator_id evaluate);
boolean knighthopper_check(validator_id evaluate);

/* Generate moves for a double hopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void doublehopper_generate_moves(vec_index_type vec_start,
                                 vec_index_type vec_end);

boolean doublegrasshopper_check(validator_id evaluate);
boolean doublerookhopper_check(validator_id evaluate);
boolean doublebishopper_check(validator_id evaluate);

/* Generate moves for a contra-grasshopper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void contra_grasshopper_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean contragrasshopper_check(validator_id evaluate);

/* Generate moves for a grasshopper-n piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void grasshoppers_n_generate_moves(vec_index_type kbeg, vec_index_type kend,
                                   unsigned int dist_hurdle_target);

boolean grasshopper_2_check(validator_id evaluate);
boolean grasshopper_3_check(validator_id evaluate);

/* Generate moves for an equihopper
 */

void equihopper_generate_moves(void);

boolean equihopper_check(validator_id evaluate);

/* Generate moves for an nonstop-equihopper
 */
void nonstop_equihopper_generate_moves(void);

boolean nonstop_equihopper_check(validator_id evaluate);

/* Generate moves for an equistopper
 */
void equistopper_generate_moves(void);

boolean equistopper_check(validator_id evaluate);

/* Generate moves for an nonstop-equistopper
 */
void nonstop_equistopper_generate_moves(void);

boolean nonstop_equistopper_check(validator_id evaluate);

/* Generate moves for an Orix
 */
void orix_generate_moves(void);

boolean orix_check(validator_id evaluate);

/* Generate moves for an nonstop-Orix
 */
void nonstop_orix_generate_moves(void);

boolean nonstop_orix_check(validator_id evaluate);

#endif
