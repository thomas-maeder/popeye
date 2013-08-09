#if !defined(PIECES_WALKS_RADIAL_H)
#define PIECES_WALKS_RADIAL_H

/* This module implements radial pieces */

/* Generate moves for a radial knight
 */
void radialknight_generate_moves(void);

/* Generate moves for a (lesser) tree hopper
 */
void treehopper_generate_moves(void);

/* Generate moves for a greater tree hopper
 */
void greater_treehopper_generate_moves(void);

/* Generate moves for a (lesser) leaf hopper
 */
void leafhopper_generate_moves(void);

/* Generate moves for a greater leaf hopper
 */
void greater_leafhopper_generate_moves(void);

#endif
