#if !defined(OUTPUT_LATEX_H)
#define OUTPUT_LATEX_H

#include "position/position.h"
#include "utilities/boolean.h"

extern boolean LaTeXout;

extern char ActTwinning[1532];

boolean LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);
void LaTeXEchoAddedPiece(Flags Spec, PieNam Name, square Square);
void LaTeXEchoRemovedPiece(Flags Spec, PieNam Name, square Square);
void LaTeXEchoMovedPiece(Flags Spec, PieNam Name, square FromSquare, square ToSquare);
void LaTeXEchoExchangedPiece(Flags Spec1, PieNam Name1, square Square1,
                             Flags Spec2, PieNam Name2, square Square2);
void LaTeXEchoSubstitutedPiece(PieNam from, PieNam to);
char *ParseLaTeXPieces(char *tok);

#endif
