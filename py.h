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
**************************** End of List ******************************/

#if !defined(PY_H)
#define PY_H

#include <limits.h>

#include "boolean.h"

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
#  if defined(DATABASE)
#    define OSTYPE "DATABASE"
#  elseif defined(C370)
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

typedef enum
{
  rot90,        /* 0 */
  rot180,       /* 1 */
  rot270,       /* 2 */
  mirra1h1,     /* 3 */
  mirra1a8,     /* 4 */
  mirra1h8,     /* 5 */
  mirra8h1      /* 6 */
} SquareTransformation;

enum
{
  maxnrhuntertypes = 10
};

typedef enum
{
  vide,             /* 0 */
  obs,              /* 1 */

  roib,             /* 2 */
  pb,               /* 3 */
  db,               /* 4 */
  cb,               /* 5 */
  tb,               /* 6 */
  fb,               /* 7 */
  leob,             /* 8 */
  maob,             /* 9 */
  paob,            /* 10 */
  vaob,            /* 11 */
  roseb,           /* 12 */
  sb,              /* 13 */
  nb,              /* 14 */
  zb,              /* 15 */
  chb,             /* 16 */
  gib,             /* 17 */
  rccinqb,         /* 18 */
  bub,             /* 19 */
  vizirb,          /* 20 */
  alfilb,          /* 21 */
  fersb,           /* 22 */
  dabb,            /* 23 */
  lionb,           /* 24 */
  nequib,          /* 25 */
  locb,            /* 26 */
  pbb,             /* 27 */
  amazb,           /* 28 */
  impb,            /* 29 */
  princb,          /* 30 */
  gnoub,           /* 31 */
  antilb,          /* 32 */
  ecurb,           /* 33 */
  waranb,          /* 34 */
  dragonb,         /* 35 */
  kangoub,         /* 36 */
  csb,             /* 37 */
  ubib,            /* 38 */
  hamstb,          /* 39 */
  mooseb,          /* 40 */
  eagleb,          /* 41 */
  sparrb,          /* 42 */
  archb,           /* 43 */
  reffoub,         /* 44 */
  cardb,           /* 45 */
  nsautb,          /* 46 */
  dummyb,          /* 47 */
  camridb,         /* 48 */
  zebridb,         /* 49 */
  gnuridb,         /* 50 */
  camhopb,         /* 51 */
  zebhopb,         /* 52 */
  gnuhopb,         /* 53 */
  dcsb,            /* 54 */
  refcb,           /* 55 */
  equib,           /* 56 */
  catb,            /* 57 */
  bspawnb,         /* 58 */
  spawnb,          /* 59 */
  tlionb,          /* 60 */
  flionb,          /* 61 */
  sireneb,         /* 62 */
  tritonb,         /* 63 */
  nereidb,         /* 64 */
  orphanb,         /* 65 */
  edgehb,          /* 66 */
  moab,            /* 67 */
  rhuntb,          /* 68 */
  bhuntb,          /* 69 */
  maoridb,         /* 70 */
  moaridb,         /* 71 */
  rookhopb,        /* 72 */
  bishophopb,      /* 73 */
  ekingb,          /* 74 */
  bscoutb,         /* 75 */
  gscoutb,         /* 76 */
  skyllab,         /* 77 */
  charybdisb,      /* 78 */
  contragrasb,     /* 79 */
  roselionb,       /* 80 */
  rosehopperb,     /* 81 */
  okapib,          /* 82 */
  leap37b,         /* 83 */
  g2b,             /* 84 */
  g3b,             /* 85 */
  leap16b,         /* 86 */
  leap24b,         /* 87 */
  leap35b,         /* 88 */
  doublegb,        /* 89 */
  khb,             /* 90 */
  orixb,           /* 91 */
  leap15b,         /* 92 */
  leap25b,         /* 93 */
  gralb,           /* 94 */
  rooklocustb,     /* 95 */
  bishoplocustb,   /* 96 */
  nightlocustb,    /* 97 */
  vizridb,         /* 98 */
  fersridb,        /* 99 */
  bisonb,         /* 100 */
  elephantb,      /* 101 */
  naob,           /* 102 */
  rookmooseb,     /* 103 */
  rookeagleb,     /* 104 */
  rooksparrb,     /* 105 */
  bishopmooseb,   /* 106 */
  bishopeagleb,   /* 107 */
  bishopsparrb,   /* 108 */
  raob,           /* 109 */
  scorpionb,      /* 110 */
  margueriteb,    /* 111 */
  leap36b,        /* 112 */
  nrlionb	,       /* 113 */
  mrlionb	,       /* 114 */
  molionb,        /* 115 */
  andergb,        /* 116 */
  friendb,        /* 117 */
  dolphinb,       /* 118 */
  rabbitb,        /* 119 */
  bobb,           /* 120 */
  equiengb,       /* 121 */
  equifrab,       /* 122 */
  querqub,        /* 123 */
  bouncerb,       /* 124 */
  rookbouncerb,   /* 125 */
  bishopbouncerb, /* 126 */
  chinesepawnb,   /* 127 */
  radialknightb,  /* 128 */
  reversepb,      /* 129 */
  roselocustb,    /* 130 */
  zebub,          /* 131 */
  refnb,          /* 132 */
  cs20b,           /* 133 */
  cs40b,           /* 134 */
  cs11b,           /* 135 */
  cs33b,           /* 136 */
  cs31b,           /* 137 */
  hunter0b,       
  derbla =        hunter0b+maxnrhuntertypes,

  roin           = -roib,
  pn             = -pb,
  dn             = -db,
  cn             = -cb,
  tn             = -tb,
  fn             = -fb,
  leon           = -leob,
  maon           = -maob,
  paon           = -paob,
  vaon           = -vaob,
  rosen          = -roseb,
  sn             = -sb,
  nn             = -nb,
  zn             = -zb,
  chn            = -chb,
  gin            = -gib,
  rccinqn        = -rccinqb,
  bun            = -bub,
  vizirn         = -vizirb,
  alfiln         = -alfilb,
  fersn          = -fersb,
  dabn           = -dabb,
  lionn          = -lionb,
  nequin         = -nequib,
  locn           = -locb,
  pbn            = -pbb,
  amazn          = -amazb,
  impn           = -impb,
  princn         = -princb,
  gnoun          = -gnoub,
  antiln         = -antilb,
  ecurn          = -ecurb,
  warann         = -waranb,
  dragonn        = -dragonb,
  kangoun        = -kangoub,
  csn            = -csb,
  ubin           = -ubib,
  hamstn         = -hamstb,
  moosen         = -mooseb,
  eaglen         = -eagleb,
  sparrn         = -sparrb,
  archn          = -archb,
  reffoun        = -reffoub,
  cardn          = -cardb,
  nsautn         = -nsautb,
  dummyn         = -dummyb,
  camridn        = -camridb,
  zebridn        = -zebridb,
  gnuridn        = -gnuridb,
  camhopn        = -camhopb,
  zebhopn        = -zebhopb,
  gnuhopn        = -gnuhopb,
  dcsn           = -dcsb,
  refcn          = -refcb,
  equin          = -equib,
  catn           = -catb,
  bspawnn        = -bspawnb,
  spawnn         = -spawnb,
  tlionn         = -tlionb,
  flionn         = -flionb,
  sirenen        = -sireneb,
  tritonn        = -tritonb,
  nereidn        = -nereidb,
  orphann        = -orphanb,
  edgehn         = -edgehb,
  moan           = -moab,
  rhuntn         = -rhuntb,
  bhuntn         = -bhuntb,
  maoridn        = -maoridb,
  moaridn        = -moaridb,
  rookhopn       = -rookhopb,
  bishophopn     = -bishophopb,
  ekingn         = -ekingb,
  bscoutn        = -bscoutb,
  gscoutn        = -gscoutb,
  skyllan        = -skyllab,
  charybdisn     = -charybdisb,
  contragrasn    = -contragrasb,
  roselionn      = -roselionb,
  rosehoppern    = -rosehopperb,
  okapin         = -okapib,
  leap37n        = -leap37b,
  g2n            = -g2b,
  g3n            = -g3b,
  leap16n        = -leap16b,
  leap24n        = -leap24b,
  leap35n        = -leap35b,
  doublegn       = -doublegb,
  khn            = -khb,
  orixn          = -orixb,
  leap15n        = -leap15b,
  leap25n        = -leap25b,
  graln          = -gralb,
  rooklocustn    = -rooklocustb,
  bishoplocustn  = -bishoplocustb,
  nightlocustn   = -nightlocustb,
  vizridn        = -vizridb,
  fersridn       = -fersridb,
  bisonn         = -bisonb,
  elephantn      = -elephantb,
  naon           = -naob,
  rookmoosen     = -rookmooseb,
  rookeaglen     = -rookeagleb,
  rooksparrn     = -rooksparrb,
  bishopmoosen   = -bishopmooseb,
  bishopeaglen   = -bishopeagleb,
  bishopsparrn   = -bishopsparrb,
  raon           = -raob,
  scorpionn      = -scorpionb,
  margueriten    = -margueriteb,
  leap36n        = -leap36b,
  nrlionn        = -nrlionb,
  mrlionn        = -mrlionb,
  molionn        = -molionb,
  andergn        = -andergb,
  friendn        = -friendb,
  dolphinn       = -dolphinb,
  rabbitn        = -rabbitb,
  bobn           = -bobb,
  equiengn       = -equiengb,
  equifran       = -equifrab,
  querqun        = -querqub,
  bouncern       = -bouncerb,
  rookbouncern   = -rookbouncerb,
  bishopbouncern = -bishopbouncerb,
  chinesepawnn   = -chinesepawnb,  
  radialknightn  = -radialknightb,
  reversepn      = -reversepb,
  roselocustn    = -roselocustb,
  zebun          = -zebub,
  refnn          = -refnb,
  cs20n          = -cs20b,
  cs40n          = -cs40b,
  cs11n          = -cs11b,
  cs33n          = -cs33b,
  cs31n          = -cs31b,
  hunter0n       = -hunter0b,
  dernoi         = -derbla
  
} piece;

