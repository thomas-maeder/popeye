#if !defined(INPUT_PLAINTEXT_TOKEN_H)
#define INPUT_PLAINTEXT_TOKEN_H

#include "input/plaintext/language.h"
#include "utilities/boolean.h"

#include <stdio.h>

enum
{
  LINESIZE = 256
};

extern char InputLine[LINESIZE];    /* This array contains the input as is */

extern char TokenLine[LINESIZE];

boolean OpenInput(char const *s);
void CloseInput(void);

char *ReadNextCaseSensitiveTokStr(void);

char *ReadNextTokStr(void);

unsigned int GetUniqIndex(unsigned int limit, char const * const *list, char const *tok);

/* read into InputLine until the next1 end of line */
void ReadToEndOfLine(void);

void ReadRemark(void);

fpos_t InputGetPosition(void);
void InputStartReplay(fpos_t pos);
void InputEndReplay(void);

#endif
