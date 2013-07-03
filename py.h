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

#include <limits.h>

#include "utilities/boolean.h"
#include "position/position.h"

#ifdef _SE_DECORATE_SOLUTION_
#define _SE_
#endif
#ifdef _SE_FORSYTH_
#define _SE_
#endif

/*   Sometimes local variables are used, that can potentially be
     used without any proper initial value. At least the compiler
     cannot assure a proper initialisation.
     The programmer should verify these places. To spot these
     places, I introduced the following macro. It may be defined
     to nothing, to get the old code -- without var intialisation.
 */
#define VARIABLE_INIT(var)	(var)=0

#if defined(C370)
/* On MVS-systems there's the problem that the C/370 compiler and the
   linker only take the first 8 significant characters valid for
   external functions and variables.
   You need the "pragma map" compiler-directive to avoid this conflict
   without renaming the C-identifiers.   HD
*/

#pragma map( renspiegelfile, "RENSPI01" )
#pragma map( renspiegel, "RENSPI02" )
#pragma map( flagmummer[White], "FLAGWH01" )
#pragma map( flagwhiteexact, "FLAGWH02" )
#pragma map( flagmummer[Black], "FLAGBL01" )
#pragma map( flagblackexact, "FLAGBL02" )
#pragma map( cirrenroib, "CIRREN01" )
#pragma map( cirrenroin, "CIRREN02" )
#pragma map( immrenroin, "IMMREN01" )
#pragma map( immrenroib, "IMMREN02" )
#pragma map( hash_value_1, "HASHVAL1" )
#pragma map( hash_value_2, "HASHVAL2" )

#endif /* C370 */

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

#if defined(_MSC_VER) && _MSC_VER == 1400
/*disable warning of deprecated functions*/
#pragma warning( disable : 4996 )
#endif


#if defined(__bsdi__)
#       define  strchr  index
#endif

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

#if defined(TESTHASH)
#       if !defined(HASHRATE)
#               define HASHRATE
#       endif   /* !HASHRATE */
#endif  /* TESTHASH */

enum
{
  /* the following values are used instead of capture square
   * to indicate special moves */
  messigny_exchange = maxsquare+1,
  min_castling = maxsquare+2,
  kingside_castling = min_castling,
  queenside_castling = maxsquare+3,
  max_castling = queenside_castling,
  platzwechsel_rochade = maxsquare+4,
};

enum
{
#if defined(__unix)
  maxply =   1002
#else
#if defined(_WIN32)
  maxply = 2702
#elif defined(_OS2)
  maxply = 302
#else
#if defined(SIXTEEN) /* DOS 16 Bit, ... */
#if defined(MSG_IN_MEM)
  maxply = 26
#else
  maxply = 48
#endif /* MSG_IN_MEM */
#else
  maxply = 702
#endif/* SIXTEEN */
#endif /* _WIN32 */
#endif /* __unix */
};

enum
{
  nil_ply = 1
};

typedef unsigned int stip_length_type;

enum
{
  no_stip_length = maxply
};

enum
{
  toppile = 60*maxply
};

enum
{
  blank = ' '
};

/* These typedefs should be judged for efficiency */

typedef int         numecoup;

enum
{
  nil_coup = 1
};

typedef int         ply;

typedef square       pilecase[maxply+1];

typedef struct {
    square departure;
    square arrival;
    square capture;
    square singlebox_type3_promotion_where;
    PieNam singlebox_type3_promotion_what;
    PieNam current_transmutation;
    union
    {
      square sq_en_passant;
      struct
      {
          vec_index_type vec_index;
          square sq_hurdle;
      } hopper;
    } auxiliary;
} move_generation_elmt;

/* These are definitions to implement arrays with lower index != 0
** as they are common in PASCAL. The arrays zzza? are defined in
** pydata.h                                                     ElB
*/

#include "pylang.h"

typedef enum
{
  BeginProblem,     /* 0 */
  EndProblem,       /* 1 */
  NextProblem,      /* 2 */
  StipToken,        /* 3 */
  StructStipToken,  /* 4 */
  Author,           /* 5 */
  Origin,           /* 6 */
  PieceToken,       /* 7 */
  CondToken,        /* 8 */
  OptToken,         /* 9 */
  RemToken,        /* 10 */
  TraceToken,      /* 11 */
  InputToken,      /* 12 */
  SepToken,        /* 13 */
  TitleToken,      /* 14 */
  TwinProblem,     /* 15 */
  ZeroPosition,    /* 16 */
  LaTeXToken,      /* 17 */
  LaTeXPieces,     /* 18 */
  Award,           /* 19 */
  Array,           /* 20 */
  Forsyth,         /* 21 */

  TokenCount       /* 22 */
} Token;