enum
{
  /* For reasons of code simplification of move generation, square a1
   * doesn't have index 0; there are some slack rows at the top and
   * bottom of the board, and some slack files at the left and right.
   */
  nr_of_slack_files_left_of_board = 8,
  nr_of_slack_rows_below_board = 8,

  nr_files_on_board = 8,
  nr_rows_on_board = 8,

  bottom_row = nr_of_slack_rows_below_board,
  top_row = bottom_row+nr_rows_on_board-1,

  left_file = nr_of_slack_files_left_of_board,
  right_file = left_file+nr_files_on_board-1,

  /* if square1-square2==onerow, then square1 is one row higher than
   * square2 */
  onerow = (nr_of_slack_files_left_of_board
            +nr_files_on_board
            +nr_of_slack_files_left_of_board),

  nr_squares_on_board = nr_files_on_board*nr_rows_on_board,

  maxsquare = ((nr_of_slack_rows_below_board
                +nr_rows_on_board
                +nr_of_slack_rows_below_board)
               *onerow),

  /* how many steps can a straight rider maximally make and still
   * remain on the board? */
  max_nr_straight_rider_steps = 7
};

/* Symbols for geometric calculations - please ALWAYS use these rather
 * than int literals */
enum
{
  dir_left  =   -1,
  dir_right =   +1,

