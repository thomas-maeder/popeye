#if !defined(OUTPUT_PLAINTEXT_TREE_H)
#define OUTPUT_PLAINTEXT_TREE_H

#include "pydata.h"

extern unsigned int nr_moves_written[maxply+1];


/* Instrument the stipulation structure with slices that implement
 * plaintext tree mode output.
 */
void stip_insert_output_plaintext_tree_slices(void);

#endif
