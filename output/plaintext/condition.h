#if !defined(OUTPUT_PLAINTEXT_CONDITION_H)
#define OUTPUT_PLAINTEXT_CONDITION_H

#include "utilities/boolean.h"

#include <stdio.h>

void WriteBGLNumber(char* a, long int b);

boolean WriteConditions(FILE *file,
                        void (*WriteCondition)(FILE *file,
                                               char const CondLine[],
                                               boolean is_first));

#endif
