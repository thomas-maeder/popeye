#if !defined(INPUT_PLAINTEXT_TOKEN_H)
#define INPUT_PLAINTEXT_TOKEN_H

#include "input/plaintext/language.h"
#include "utilities/boolean.h"

#include <stdio.h>

typedef enum
{
  BeginProblem,     /* 0 */

  GlobalTokenCount  /* 1 */
} GlobalToken;

extern char const *GlobalTokenString[LanguageCount][GlobalTokenCount];
extern char const **GlobalTokenTab;

typedef enum
{
  EndProblem,      /*  0 */
  NextProblem,     /*  1 */
  StipToken,       /*  2 */
  StructStipToken, /*  3 */
  Author,          /*  4 */
  Origin,          /*  5 */
  PieceToken,      /*  6 */
  CondToken,       /*  7 */
  OptToken,        /*  8 */
  RemToken,        /*  9 */
  TraceToken,      /* 10 */
  SepToken,        /* 11 */
  TitleToken,      /* 12 */
  TwinProblem,     /* 13 */
  ZeroPosition,    /* 14 */
  LaTeXToken,      /* 15 */
  LaTeXPieces,     /* 16 */
  Award,           /* 17 */
  Array,           /* 18 */
  Forsyth,         /* 19 */

  TokenCount       /* 20 */
} Token;

extern char const *TokenString[LanguageCount][TokenCount];
extern char const **TokenTab; /* set according to language */

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
