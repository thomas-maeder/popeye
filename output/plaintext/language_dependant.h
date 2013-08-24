#if !defined(OUTPUT_PLAINTEXT_LANGUAGE_DEPENDANT_H)
#define OUTPUT_PLAINTEXT_LANGUAGE_DEPENDANT_H

/* This module provides language dependant strings
 */
#include "position/position.h"
#include "pylang.h"
#include "py.h"
#include "conditions/conditions.h"

typedef char PieceChar[2];
typedef PieceChar PieTable[PieceCount];
extern PieTable PieNamString[LanguageCount];

extern PieceChar *PieceTab;

extern boolean OptFlag[OptCount];
extern char *OptString[LanguageCount][OptCount];

extern boolean  CondFlag[CondCount];
extern char *CondString[LanguageCount][CondCount];

extern char *ColorString[LanguageCount][nr_colors];
extern char **ColorTab;

extern  char *PieSpString[LanguageCount][PieSpCount-nr_sides];
extern  char **PieSpTab;

extern char *VariantTypeString[LanguageCount][VariantTypeCount];

extern boolean  ExtraCondFlag[ExtraCondCount];
extern char *ExtraCondString[LanguageCount][ExtraCondCount];

/* Make sure that we don't define ambiguous piece name shortcuts */
void enforce_piecename_uniqueness(void);

#endif
