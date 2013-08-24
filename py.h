/********************** MODIFICATIONS to py.h **************************
** This is the list of modifications done to py.h
**
** Date       Who  What
**
** 2006/05/07 SE   bug fix: StipExch + Duplex
**
** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
**
** 2006/07/30 SE   New condition: Schwarzschacher
**
** 2007/01/28 SE   New condition: NormalPawn
**
** 2007/01/28 SE   New condition: Annan Chess
**
** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
**
** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
**                 Transmuting/Reflecting Ks now take optional piece list
**                 turning them into vaulting types
**
** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
**
** 2007/12/21 SE   Command-line switch: -b set low priority (Win32 only so far)
**
** 2007/12/26 SE   Pragma: disable warnings on deprecated functions in VC8
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
** 2009/06/14 SE   New optiion: LastCapture
**
** 2009/06/27 SE   Extended imitators/hurdlecolorchanging to moose etc.
**
** 2012/01/27 NG   AlphabeticChess now possible for white or black only.
**
**************************** End of List ******************************/

#if !defined(PY_H)
#define PY_H

#ifdef _SE_DECORATE_SOLUTION_
#define _SE_
#endif
#ifdef _SE_FORSYTH_
#define _SE_
#endif

#if !defined(OSTYPE)
#  if defined(C370)
#    define OSTYPE "MVS"
#  elseif defined(DOS)
#    define OSTYPE "DOS"
#  elseif defined(ATARI)
#    define OSTYPE "ATARI"
#  elseif defined(_WIN98)
#    define OSTYPE "WINDOWS98"
#  elseif defined(_WIN16) || defined(_WIN32)
#    define OSTYPE "WINDOWS"
#  elseif defined(__unix)
#    if defined(__GO32__)
#      define OSTYPE "DOS"
#    else
#      define OSTYPE "UNIX"
#    endif  /* __GO32__ */
#  else
#    define OSTYPE "C"
#  endif
#endif

#endif  /* PY_H */