  dir_up    =   onerow,
  dir_down  =  -onerow
};

/* Symbols for squares - using these makes code much more human-readable */
enum
{
  square_a1 = (nr_of_slack_rows_below_board*onerow
               +nr_of_slack_files_left_of_board),
  square_b1,
  square_c1,
  square_d1,
  square_e1,
  square_f1,
  square_g1,
  square_h1,

  square_a2 = square_a1+dir_up,
  square_b2,
  square_c2,
  square_d2,
  square_e2,
  square_f2,
  square_g2,
  square_h2,

  square_a3 = square_a2+dir_up,
  square_b3,
  square_c3,
  square_d3,
  square_e3,
  square_f3,
  square_g3,
  square_h3,

  square_a4 = square_a3+dir_up,
  square_b4,
  square_c4,
  square_d4,
  square_e4,
  square_f4,
  square_g4,
  square_h4,

  square_a5 = square_a4+dir_up,
  square_b5,
  square_c5,
  square_d5,
  square_e5,
  square_f5,
  square_g5,
  square_h5,

  square_a6 = square_a5+dir_up,
  square_b6,
  square_c6,
  square_d6,
  square_e6,
  square_f6,
  square_g6,
  square_h6,

  square_a7 = square_a6+dir_up,
  square_b7,
  square_c7,
  square_d7,
  square_e7,
  square_f7,
  square_g7,
  square_h7,

