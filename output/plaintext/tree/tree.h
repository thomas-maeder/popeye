#if !defined(OUTPUT_PLAINTEXT_TREE_H)
#define OUTPUT_PLAINTEXT_TREE_H

#include "pydata.h"
#include "pystip.h"

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 * @param si identifies slice where to start
 */
void stip_insert_output_plaintext_tree_slices(slice_index si);

/* Remember the decoration (! or ?) for the move just written
 * @param type identifies decoration to be written
 */
void output_plaintext_tree_remember_move_decoration(attack_type type);

/* Write a possibly pending move decoration
 */
void output_plaintext_tree_write_pending_move_decoration(void);

/* Write a move in battle play
 */
void output_plaintext_tree_write_move(void);

#endif
