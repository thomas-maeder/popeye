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
**************************** End of List ******************************/

#if !defined(PY_H)
#define PY_H

#include <limits.h>

#include "boolean.h"
#include "pyposit.h"

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
#pragma map( flagwhitemummer, "FLAGWH01" )
#pragma map( flagwhiteexact, "FLAGWH02" )
#pragma map( flagblackmummer, "FLAGBL01" )
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
  max_castling = queenside_castling
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
  toppile = 60*maxply,
  maxvec = 232
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
typedef int        numvec;

typedef square       pilecase[maxply+1];

typedef struct {
    square departure;
    square arrival;
    square capture;
} move_generation_elmt;

typedef struct {
    boolean found;
    move_generation_elmt move;
    square mren;
} killer_state;

typedef struct {
  square square;
  piece pc;
} change_rec;

typedef struct {
	square          cdzz, cazz, cpzz;
	square          sqren;
	piece           pjzz, ppri, ren_parrain, norm_prom, cir_prom;

	piece           pjazz;
	square          renkam;
	unsigned int    numi;
	int             sum;
	boolean         promi,
			bool_senti,
			bool_norm_cham_prom,
			bool_cir_cham_prom;
	Side		tr;
	Flags           speci, ren_spec;
	square		repub_k;
	Flags           new_spec;
	square		hurdle;
	square		sb2where;
	piece		sb2what;
	square		sb3where;
	piece		sb3what;
	square		mren;
	boolean		osc;
    change_rec *push_bottom, *push_top;
    square      roch_sq;
    piece       roch_pc;
    Flags       roch_sp;
    piece       ghost_piece;
    Flags       ghost_flags;
  long int bgl_wh, bgl_bl;
} coup;

typedef struct {
	move_generation_elmt move;
    square mren;
	int                  nr_opponent_moves;
} empile_optimization_table_elmt;

enum
{
  empile_optimization_priorize_killmove_by = 5
};

typedef enum {
  move_generation_optimized_by_nr_opponent_moves,
  move_generation_optimized_by_killer_move,
  move_generation_not_optimized
} move_generation_mode_type;


/* These are definitions to implement arrays with lower index != 0
** as they are common in PASCAL. The arrays zzza? are defined in
** pydata.h                                                     ElB
*/
#define nbpiece         (zzzaa - dernoi)

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

