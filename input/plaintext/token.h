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
extern char const **GlobalTokenTab; /* used for determining language */

typedef enum
{
  EndProblem,       /* 0 */
  NextProblem,      /* 1 */

  ProblemTokenCount /* 2 */
} ProblemToken;

extern char const *ProblemTokenString[LanguageCount][ProblemTokenCount];
extern char const **ProblemTokenTab; /* set according to language */

typedef enum
{
  TwinProblem,     /* 0 */
  ZeroPosition,    /* 1 */

  EndTwinTokenCount,  /* 2 */
} EndTwinToken;

extern char const *EndTwinTokenString[LanguageCount][EndTwinTokenCount];
extern char const **EndTwinTokenTab; /* set according to language */

typedef enum
{
  RemToken,        /*  0 */
  StructStipToken, /*  1 */
  Author,          /*  2 */
  Origin,          /*  3 */
  PieceToken,      /*  4 */
  CondToken,       /*  5 */
  OptToken,        /*  6 */
  StipToken,       /*  7 */
  TraceToken,      /*  8 */
  SepToken,        /*  9 */
  TitleToken,      /* 10 */
  LaTeXToken,      /* 11 */
  LaTeXPieces,     /* 12 */
  Award,           /* 13 */
  Array,           /* 14 */
  Forsyth,         /* 15 */

  InitialTwinTokenCount,       /* 16 */
  SubsequentTwinTokenCount = 1
} InitialTwinToken;

extern char const *InitialTwinTokenString[LanguageCount][InitialTwinTokenCount];
extern char const **InitialTwinTokenTab; /* set according to language */

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
