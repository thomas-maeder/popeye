#if !defined(INPUT_PLAINTEXT_STIPULATION_H)
#define INPUT_PLAINTEXT_STIPULATION_H

#include "stipulation/stipulation.h"

void select_output_mode(slice_index si, output_mode mode);

char *ParseStip(char *tok, slice_index start);

#endif
