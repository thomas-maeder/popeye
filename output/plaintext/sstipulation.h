#if !defined(OUTPUT_PLAINTEXT_SSTIPULATION_H)
#define OUTPUT_PLAINTEXT_SSTIPULATION_H

/* Plain text output of structured stipulations */

#include "stipulation/stipulation.h"

#include <stdio.h>

/* Write a structured stipulation
 * @param file where to write to
 * @param si identiifes the entry slice into the stipulation
 * @return number of characters written
 */
unsigned int WriteSStipulation(FILE *file, slice_index si);

#endif
