#if !defined(PYOUTPUT_H)
#define PYOUTPUT_H

#include "pystip.h"

/* identifies a slice whose starter is the nominal starter of the stipulation
 * before any move inversions are applied
 * (e.g. in a h#N.5, this slice's starter is Black)
 */
extern slice_index output_plaintext_slice_determining_starter;

/* Allocate an STOutputModeSelector slice
 * @param mode output mode to be selected by the allocated slice
 * @return identifier of the allocated slice
 */
slice_index alloc_output_mode_selector(output_mode mode);

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void stip_insert_output_slices(slice_index si);

#endif
