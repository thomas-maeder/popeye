#if !defined(OUTPUT_LATEX_H)
#define OUTPUT_LATEX_H

#include "position/position.h"
#include "input/plaintext/twin.h"
#include "utilities/boolean.h"

#include <stdio.h>

extern boolean LaTeXout;

extern FILE *LaTeXFile;
extern FILE *TextualSolutionBuffer;

boolean LaTeXSetup(void);
void LaTeXShutdown(void);

void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

void LaTexOpenSolution(void);
void LaTeXFlushSolution(void);

char *ParseLaTeXPieces(char *tok);

void LaTeXStr(char const *line);
char *LaTeXWalk(piece_walk_type walk);

#endif