typedef char PieceChar[2];
typedef PieceChar       PieTable[PieceCount];

/* for multiple variants of conditions */
typedef enum
{
  TypeB,                   /* 0 */
  PionAdverse,             /* 1 */
  AntiCirTypeCheylan,      /* 2 */
  AntiCirTypeCalvet,       /* 3 */
  PionNeutral,             /* 4 */
  PionNoirMaximum,         /* 5 */
  PionBlancMaximum,        /* 6 */
  ParaSent,                /* 7 */
  PionTotalMaximum,        /* 8 */
  SentBerolina,            /* 9 */
  Type1,                  /* 10 */
  Type2,                  /* 11 */
  Type3,                  /* 12 */
  Neighbour,              /* 13 */
  TypeC,                  /* 14 */
  TypeD,                  /* 15 */
  ShiftRank,              /* 16 */
  ShiftFile,              /* 17 */
  ShiftRankFile,          /* 18 */
  Orthogonal,             /* 19 */
  Irregular,              /* 20 */
  ExtraGridLines,         /* 21 */
  Transmuting,            /* 22 */
  WhiteOnly,            /* 23 */
  BlackOnly,            /* 24 */

  VariantTypeCount        /* 25 */
} VariantType;

/* for intelligent specification of one-sided conditions */
typedef enum
{
  maxi,             /* 0 */
  ultraschachzwang,	/* 1 */

  ExtraCondCount    /* 2 */
} ExtraCond;

/* Some remarks to the efficiency of testing bits in C-code
** Look at the following C-fragment:
**      if (a&8 == 0) {
**              do something
**      }
** On a Risc-Maschine this if-statement compiles to three instructions.
** One for loading the variable a (if its not already in a register),
** one for anding with the constant (setting the flags) and the conditional
** jump instruction.
** What's on a CISC (like ix86,68xxx oe) ?
** Here it depends extremly on the compiler !!
** Best code would a thing like the test-instruction of the ix86.
** Then it compiles to this instruction plus the conditinal jump.
** Real code, generated by compilers, loads first the variable a,
** does the test(=and)-instruction and then the conditional jump.
** Is there any improvement possible, when the condition has to be
** testet a lot of times ? - Look at the following C-Code:
**      b= a&8 == 0;            * One-time Initialisation *
**      if (b) {                * This comes more than once *
**              do something
**      }
** The overhead of the initialisation for b is not counted here.
** We look at the if(b). On a RISC machine, there is no change:
** Load the variable b, test it against 0 and jump if appropriate.
** On a CISC ? Best would be the test-Instruction with a conditional jump.
** In this case it generated by most C-compilers. Improvement is according
** to acutal generated code one instruction. But only because of loosy
** optimizers.
** Other results come up, if you use the macro TestStip.
** Here is always one instruction more neccesary:
** Evaluation of the '&' and then '=='. The result of this operation is
** NOT a `boolean' value as a C runtime system would store it in a char,
** or ** use it in the evaluation of a boolean expression. (Especially when
** '^' is used there).
** Therefore if you need to use the result of TestStip quite often in a
** boolean expression, you can gain speed if you assign the result
** of TestStip to a variable, and use the variable in the expression
** instead of TestStip. The net improvement depends on the architecture
** of your machine. On RISC maschines the improvement is not very dramatic,
** but can be on CISC-maschines.
** But this is also question of the optimizer. If the optimizer would work
** a little bit harder, he could recognize this an do it by himself.
** But I do not know any optimizer which is capable of this.
**                                                      ElB 31.9.1991
*/



/* names for options */
/* Please keep the 3 options 14, 15 and 16. I need them for my database project. TLi */
typedef enum
{
  soltout,          /* 0 */
  solapparent,      /* 1 */
  whitetoplay,      /* 2 */
  solvariantes,     /* 3 */
  movenbr,          /* 4 */
  restart,          /* 5 */
  sansrb,           /* 6 */
  duplex,           /* 7 */
  sansrn,           /* 8 */
  solmenaces,       /* 9 */
  nothreat,        /* 10 */
  solessais,       /* 11 */
  maxsols,         /* 12 */
  solflights,      /* 13 */
  search,          /* 14 */
  multi,           /* 15 */
  nosymmetry,      /* 16 */
  nontrivial,      /* 17 */
  keepmating,      /* 18 */
  enpassant,       /* 19 */
  noboard,         /* 20 */
  noshort,         /* 21 */
  halfduplex,      /* 22 */
  postkeyplay,     /* 23 */
  intelligent,     /* 24 */
  maxtime,         /* 25 */
  nocastling,      /* 26 */
  quodlibet,       /* 27 */
  stoponshort,     /* 28 */
  beep,            /* 29 */
  suppressgrid,    /* 30 */
  writegrid,       /* 31 */
  lastcapture,     /* 32 */
  mutuallyexclusivecastling, /* 33 */
  goal_is_end,     /* 34 */
  degeneratetree,  /* 35 */

  OptCount         /* 36 */
} Opt;

