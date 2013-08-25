#if !defined(OUTPUT_PLAINTEXT_CONDITION_H)
#define OUTPUT_PLAINTEXT_CONDITION_H

#include "utilities/boolean.h"

void WriteBGLNumber(char* a, long int b);

boolean WriteConditions(void (*WriteCondition)(char const CondLine[], boolean is_first));

#endif
