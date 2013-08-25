#if !defined(INPUT_PLAINTEXT_STIPULATION_H)
#define INPUT_PLAINTEXT_STIPULATION_H

#include "stipulation/stipulation.h"

extern char AlphaStip[200];

void select_output_mode(slice_index si, output_mode mode);

char *ParseStip(slice_index root_slice_hook);

#endif
