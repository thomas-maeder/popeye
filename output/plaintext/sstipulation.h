#if !defined(OUTPUT_PLAINTEXT_SSTIPULATION_H)
#define OUTPUT_PLAINTEXT_SSTIPULATION_H

/* Plain text output of structured stipulations */

#include "stipulation/stipulation.h"

/* Write a structured stiplation
 * @param si identiifes the entry slice into the stipulation
 * @return number of characters written
 */
int WriteSStipulation(slice_index si);

#endif
