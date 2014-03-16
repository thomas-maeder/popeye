#if !defined(OUTPUT_PLAINTEXT_LANGUAGE_DEPENDANT_H)
#define OUTPUT_PLAINTEXT_LANGUAGE_DEPENDANT_H

/* This module provides language dependant strings
 */
#include "pieces/pieces.h"
#include "position/position.h"
#include "pylang.h"
#include "conditions/conditions.h"
#include "options/options.h"
#include "input/plaintext/twin.h"
#include "conditions/mummer.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/sentinelles.h"

typedef char PieceChar[2];
typedef PieceChar PieTable[PieceCount];
extern PieTable PieNamString[LanguageCount];

extern PieceChar *PieceTab;

extern char const * const *OptTab;
extern char const * const OptString[LanguageCount][OptCount];

extern char const * const CondString[LanguageCount][CondCount];
extern char const * const *CondTab;

extern char const * const ExtraCondString[LanguageCount][ExtraCondCount];
extern char const * const *ExtraCondTab;

extern char const * const ColorString[LanguageCount][nr_colors];
extern char const * const *ColorTab;

extern  char const * const PieSpString[LanguageCount][PieSpCount-nr_sides];
extern  char const * const *PieSpTab;

extern char const * const VaultingVariantTypeString[LanguageCount][1];
char const * const *VaultingVariantTypeTab;

extern char const * const ConditionLetteredVariantTypeString[LanguageCount][ConditionLetteredVariantTypeCount];
extern char const * const *ConditionLetteredVariantTypeTab;

extern char const * const ConditionNumberedVariantTypeString[LanguageCount][ConditionNumberedVariantTypeCount];
extern char const * const *ConditionNumberedVariantTypeTab;

extern char const * const AntiCirceVariantTypeString[LanguageCount][anticirce_type_count];
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

extern char const * const SentinellesVariantTypeString[LanguageCount][SentinellesVariantCount];
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

extern char const * const GridVariantTypeString[LanguageCount][GridVariantCount];
extern char const * const *GridVariantTypeTab;

typedef enum
{
  KobulWhiteOnly,
  KobulBlackOnly,

  KobulVariantCount
} KobulVariantType;

extern char const * const KobulVariantTypeString[LanguageCount][KobulVariantCount];
extern char const * const *KobulVariantTypeTab;

extern char const * const KoekoVariantTypeString[LanguageCount][1];
extern char const * const *KoekoVariantTypeTab;

typedef enum
{
  CirceVariantRexInclusive,
  CirceVariantRexExclusive,
  CirceVariantMirror,
  CirceVariantAssassin,
  CirceVariantDiametral,
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

  CirceVariantCount
} CirceVariantType;

extern char const * const CirceVariantTypeString[LanguageCount][CirceVariantCount];
extern char const * const *CirceVariantTypeTab;

extern char const * const TwinningString[LanguageCount][TwinningCount];
extern char const * const *TwinningTab;

extern char const * const mummer_strictness_string[LanguageCount][nr_mummer_strictness];
extern char const * const *mummer_strictness_tab;

/* Make sure that we don't define ambiguous piece name shortcuts */
void enforce_piecename_uniqueness(void);

#endif
