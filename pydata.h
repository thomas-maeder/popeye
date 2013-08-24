/********************** MODIFICATIONS to pydata.h **************************
** This is the list of modifications done to pydata.h
**
** Date       Who  What
**
** 2006/07/30 SE   New condition: Schwarzschacher
**
** 2007/01/28 SE   New condition: NormalPawn
**
** 2007/01/28 SE   New condition: Annan Chess
**
** 2007/04/30 NG   sentinelles variants: naming changed for consistency ...
**
** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
**
** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
**
** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
**
** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
**                 Transmuting/Reflecting Ks now take optional piece list
**                 turning them into vaulting types
**
** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
**
** 2007/12/21 SE   Command-line switch: -maxtime (same func as Option)
**
** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
**                 New condition: Protean Chess
**                 New piece type: Protean man (invent A.H.Kniest?)
**                 (Differs from Frankfurt chess in that royal riders
**                 are not 'non-passant'. Too hard to do but possibly
**                 implement as an independent condition later).
**
** 2008/01/02 NG   New condition: Geneva Chess
**
** 2008/01/11 SE   New variant: Special Grids
**
** 2008/01/13 SE   New conditions: White/Black Vaulting Kings
**
** 2008/01/24 SE   New variant: Gridlines
**
** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)
**
** 2008/02/19 SE   New condition: AntiKoeko
**
** 2008/02/19 SE   New piece: RoseLocust
**
** 2008/02/25 SE   New piece type: Magic
**
** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
**
** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
**
** 2009/02/24 SE   New pieces: 2,0-Spiralknight
**                             4,0-Spiralknight
**                             1,1-Spiralknight
**                             3,3-Spiralknight
**                             Quintessence (invented Joerg Knappen)
**
** 2009/04/25 SE   New condition: Provacateurs
**                 New piece type: Patrol pieces
**
** 2009/06/14 SE   New option: LastCapture
**
** 2012/01/27 NG   AlphabeticChess now possible for white or black only.
**
**************************** End of List ******************************/

#if !defined(PYDATA_H)
#define PYDATA_H

#include "position/position.h"
#include "utilities/boolean.h"
#include "solving/ply.h"

#include <limits.h>

#if !defined(EXTERN)
#       define EXTERN extern
#       define WE_ARE_EXTERN
#endif  /* EXTERN */

EXTERN boolean LaTeXout;
EXTERN boolean flag_regression;

EXTERN char versionString[100];

EXTERN unsigned int number_of_pieces[nr_sides][PieceCount];

EXTERN  boolean  exist[PieceCount];
EXTERN  boolean  may_exist[PieceCount];

EXTERN  Side trait[maxply+1];
EXTERN  boolean flagfee;

#if defined(WE_ARE_EXTERN)
  extern char GlobalStr[];
#else
  char GlobalStr[4*maxply];
#endif

#define hunterseparator '/'

#endif  /* not PYDATA_H */
