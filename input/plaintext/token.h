#if !defined(INPUT_PLAINTEXT_TOKEN_H)
#define INPUT_PLAINTEXT_TOKEN_H

#include "pylang.h"
#include "input/plaintext/line.h"

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
  InputToken,      /* 12 */
  SepToken,        /* 13 */
  TitleToken,      /* 14 */
  TwinProblem,     /* 15 */
  ZeroPosition,    /* 16 */
  LaTeXToken,      /* 17 */
  LaTeXPieces,     /* 18 */
  Award,           /* 19 */
  Array,           /* 20 */
  Forsyth,         /* 21 */

  TokenCount       /* 22 */
} Token;

extern char LastChar;

extern char const *TokenString[LanguageCount][TokenCount];
extern char const **TokenTab; /* set according to language */

extern char TokenLine[LINESIZE];

/* advance LastChar to the next1 input character */
void NextChar(void);

char *ReadNextCaseSensitiveTokStr(void);

char *ReadNextTokStr(void);

unsigned int GetUniqIndex(unsigned int limit, char const * const *list, char const *tok);

Token StringToToken(char const *tok);

#endif
