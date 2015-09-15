#if !defined(OUTPUT_PLAINTEXT_TREE_H)
#define OUTPUT_PLAINTEXT_TREE_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 * @param si identifies slice where to start
 */
void solving_insert_output_plaintext_tree_slices(slice_index si);

/* Instrument the solving machinery with move inversion counter slices
 * @param si identifies slice where to start
 */
void solving_insert_move_inversion_counter_slices(slice_index si);

/* Write a possibly pending move decoration
 */
void output_plaintext_tree_write_pending_move_decoration(void);

/* Write a move in battle play
 */
void output_plaintext_tree_write_move(void);

#endif
