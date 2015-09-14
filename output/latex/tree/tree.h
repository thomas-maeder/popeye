#if !defined(OUTPUT_LATEX_TREE_H)
#define OUTPUT_LATEX_TREE_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

#include <stdio.h>

/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 * @param si identifies slice where to start
 */
void solving_insert_output_latex_tree_slices(slice_index si, FILE *file);

/* Write a possibly pending move decoration
 */
void output_latex_tree_write_pending_move_decoration(void);

#endif