/* TODO reduce to 1 piece kind enumeration type */
typedef enum
{
  Invalid,                 /* 0 */
  Empty,                   /* 1 */
  King,                    /* 2 */
  Pawn,                    /* 3 */
  Queen,                   /* 4 */
  Knight,                  /* 5 */
  Rook,                    /* 6 */
  Bishop,                  /* 7 */
  Leo,                     /* 8 */
  Mao,                     /* 9 */
  Pao,                    /* 10 */
  Vao,                    /* 11 */
  Rose,                   /* 12 */
  Grashopper,             /* 13 */
  NightRider,             /* 14 */
  Zebra,                  /* 15 */
  Camel,                  /* 16 */
  Giraffe,                /* 17 */
  RootFiftyLeaper,        /* 18 */
  Bucephale,              /* 19 */
  Wesir,                  /* 20 */
  Alfil,                  /* 21 */
  Fers,                   /* 22 */
  Dabbaba,                /* 23 */
  Lion,                   /* 24 */
  NonStopEquihopper,      /* 25 */
  Locust,                 /* 26 */
  BerolinaPawn,           /* 27 */
  Amazone,                /* 28 */
  Empress,                /* 29 */
  Princess,               /* 30 */
  Gnu,                    /* 31 */
  Antilope,               /* 32 */
  Squirrel,               /* 33 */
  Waran,                  /* 34 */
  Dragon,                 /* 35 */
  Kangaroo,               /* 36 */
  SpiralSpringer,         /* 37 */
  UbiUbi,                 /* 38 */
  Hamster,                /* 39 */
  Elk,                    /* 40 */
  Eagle,                  /* 41 */
  Sparrow,                /* 42 */
  Archbishop,             /* 43 */
  ReflectBishop,          /* 44 */
  Cardinal,               /* 45 */
  NightriderHopper,       /* 46 */
  Dummy,                  /* 47 */
  Camelrider,             /* 48 */
  Zebrarider,             /* 49 */
  Gnurider,               /* 50 */
  CamelHopper,            /* 51 */
  ZebraHopper,            /* 52 */
  GnuHopper,              /* 53 */
  DiagonalSpiralSpringer, /* 54 */
  BouncyKnight,           /* 55 */
  EquiHopper,             /* 56 */
  CAT,                    /* 57 */
  SuperBerolinaPawn,      /* 58 */
  SuperPawn,              /* 59 */
  RookLion,               /* 60 */
  BishopLion,             /* 61 */
  Sirene,                 /* 62 */
  Triton,                 /* 63 */
  Nereide,                /* 64 */
  Orphan,                 /* 65 */
  EdgeHog,                /* 66 */
  Moa,                    /* 67 */
  RookHunter,             /* 68 */
  BishopHunter,           /* 69 */
  MaoRider,               /* 70 */
  MoaRider,               /* 71 */
  RookHopper,             /* 72 */
  BishopHopper,           /* 73 */
  ErlKing,                /* 74 */
  BoyScout,               /* 75 */
  GirlScout,              /* 76 */
  Skylla,                 /* 77 */
  Charybdis,              /* 78 */
  ContraGras,             /* 79 */
  RoseLion,               /* 80 */
  RoseHopper,             /* 81 */
  Okapi,                  /* 82 */
  Leap37,                 /* 83 */
  GrassHopper2,           /* 84 */
  GrassHopper3,           /* 85 */
  Leap16,                 /* 86 */
  Leap24,                 /* 87 */
  Leap35,                 /* 88 */
  DoubleGras,             /* 89 */
  KingHopper,             /* 90 */
  Orix,                   /* 91 */
  Leap15,                 /* 92 */
  Leap25,                 /* 93 */
  Gral,                   /* 94 */
  RookLocust,             /* 95 */
  BishopLocust,           /* 96 */
  NightLocust,            /* 97 */
  WesirRider,             /* 98 */
  FersRider,              /* 99 */
  Bison,                 /* 100 */
  Elephant,              /* 101 */
  Nao,                   /* 102 */
  RookMoose,             /* 103 */
  RookEagle,             /* 104 */
  RookSparrow,           /* 105 */
  BishopMoose,           /* 106 */
  BishopEagle,           /* 107 */
  BishopSparrow,         /* 108 */
  Rao,                   /* 109 */
  Scorpion,              /* 110 */
  Marguerite,            /* 111 */
  Leap36,                /* 112 */
  NightRiderLion,        /* 113 */
  MaoRiderLion,          /* 114 */
  MoaRiderLion,          /* 115 */
  AndernachGrasshopper,  /* 116 */
  Friend,                /* 117 */
  Dolphin,               /* 118 */
  Rabbit,                /* 119 */
  Bob,                   /* 120 */
  EquiEnglish,           /* 121 */
  EquiFrench,            /* 122 */
  Querquisite,           /* 123 */
  Bouncer,               /* 124 */
  RookBouncer,           /* 125 */
  BishopBouncer,         /* 126 */
  ChinesePawn,           /* 127 */
  RadialKnight,          /* 128 */
  ReversePawn,           /* 129 */
  RoseLocust,            /* 130 */
  Zebu,                  /* 131 */
  BouncyNightrider,      /* 132 */
  SpiralSpringer20,      /* 133 */
  SpiralSpringer40,      /* 134 */
  SpiralSpringer11,      /* 135 */
  SpiralSpringer33,      /* 136 */
  Quintessence,          /* 137 */
  DoubleRookHopper,      /* 138 */
  DoubleBishopper,       /* 139 */
  Hunter0,               /* 140 */

  PieceCount             = Hunter0+maxnrhuntertypes
} PieNam;

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

  VariantTypeCount        /* 23 */
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
  degeneratetree,  /* 34 */

  OptCount         /* 35 */
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
  exact,                   /* 73 */
  traitor,                 /* 74 */
  andernach,               /* 75 */
  whforsqu,                /* 76 */
  whconforsqu,             /* 77 */
  blforsqu,                /* 78 */
  blconforsqu,             /* 79 */
  ultra,                   /* 80 */
  chamchess,               /* 81 */
  beamten,                 /* 82 */
  glasgow,                 /* 83 */
  antiandernach,           /* 84 */
  frischauf,               /* 85 */
  circemalefiquevertical,  /* 86 */
  isardam,                 /* 87 */
  ohneschach,              /* 88 */
  circediametral,          /* 89 */
  promotiononly,           /* 90 */
  circerank,               /* 91 */
  exclusive,               /* 92 */
  mars,                    /* 93 */
  marsmirror,              /* 94 */
  phantom,                 /* 95 */
  whrefl_king,             /* 96 */
  blrefl_king,             /* 97 */
  whtrans_king,            /* 98 */
  bltrans_king,            /* 99 */
  antieinstein,           /* 100 */
  couscousmirror,         /* 101 */
  blroyalsq,              /* 102 */
  whroyalsq,              /* 103 */
  brunner,                /* 104 */
  plus,                   /* 105 */
  circeassassin,          /* 106 */
  patience,               /* 107 */
  republican,             /* 108 */
  extinction,             /* 109 */
  central,                /* 110 */
  actrevolving,           /* 111 */
  messigny,               /* 112 */
  woozles,                /* 113 */
  biwoozles,              /* 114 */
  heffalumps,             /* 115 */
  biheffalumps,           /* 116 */
  rexexcl,                /* 117 */
  whprom_sq,              /* 118 */
  blprom_sq,              /* 119 */
  nowhiteprom,            /* 120 */
  noblackprom,            /* 121 */
  eiffel,                 /* 122 */
  blackultraschachzwang,  /* 123 */
  whiteultraschachzwang,  /* 124 */
  arc,                    /* 125 */
  shieldedkings,          /* 126 */
  sting,                  /* 127 */
  linechamchess,          /* 128 */
  nowhcapture,            /* 129 */
  noblcapture,            /* 130 */
  april,                  /* 131 */
  alphabetic,             /* 132 */
  circeturncoats,         /* 133 */
  circedoubleagents,      /* 134 */
  amu,                    /* 135 */
  singlebox,              /* 136 */
  MAFF,                   /* 137 */
  OWU,                    /* 138 */
  white_oscillatingKs,    /* 139 */
  black_oscillatingKs,    /* 140 */
  antikings,              /* 141 */
  antimars,               /* 142 */
  antimarsmirror,         /* 143 */
  antimarsantipodean,     /* 144 */
  whsupertrans_king,      /* 145 */
  blsupertrans_king,      /* 146 */
  antisuper,              /* 147 */
  ultrapatrouille,        /* 148 */
  swappingkings,          /* 149 */
  dynasty,                /* 150 */
  SAT,                    /* 151 */
  strictSAT,              /* 152 */
  takemake,               /* 153 */
  blacksynchron,          /* 154 */
  whitesynchron,          /* 155 */
  blackantisynchron,      /* 156 */
  whiteantisynchron,      /* 157 */
  masand,                 /* 158 */
  BGL,                    /* 159 */
  schwarzschacher,        /* 160 */
  annan,                  /* 161 */
  normalp,                /* 162 */
  lortap,                 /* 163 */
  vault_king,             /* 164 */
  whvault_king,           /* 165 */
  blvault_king,           /* 166 */
  protean,                /* 167 */
  geneva,                 /* 168 */
  champursue,             /* 169 */
  antikoeko,              /* 170 */
  castlingchess,          /* 171 */
  losingchess,            /* 172 */
  disparate,              /* 173 */
  ghostchess,             /* 174 */
  hauntedchess,           /* 175 */
  provacateurs,           /* 176 */
  circecage,              /* 177 */
  immuncage,              /* 178 */
  football,               /* 179 */
  contraparrain,          /* 180 */
  CondCount               /* 181 */
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
  Neutral = nr_sides,
  Kamikaze,
  Royal,
  Paralyse,
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

  PieSpCount
} PieSpec;

