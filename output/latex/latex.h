#if !defined(OUTPUT_LATEX_H)
#define OUTPUT_LATEX_H

#include "position/position.h"
#include "utilities/boolean.h"

#include <stdio.h>

extern boolean LaTeXout;
extern FILE *TextualSolutionBuffer;

boolean LaTeXSetup(void);
void LaTeXShutdown(void);

void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

void LaTeXEchoAddedPiece(Flags Spec, piece_walk_type Name, square Square);
void LaTeXEchoRemovedPiece(Flags Spec, piece_walk_type Name, square Square);
void LaTeXEchoMovedPiece(Flags Spec, piece_walk_type Name, square FromSquare, square ToSquare);
void LaTeXEchoExchangedPiece(Flags Spec1, piece_walk_type Name1, square Square1,
                             Flags Spec2, piece_walk_type Name2, square Square2);
void LaTeXEchoSubstitutedPiece(piece_walk_type from, piece_walk_type to);
char *ParseLaTeXPieces(char *tok);

void LaTeXBeginTwinning(unsigned int TwinNumber);
void LaTeXEndTwinning(void);

void LaTeXNextTwinning(void);
void LaTeXContinuedTwinning(void);

void LaTeXTwinningRotate(char const text[]);
void LaTeXTwinningShift(square From, square To);
void LaTeXTwinningPolish(void);
void LaTeXTwinningFirstCondition(char const text[]);
void LaTeXTwinningNextCondition(char const text[]);
void LaTeXTwinningStipulation(char const text[]);

#endif
