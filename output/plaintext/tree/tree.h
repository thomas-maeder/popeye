#if !defined(OUTPUT_PLAINTEXT_TREE_H)
#define OUTPUT_PLAINTEXT_TREE_H

#include "pydata.h"
#include "pystip.h"

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 */
void stip_insert_output_plaintext_tree_slices(void);

/* Remember the decoration (! or ?) for a battle move just written
 * @param type identifies decoration to be written
 */
void remember_battle_move_decoration(attack_type type);

/* Write a possibly pending move decoration
 */
void write_pending_decoration(void);

#endif