typedef Side pileside[maxply+1];

#define PieSpMask ((1<<PieSpCount)-1)


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

enum
{
  NullPieceId = 0,
  MinPieceId = 1,
  MaxPieceId = 100
};

typedef unsigned int        PieceIdType;

#define PieceIdOffset       PieSpCount
#define SetPieceId(spec,id) ((spec) = ((id)<<PieceIdOffset) | ((spec)&PieSpMask))
#define GetPieceId(spec)    ((spec) >> PieceIdOffset)
#define ClearPieceId(spec)  SetPieceId(spec,NullPieceId)

extern square DiaRenSquares[MaxPieceId+1];

extern square PiecePositionsInDiagram[MaxPieceId+1];

#define GetPositionInDiagram(spec)     PiecePositionsInDiagram[GetPieceId(spec)]
#define SavePositionInDiagram(spec,sq) (PiecePositionsInDiagram[GetPieceId(spec)] = (sq))
#define ClearPositionInDiagram(spec)   SavePositionInDiagram(spec,initsquare)

#define FrischAuf       PieSpCount

#define encore()        (nbcou > repere[nbply])
#define advers(camp)    ((camp) ? White : Black)
#define color(piesqu)   (e[(piesqu)]<=roin ? Black : White)

#define COLORFLAGS      (BIT(Black)+BIT(White)+BIT(Neutral))
#define SETCOLOR(a,b)   (a)=((a)&~COLORFLAGS)+((b)&COLORFLAGS)
#define CHANGECOLOR(a)  (a)^=BIT(Black)+BIT(White)

