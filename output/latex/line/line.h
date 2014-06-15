#if !defined(OUTPUT_LATEX_LINE_H)
#define OUTPUT_LATEX_LINE_H

#include "stipulation/stipulation.h"

/* Instrument the stipulation structure with slices that implement
 * LaTeX line mode output.
 * @param si identifies slice where to start
 */
void solving_insert_output_latex_line_slices(slice_index si);

#endif