/* Names for conditions */
typedef enum
{
  rexincl,                  /* 0 */
  circe,                    /* 1 */
  circemalefique,           /* 2 */
  madras,                   /* 3 */
  volage,                   /* 4 */
  hypervolage,              /* 5 */
  bichro,                   /* 6 */
  monochro,                 /* 7 */
  gridchess,                /* 8 */
  koeko,                    /* 9 */
  blackedge,               /* 10 */
  whiteedge,               /* 11 */
  leofamily,               /* 12 */
  chinoises,               /* 13 */
  patrouille,              /* 14 */
  pwc,                     /* 15 */
  nocapture,               /* 16 */
  immun,                   /* 17 */
  immunmalefique,          /* 18 */
  contactgrid,             /* 19 */
  imitators,               /* 20 */
  cavaliermajeur,          /* 21 */
  haanerchess,             /* 22 */
  chamcirce,               /* 23 */
  couscous,                /* 24 */
  circeequipollents,       /* 25 */
  circefile,               /* 26 */
  blmax,                   /* 27 */
  blmin,                   /* 28 */
  whmax,                   /* 29 */
  whmin,                   /* 30 */
  magicsquare,             /* 31 */
  sentinelles,             /* 32 */
  tibet,                   /* 33 */
  dbltibet,                /* 34 */
  circediagramm,           /* 35 */
  holes,                   /* 36 */
  blcapt,                  /* 37 */
  whcapt,                  /* 38 */
  refl_king,               /* 39 */
  trans_king,              /* 40 */
  blfollow,                /* 41 */
  whfollow,                /* 42 */
  duellist,                /* 43 */
  parrain,                 /* 44 */
  noiprom,                 /* 45 */
  circesymmetry,           /* 46 */
  vogt,                    /* 47 */
  einstein,                /* 48 */
  bicolores,               /* 49 */
  newkoeko,                /* 50 */
  circeclone,              /* 51 */
  anti,                    /* 52 */
  circefilemalefique,      /* 53 */
  circeantipoden,          /* 54 */
  circeclonemalefique,     /* 55 */
  antispiegel,             /* 56 */
  antidiagramm,            /* 57 */
  antifile,                /* 58 */
  antisymmetrie,           /* 59 */
  antispiegelfile,         /* 60 */
  antiantipoden,           /* 61 */
  antiequipollents,        /* 62 */
  immunfile,               /* 63 */
  immundiagramm,           /* 64 */
  immunspiegelfile,        /* 65 */
  immunsymmetry,           /* 66 */
  immunantipoden,          /* 67 */
  immunequipollents,       /* 68 */
  reveinstein,             /* 69 */
  supercirce,              /* 70 */
  degradierung,            /* 71 */
  norsk,                   /* 72 */
  traitor,                 /* 73 */
  andernach,               /* 74 */
  whforsqu,                /* 75 */
  whconforsqu,             /* 76 */
  blforsqu,                /* 77 */
  blconforsqu,             /* 78 */
  chamchess,               /* 79 */
  beamten,                 /* 80 */
  glasgow,                 /* 81 */
  antiandernach,           /* 82 */
  frischauf,               /* 83 */
  circemalefiquevertical,  /* 84 */
  isardam,                 /* 85 */
  ohneschach,              /* 86 */
  circediametral,          /* 87 */
  promotiononly,           /* 88 */
  circerank,               /* 89 */
  exclusive,               /* 90 */
  mars,                    /* 91 */
  marsmirror,              /* 92 */
  phantom,                 /* 93 */
  whrefl_king,             /* 94 */
  blrefl_king,             /* 95 */
  whtrans_king,            /* 96 */
  bltrans_king,            /* 97 */
  antieinstein,            /* 98 */
  couscousmirror,          /* 99 */
  blroyalsq,              /* 100 */
  whroyalsq,              /* 101 */
  brunner,                /* 102 */
  plus,                   /* 103 */
  circeassassin,          /* 104 */
  patience,               /* 105 */
  republican,             /* 106 */
  extinction,             /* 107 */
  central,                /* 108 */
  actrevolving,           /* 109 */
  messigny,               /* 110 */
  woozles,                /* 111 */
  biwoozles,              /* 112 */
  heffalumps,             /* 113 */
  biheffalumps,           /* 114 */
  rexexcl,                /* 115 */
  whprom_sq,              /* 116 */
  blprom_sq,              /* 117 */
  nowhiteprom,            /* 118 */
  noblackprom,            /* 119 */
  eiffel,                 /* 120 */
  blackultraschachzwang,  /* 121 */
  whiteultraschachzwang,  /* 122 */
  arc,                    /* 123 */
  shieldedkings,          /* 124 */
  sting,                  /* 125 */
  linechamchess,          /* 126 */
  nowhcapture,            /* 127 */
  noblcapture,            /* 128 */
  april,                  /* 129 */
  alphabetic,             /* 130 */
  circeturncoats,         /* 131 */
  circedoubleagents,      /* 132 */
  amu,                    /* 133 */
  singlebox,              /* 134 */
  MAFF,                   /* 135 */
  OWU,                    /* 136 */
  white_oscillatingKs,    /* 137 */
  black_oscillatingKs,    /* 138 */
  antikings,              /* 139 */
  antimars,               /* 140 */
  antimarsmirror,         /* 141 */
  antimarsantipodean,     /* 142 */
  whsupertrans_king,      /* 143 */
  blsupertrans_king,      /* 144 */
  antisuper,              /* 145 */
  ultrapatrouille,        /* 146 */
  swappingkings,          /* 147 */
  dynasty,                /* 148 */
  SAT,                    /* 149 */
  strictSAT,              /* 150 */
  takemake,               /* 151 */
  blacksynchron,          /* 152 */
  whitesynchron,          /* 153 */
  blackantisynchron,      /* 154 */
  whiteantisynchron,      /* 155 */
  masand,                 /* 156 */
  BGL,                    /* 157 */
  schwarzschacher,        /* 158 */
  annan,                  /* 159 */
  normalp,                /* 160 */
  lortap,                 /* 161 */
  vault_king,             /* 162 */
  whvault_king,           /* 163 */
  blvault_king,           /* 164 */
  protean,                /* 165 */
  geneva,                 /* 166 */
  champursue,             /* 167 */
  antikoeko,              /* 168 */
  castlingchess,          /* 169 */
  losingchess,            /* 170 */
  disparate,              /* 171 */
  ghostchess,             /* 172 */
  hauntedchess,           /* 173 */
  provocateurs,           /* 174 */
  circecage,              /* 175 */
  immuncage,              /* 176 */
  football,               /* 177 */
  contraparrain,          /* 178 */
  kobulkings,             /* 179 */
  platzwechselrochade,    /* 180 */
  whitealphabetic,	      /* 181 */
  blackalphabetic,	      /* 182 */
  circetakeandmake,       /* 183 */
  superguards,            /* 184 */
  wormholes,              /* 185 */
  marine,                 /* 186 */
  ultramarine,            /* 187 */
  backhome,               /* 188 */

  CondCount               /* 189 */
} Cond;

