#if !defined(INPUT_PLAINTEXT_INPUT_STACK_H)
#define INPUT_PLAINTEXT_INPUT_STACK_H

#include <stdio.h>

enum
{
  MAXNEST = 10
};

extern FILE *InputStack[MAXNEST];

extern unsigned int NestLevel;

void OpenInput(char const *s);
void CloseInput(void);

int PushInput(char const *FileName);
int PopInput(void);

#endif
