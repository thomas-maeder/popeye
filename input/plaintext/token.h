#if !defined(INPUT_PLAINTEXT_TOKEN_H)
#define INPUT_PLAINTEXT_TOKEN_H

#include "input/plaintext/language.h"

#include <stdio.h>

typedef enum
{
  BeginProblem,     /* 0 */
  EndProblem,       /* 1 */
  NextProblem,      /* 2 */
  StipToken,        /* 3 */
  StructStipToken,  /* 4 */
  Author,           /* 5 */
  Origin,           /* 6 */
  PieceToken,       /* 7 */
  CondToken,        /* 8 */
  OptToken,         /* 9 */
  RemToken,        /* 10 */
  TraceToken,      /* 11 */
  SepToken,        /* 12 */
  TitleToken,      /* 13 */
  TwinProblem,     /* 14 */
  ZeroPosition,    /* 15 */
  LaTeXToken,      /* 16 */
  LaTeXPieces,     /* 17 */
  Award,           /* 18 */
  Array,           /* 19 */
  Forsyth,         /* 20 */

  TokenCount       /* 21 */
} Token;

extern char const *TokenString[LanguageCount][TokenCount];
extern char const **TokenTab; /* set according to language */

enum
{
  LINESIZE = 256
};

extern char InputLine[LINESIZE];    /* This array contains the input as is */

extern char TokenLine[LINESIZE];

void OpenInput(char const *s);
void CloseInput(void);

char *ReadNextCaseSensitiveTokStr(void);

char *ReadNextTokStr(void);

unsigned int GetUniqIndex(unsigned int limit, char const * const *list, char const *tok);

Token StringToToken(char const *tok);

/* read into InputLine until the next1 end of line */
void ReadToEndOfLine(void);

void ReadRemark(void);

fpos_t InputGetPosition(void);
void InputStartReplay(fpos_t pos);
void InputEndReplay(void);

#endif