/* Some remarks to the conditions:
**      if hypervolage is set, also volage will be set
**      if leofamily is set, also chinoises will be set
**      if patrouille is set, also patrcalcule and patrsoutien will
**              be set.
**      if immunmalefique is set, also immun and malefique will be set
**      if pwc is set, also circe will be set
**      if iprom is set, also imitators will be set
**      if equipollents or coucou is set, also pwc will be set
*/

/* Enumeration type for various piece properties
 * Make sure to keep enumerators >= nr_sides
 */
typedef enum
{
  Royal = nr_sides,
  Kamikaze,
  Paralysing,
  Chameleon,
  Jigger,
  Volage,
  Beamtet,
  HalfNeutral,
  ColourChange,
  Protean,
  Magic,
  Uncapturable,
  Patrol,
  FrischAuf,

  PieSpCount
} PieSpec;

typedef Side pileside[maxply+1];

enum
{
  file_rook_queenside,
  file_knight_queenside,
  file_bishop_queenside,
  file_queen,
  file_king,
  file_bishop_kingside,
  file_knight_kingside,
  file_rook_kingside
};

enum
{
  grid_normal,
  grid_vertical_shift,
  grid_horizontal_shift,
  grid_diagonal_shift,
  grid_orthogonal_lines,
  grid_irregular
};

typedef boolean (*nocontactfunc_t)(square);

typedef unsigned int slice_index;

#define encore()        (current_move[nbply] > current_move[nbply-1])
#define advers(camp)    ((camp) ? White : Black)

#define piece_belongs_to_opponent(sq,camp)    TSTFLAG(spec[(sq)],advers(camp))

#define GridLegal(sq1, sq2) (GridNum(sq1) != GridNum(sq2) ||  \
  (numgridlines && CrossesGridLines((sq1), (sq2))))

#endif  /* PY_H */
