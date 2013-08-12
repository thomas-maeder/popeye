#if !defined(POSITION_POSITION_H)
#define POSITION_POSITION_H

#include "position/board.h"
#include "pieces/walks/vectors.h"
#include "utilities/boolean.h"

/* Declarations of types and functions related to chess positions
 */

enum
{
  maxnrhuntertypes = 10
};

typedef enum
{
  Empty,                   /* 0 */
  Invalid,                 /* 1 */
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
  Grasshopper,            /* 13 */
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
  CamelRiderHopper,            /* 51 */
  ZebraRiderHopper,            /* 52 */
  GnuRiderHopper,              /* 53 */
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
  Friend,                /* 116 */
  Dolphin,               /* 117 */
  Rabbit,                /* 118 */
  Bob,                   /* 119 */
  EquiStopper,           /* 120 */
  NonstopEquiStopper,            /* 121 */
  Querquisite,           /* 122 */
  Bouncer,               /* 123 */
  RookBouncer,           /* 124 */
  BishopBouncer,         /* 125 */
  ChinesePawn,           /* 126 */
  RadialKnight,          /* 127 */
  ReversePawn,           /* 128 */
  RoseLocust,            /* 129 */
  Zebu,                  /* 130 */
  BouncyNightrider,      /* 131 */
  SpiralSpringer20,      /* 132 */
  SpiralSpringer40,      /* 133 */
  SpiralSpringer11,      /* 134 */
  SpiralSpringer33,      /* 135 */
  Quintessence,          /* 136 */
  DoubleRookHopper,      /* 137 */
  DoubleBishopper,       /* 138 */
  NonStopOrix,           /* 139 */
  Treehopper,            /* 140 */
  Leafhopper,            /* 141 */
  GreaterTreehopper,     /* 142 */
  GreaterLeafhopper,     /* 143 */
  KangarooLion,          /* 144 */
  Kao,                   /* 145 */
  KnightHopper,          /* 146 */
  Ship,                  /* 147 */
  Gryphon,               /* 148 */
  MarineKnight,          /* 149 */
  Poseidon,              /* 150 */
  MarinePawn,            /* 151 */
  MarineShip,            /* 152 */
  Hunter0,               /* 153 */

  PieceCount             = Hunter0+maxnrhuntertypes
} PieNam;

/* Array containing an element for each square plus many slack square
 * for making move generation easier
 */
typedef PieNam echiquier[maxsquare+4];

/* Set of flags representing characteristics of pieces (e.g. being
 * Neutral, or being Uncapturable)
 */
typedef unsigned long Flags;

/* Some useful symbols for dealing with these flags
 */

/* Enumeration type for the two sides which move, deliver mate etc.
 */

#include "position/board.h"

#define ENUMERATION_TYPENAME Side
#define ENUMERATORS \
  ENUMERATOR(White), \
    ENUMERATOR(Black), \
                       \
    ENUMERATOR(nr_sides), \
    ASSIGNED_ENUMERATOR(no_side = nr_sides)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

typedef enum
{
  color_white,
  color_black,
  color_neutral,

  nr_colors
} Colors;

#define COLORFLAGS      (BIT(color_black)+BIT(color_white)+BIT(color_neutral))
#define SETCOLOR(a,b)   (a)=((a)&~COLORFLAGS)+((b)&COLORFLAGS)

extern echiquier e;
extern Flags spec[maxsquare+4];
extern square king_square[nr_sides];
extern boolean areColorsSwapped;
extern boolean isBoardReflected;

enum
{
  BorderSpec = 0,
  EmptySpec  = 0
};

enum
{
  maxinum    = 10    /* maximum supported number of imitators */
};

typedef square imarr[maxinum]; /* squares currently occupied by imitators */

/* Structure containing the pieces of data that together represent a
 * position.
 */
typedef struct
{
    echiquier board;                     /* placement of the pieces */
    Flags spec[maxsquare+4];      /* spec[s] contains flags for piece board[i]*/
    square king_square[nr_sides];        /* placement of the kings */
    unsigned int inum;                   /* number of iterators */
    imarr isquare;                       /* placement of iterators */
    unsigned number_of_pieces[nr_sides][PieceCount]; /* number of piece kind */
} position;


/* Sequence of pieces corresponding to the game array (a1..h1, a2..h2
 * ... a8..h8)
 */
extern PieNam const PAS[nr_squares_on_board];
extern Side const PAS_sides[nr_squares_on_board];

/* Initial game position.
 *
 */
extern position const game_array;

/* Initialize the game array into a position object
 * @param pos address of position object
 */
void initialise_game_array(position *pos);

/* Swap the sides of all the pieces */
void swap_sides(void);

/* Reflect the position at the horizontal central line */
void reflect_position(void);

void empty_square(square s);
void block_square(square s);
void occupy_square(square s, PieNam piece, Flags flags);
void replace_piece(square s, PieNam piece);
#define is_square_empty(s) (e[(s)]==Empty)
#define is_square_blocked(s) (e[(s)]==Invalid)
#define get_walk_of_piece_on_square(s) (e[(s)])
square find_end_of_line(square from, numvec dir);

/* Change the side of some piece specs
 * @param spec address of piece specs where to change the side
 */
void piece_change_side(Flags *spec);

#endif
