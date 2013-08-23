#if !defined(OUTPUT_PLAINTEXT_LINE_H)
#define OUTPUT_PLAINTEXT_LINE_H

#include "stipulation/stipulation.h"

/* Instrument the stipulation structure with slices that implement
 * plaintext line mode output.
 * @param si identifies slice where to start
 */
void stip_insert_output_plaintext_line_slices(slice_index si);

#endif