  square_a8 = square_a7+dir_up,
  square_b8,
  square_c8,
  square_d8,
  square_e8,
  square_f8,
  square_g8,
  square_h8
};

enum
{
  /* the following values are used instead of capture square
   * to indicate special moves */
  messigny_exchange = maxsquare+1,
  kingside_castling = maxsquare+2,
  queenside_castling = maxsquare+3
};

typedef unsigned int stip_length_type;

enum
{
#if defined(DATABASE)
  maxply = 302
#else
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
#endif /* DATABASE */
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

enum
{
  initsquare = 0,      /* to initialize square-variables */
  maxinum    = 10,    /* max. number of imitators */
  nullsquare = 1
};

/* Enumeration type for the two sides which move, deliver mate etc.
 */
typedef enum
{
  White,
  Black,

  nr_sides,
  no_side = nr_sides
} Side;

/* These typedefs should be judged for efficiency */

typedef unsigned long   Flags;

typedef int        square;
typedef int         numecoup;
typedef int         ply;
typedef int        numvec;

typedef piece        echiquier[maxsquare+4];
typedef square       pilecase[maxply+1];

typedef struct {
    square departure;
    square arrival;
    square capture;
} move_generation_elmt;

typedef struct {
    boolean found;
    move_generation_elmt move;
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
			echec,
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
} coup;

typedef struct {
	move_generation_elmt move;
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


typedef square imarr[maxinum]; /* squares currently occupied by imitators */

/* These are definitions to implement arrays with lower index != 0
** as they are common in PASCAL. The arrays zzza? are defined in
** pydata.h                                                     ElB
*/
#define nbpiece         (zzzaa - dernoi)

typedef enum
{
  MagicSq,         /* 0 */
  WhForcedSq,      /* 1 */
  BlForcedSq,      /* 2 */
  WhConsForcedSq,  /* 3 */
  BlConsForcedSq,  /* 4 */
  NoEdgeSq,        /* 5 */
  SqColor,         /* 6 */
  WhPromSq,        /* 7 */
  BlPromSq,        /* 8 */
  Grid,            /* 9 */

  nrSquareFlags
} SquareFlags;

#define sq_spec         (zzzan - square_a1)
#define sq_num          (zzzao - square_a1)
#define NoEdge(i)       TSTFLAG(sq_spec[(i)], NoEdgeSq)
#define SquareCol(i)    TSTFLAG(sq_spec[(i)], SqColor)
#define GridNum(s)      (sq_spec[(s)] >> Grid)
#define ClearGridNum(s) (sq_spec[(s)] &= ((1<<Grid)-1))

#define BIT(pos)                (1<<(pos))
#define TSTFLAG(bits,pos)       (((bits)&BIT(pos))!=0)
#define CLRFLAG(bits,pos)       ((bits)&= ~BIT(pos))
#define SETFLAG(bits,pos)       ((bits)|= BIT(pos))
#define CLEARFL(bits)           ((bits)=0)

#define TSTFLAGMASK(bits,mask)  ((bits)&(mask))
#define CLRFLAGMASK(bits,mask)  ((bits) &= ~(mask))
#define SETFLAGMASK(bits,mask)  ((bits) |= (mask))

#include "pylang.h"

typedef enum
{
  BeginProblem,    /* 0 */
  EndProblem,      /* 1 */
  NextProblem,     /* 2 */
  StipToken,       /* 3 */
  Author,          /* 4 */
  Origin,          /* 5 */
  PieceToken,      /* 6 */
  CondToken,       /* 7 */
  OptToken,        /* 8 */
  RemToken,        /* 9 */
  TraceToken,     /* 10 */
  InputToken,     /* 11 */
  SepToken,       /* 12 */
  TitleToken,     /* 13 */
  TwinProblem,    /* 14 */
  ZeroPosition,   /* 15 */
  LaTeXToken,     /* 16 */
  LaTeXPieces,    /* 17 */
  Award,          /* 18 */
  Array,          /* 19 */
  Forsyth,        /* 20 */

  TokenCount      /* 21 */
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
  Hunter0,               /* 138 */

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

  OptCount        /* 32 */
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
  CondCount               /* 176 */
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

  PieSpCount
} PieSpec;

typedef Side pileside[maxply+1];

enum
{
  BorderSpec = 0,
  EmptySpec  = BIT(Black)+BIT(White)
};


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

#define DiaCirce        PieSpCount
#define DiaRen(s)       (boardnum[((s) >> DiaCirce)])
#define DiaRenMask      ((1<<DiaCirce)-1)
#define SetDiaRen(s, f) ((s)=((unsigned int)((((f)-square_a1)/onerow)*8+((f)-square_a1)%onerow)<<DiaCirce) + ((s)&DiaRenMask))
#define FrischAuf       PieSpCount

/* needed for Twinning Reset. */
#define ClrDiaRen(s)    ((s)-=((unsigned int)((s)>>DiaCirce)<<DiaCirce))

#define encore()        (nbcou > repere[nbply])
#define advers(camp)    ((camp) ? White : Black)
#define color(piesqu)   (e[(piesqu)]<=roin ? Black : White)

#define coupfort()      {kpilcd[nbply]= move_generation_stack[nbcou].departure; kpilca[nbply]= move_generation_stack[nbcou].arrival;}


#define COLORFLAGS      (BIT(Black)+BIT(White)+BIT(Neutral))
#define SETCOLOR(a,b)   (a)=((a)&~COLORFLAGS)+((b)&COLORFLAGS)
#define CHANGECOLOR(a)  (a)^=BIT(Black)+BIT(White)

#define imcheck(i, j) (!CondFlag[imitators] || imok((i), (j)))
#define ridimcheck(sq, j, diff) (!CondFlag[imitators] || ridimok((sq), (j), (diff)))

#define hopimcheck(sq, j, over, diff) (!checkhopim || hopimok((sq), (j), (over), (diff)))
#define maooaimcheck(sq, j, pass) (!CondFlag[imitators] || maooaimok((sq), (j), (pass)))   

#define setneutre(i)            do {if (neutcoul != color(i)) change(i);} while(0)
#define change(i)               do {register piece pp; nbpiece[pp= e[(i)]]--; nbpiece[e[(i)]= -pp]++;} while (0)
#define finligne(i,k,p,sq)      do {register int kk= (k); (sq)= (i); while (e[(sq)+=(kk)]==vide); p= e[(sq)];} while (0)

#define rightcolor(ej, camp)    ((camp)==White ? (ej)<=roin : (ej)>=roib)

#define lrhopcheck(ply_id, sq, ka, ke, p, ev)   riderhoppercheck(ply_id, sq, ka, ke, p, 0, 0, ev)
#define rhopcheck(ply_id, sq, ka, ke, p, ev)    riderhoppercheck(ply_id, sq, ka, ke, p, 0, 1, ev)
#define crhopcheck(ply_id, sq, ka, ke, p, ev)   riderhoppercheck(ply_id, sq, ka, ke, p, 1, 0, ev)

#define gelrhop(sq, ka, ke, camp)       geriderhopper(sq, ka, ke, 0, 0, camp)
#define gerhop(sq, ka, ke, camp)        geriderhopper(sq, ka, ke, 0, 1, camp)
#define gecrhop(sq, ka, ke, camp)       geriderhopper(sq, ka, ke, 1, 0, camp)

#define rhop2check(ply_id, sq, ka, ke, p, ev)   riderhoppercheck(ply_id, sq, ka, ke, p, 0, 2, ev)
#define gerhop2(sq, ka, ke, camp)      geriderhopper(sq, ka, ke, 0, 2, camp)

#define rhop3check(ply_id, sq, ka, ke, p, ev)   riderhoppercheck(ply_id, sq, ka, ke, p, 0, 3, ev)
#define gerhop3(sq, ka, ke, camp)      geriderhopper(sq, ka, ke, 0, 3, camp)

#define shopcheck(ply_id, sq, ka, ke, p, ev)     riderhoppercheck(ply_id, sq, ka, ke, p, 1, 1, ev)
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

#endif  /* PY_H */
