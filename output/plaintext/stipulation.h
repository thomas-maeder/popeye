#if !defined(OUTPUT_PLAINTEXT_STIPULATION_H)
#define OUTPUT_PLAINTEXT_STIPULATION_H

#include "stipulation/stipulation.h"

#include <stdio.h>

/* Write a traditional stipulation
 * @param file where to write to
 * @param si identiifes the entry slice into the stipulation
 * @return number of characters written
 */
int WriteStipulation(FILE *file, slice_index si);

#endif
