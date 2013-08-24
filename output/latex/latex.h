#if !defined(OUTPUT_LATEX_H)
#define OUTPUT_LATEX_H

#include "position/position.h"
#include "utilities/boolean.h"

extern boolean LaTeXout;

boolean LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);
void LaTeXEchoAddedPiece(Flags Spec, PieNam Name, square Square);
void LaTeXEchoRemovedPiece(Flags Spec, PieNam Name, square Square);
void LaTeXEchoExchangedPiece(Flags Spec, PieNam Name);
void LaTeXEchoSubstitutedPiece(PieNam from, PieNam to);
char *ParseLaTeXPieces(char *tok);

#endif
