#if !defined(INPUT_PLAINTEXT_CONDITION_H)
#define INPUT_PLAINTEXT_CONDITION_H

#include "stipulation/stipulation.h"

char *ParseCond(char *tok);

void InitCond(void);

void  conditions_resetter_solve(slice_index si);

#endif
