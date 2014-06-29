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
