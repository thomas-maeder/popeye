/******************** MODIFICATIONS to pyproc.h ************************
**
** This is the list of modifications done to pyproc.h
**
** Date       Who  What
**
** 2007/01/28 SE   New condition: Annan Chess
**
** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
**
** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
**
** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
**
** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
**
** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
**
** 2008/01/11 SE   New variant: Special Grids
**
** 2008/01/24 SE   New variant: Gridlines
**
** 2008/02/20 SE   Bugfix: Annan
**
** 2008/02/19 SE   New condition: AntiKoeko
**
** 2008/02/19 SE   New piece: RoseLocust
**
** 2008/02/25 SE   New piece type: Magic
**
** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
**
** 2009/02/24 SE   New pieces: 2,0-Spiralknight
**                             4,0-Spiralknight
**                             1,1-Spiralknight
**                             3,3-Spiralknight
**                             Quintessence (invented Joerg Knappen)
** 2009/06/27 SE   Extended imitators/hurdlecolorchanging to moose etc.
**
**
**************************** End of List ******************************/

#if !defined(PYPROC_H)
#define PYPROC_H

#include "pieces/walks/vectors.h"
#include "stipulation/goals/goals.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

#include <stdio.h>

void InitMetaData(void);
void    OpenInput(char *s);
void    CloseInput(void);
void    MultiCenter(char *s);
void    WritePosition(void);
void    WriteSquare(square a);
void    WritePiece(PieNam p);
boolean WriteSpec(Flags pspec, PieNam p, boolean printcolours);
void    WriteGrid(void);

void    hardinit(void);

void    PrintTime();

Token   ReadTwin(Token tk, slice_index root_slice_hook);
void WriteTwinNumber(void);
void    ErrString(char const *s);
void    StdChar(char c);
void    StdString(char const *s);
void    logChrArg(char arg);
void    logStrArg(char *arg);
void    logIntArg(int arg);
void    logLngArg(long arg);

void pyfputs(char const *s, FILE *f);

void	pyInitSignal(void);
void	InitCond(void);
char	*MakeTimeString(void);

char *ReadPieces(int cond);

void LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

void WriteBGLNumber(char* a, long int b);

#endif  /* PYPROC_H */