#define imcheck(i, j) (!CondFlag[imitators] || imok((i), (j)))
#define ridimcheck(sq, j, diff) (!CondFlag[imitators] || ridimok((sq), (j), (diff)))

#define hopimcheck(sq, j, over, diff) (!checkhopim || hopimok((sq), (j), (over), (diff), (diff)))
#define hopimmcheck(sq, j, over, diff, diff1) (!checkhopim || hopimok((sq), (j), (over), (diff), (diff1)))
#define maooaimcheck(sq, j, pass) (!CondFlag[imitators] || maooaimok((sq), (j), (pass)))

#define setneutre(i)            do {if (neutcoul != color(i)) change(i);} while(0)
#define change(i)               do {register piece pp; nbpiece[pp= e[(i)]]--; nbpiece[e[(i)]= -pp]++;} while (0)
#define finligne(i,k,p,sq)      do {register int kk= (k); (sq)= (i); while (e[(sq)+=(kk)]==vide); p= e[(sq)];} while (0)

#define rightcolor(ej, camp)    ((camp)==White ? (ej)<=roin : (ej)>=roib)

#define lrhopcheck(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 0, 0, ev)
#define rhopcheck(sq, ka, ke, p, ev)    riderhoppercheck(sq, ka, ke, p, 0, 1, ev)
#define crhopcheck(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 1, 0, ev)

#define gelrhop(sq, ka, ke, camp)       geriderhopper(sq, ka, ke, 0, 0, camp)
#define gerhop(sq, ka, ke, camp)        geriderhopper(sq, ka, ke, 0, 1, camp)
#define gecrhop(sq, ka, ke, camp)       geriderhopper(sq, ka, ke, 1, 0, camp)

#define rhop2check(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 0, 2, ev)
#define gerhop2(sq, ka, ke, camp)      geriderhopper(sq, ka, ke, 0, 2, camp)

#define rhop3check(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 0, 3, ev)
#define gerhop3(sq, ka, ke, camp)      geriderhopper(sq, ka, ke, 0, 3, camp)

#define shopcheck(sq, ka, ke, p, ev)     riderhoppercheck(sq, ka, ke, p, 1, 1, ev)
#define geshop(sq, ka, ke, camp)        geriderhopper(sq, ka, ke, 1, 1, camp)

#define PromSq(col,sq) (TSTFLAG(sq_spec[(sq)],(col)==White?WhPromSq:BlPromSq))
#define ReversePromSq(col,sq) (TSTFLAG(sq_spec[(sq)],(col)==Black?WhPromSq:BlPromSq))

#define ChamCircePiece(p)    ((((p) < vide) ? - NextChamCircePiece[-(p)] : \
                               NextChamCircePiece[(p)]))

#define EndOfLine(from,dir,end)  {end = (from); \
    do (end)+= (dir);                           \
    while (e[(end)] == vide);}

#define LegalAntiCirceMove(reb, cap, dep)  \
  (e[(reb)] == vide \
   || (!AntiCirCheylan && (reb) == (cap)) \
   || ((reb) == (dep)))

#define GridLegal(sq1, sq2) (GridNum(sq1) != GridNum(sq2) ||  \
  (numgridlines && CrossesGridLines((sq1), (sq2))))

#define BGL_infinity 10000000 	/* this will do I expect; e.g. max len = 980 maxply < 1000 */
#define PushMagic(sq, id1, id2, v) \
{if (nbmagic < magicviews_size) \
  {magicviews[nbmagic].piecesquare=(sq); \
  magicviews[nbmagic].pieceid=(id1); \
  magicviews[nbmagic].magicpieceid=(id2); \
  magicviews[nbmagic++].vecnum=(v);}\
 else {FtlMsg(5);}}

#define PushChangedColour(stack, limit, sq, pie) \
  {if ((stack) - (limit) < 0) {\
    (stack)->square=(sq); \
    (stack)->pc=(pie); \
    (stack)++;}\
  else {flag_outputmultiplecolourchanges=false;}}

#define ENEMYOBS(sq) \
  (TSTFLAG(spec[sq], Beamtet))
#define ENEMYANTI(sq) \
  (false)
#define FRIENDOBS(sq) \
  (TSTFLAG(spec[sq], Patrol))
#define FRIENDANTI(sq) \
  (false)

#endif  /* PY_H */
