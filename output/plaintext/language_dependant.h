#if !defined(OUTPUT_PLAINTEXT_LANGUAGE_DEPENDANT_H)
#define OUTPUT_PLAINTEXT_LANGUAGE_DEPENDANT_H

/* This module provides language dependant strings
 */
#include "pieces/pieces.h"
#include "position/position.h"
#include "conditions/conditions.h"
#include "options/options.h"
#include "input/plaintext/twin.h"
#include "input/plaintext/language.h"
#include "conditions/mummer.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/sentinelles.h"
#include "output/plaintext/message.h"

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

  EndTwinTokenCount  /* 2 */
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
  SubsequentTwinTokenCount = 1 /* only remark allowed in subsequent twins */
} InitialTwinToken;

extern char const *InitialTwinTokenString[LanguageCount][InitialTwinTokenCount];
extern char const **InitialTwinTokenTab; /* set according to language */

typedef char PieceChar[2];

extern PieceChar *PieceTab;

extern char const * const *OptTab;
extern char const * const *CondTab;
extern char const * const *ExtraCondTab;
extern char const * const *ColourTab;
extern char const * const *PieSpTab;
extern char const * const *VaultingVariantTypeTab;
extern char const * const *ConditionLetteredVariantTypeTab;
extern char const * const *ConditionNumberedVariantTypeTab;
extern char const * const *AntiCirceVariantTypeTab;

typedef enum
{
  SentinellesVariantPionPropre,
  SentinellesVariantPionAdverse,
  SentinellesVariantPionNeutral,
  SentinellesVariantPionNoirMaximum,
  SentinellesVariantPionBlancMaximum,
  SentinellesVariantPionTotalMaximum,
  SentinellesVariantPara,
  SentinellesVariantBerolina,

  SentinellesVariantCount
} SentinellesVariantType;

extern char const * const *SentinellesVariantTypeTab;

typedef enum
{
  GridVariantShiftRank,
  GridVariantShiftFile,
  GridVariantShiftRankFile,
  GridVariantOrthogonal,
  GridVariantIrregular,
  GridVariantExtraGridLines,

  GridVariantCount
} GridVariantType;

extern char const * const *GridVariantTypeTab;

typedef enum
{
  KobulWhiteOnly,
  KobulBlackOnly,

  KobulVariantCount
} KobulVariantType;

extern char const * const *KobulVariantTypeTab;

extern char const * const *KoekoVariantTypeTab;

typedef enum
{
  CirceVariantRexInclusive,
  CirceVariantRexExclusive,
  CirceVariantMirror,
  CirceVariantAssassin,
  CirceVariantDiametral,
  CirceVariantVerticalMirror,
  CirceVariantClone,
  CirceVariantChameleon,
  CirceVariantTurncoats,
  CirceVariantCouscous,
  CirceVariantLastMove,
  CirceVariantEquipollents,
  CirceVariantParrain,
  CirceVariantContraParrain,
  CirceVariantCage,
  CirceVariantRank,
  CirceVariantFile,
  CirceVariantSymmetry,
  CirceVariantVerticalSymmetry,
  CirceVariantHorizontalSymmetry,
  CirceVariantDiagramm,
  CirceVariantPWC,
  CirceVariantAntipodes,
  CirceVariantTakeAndMake,
  CirceVariantSuper,
  CirceVariantApril,
  CirceVariantFrischauf,
  CirceVariantCalvet,
  CirceVariantCheylan,
  CirceVariantStrict,
  CirceVariantRelaxed,
  CirceVariantVolcanic,
  CirceVariantParachute,
  CirceVariantEinstein,
  CirceVariantReverseEinstein,

  CirceVariantCount
} CirceVariantType;

extern char const * const *CirceVariantTypeTab;
extern char const * const *TwinningTab;
extern char const * const *TwinningMirrorTab;
extern char const * const *mummer_strictness_tab;

void output_plaintext_select_language(Language lang);

/* Make sure that we don't define ambiguous piece name shortcuts */
void enforce_piecename_uniqueness(void);

#endif
