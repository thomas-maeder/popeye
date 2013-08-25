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

typedef char PieceChar[2];
typedef PieceChar PieTable[PieceCount];
extern PieTable PieNamString[LanguageCount];

extern PieceChar *PieceTab;
extern char const * *OptTab;

extern char const *OptString[LanguageCount][OptCount];

extern char const *CondString[LanguageCount][CondCount];
extern char const **CondTab;

extern char const *ExtraCondString[LanguageCount][ExtraCondCount];
extern char const **ExtraCondTab;

extern char const *ColorString[LanguageCount][nr_colors];
extern char const **ColorTab;

extern  char const *PieSpString[LanguageCount][PieSpCount-nr_sides];
extern  char const **PieSpTab;

extern char const *VariantTypeString[LanguageCount][VariantTypeCount];
char const **VariantTypeTab;

extern char const *TwinningString[LanguageCount][TwinningCount];
extern char const **TwinningTab;

extern char const *mummer_strictness_string[LanguageCount][nr_mummer_strictness];
extern char const **mummer_strictness_tab;

/* Make sure that we don't define ambiguous piece name shortcuts */
void enforce_piecename_uniqueness(void);

#endif
