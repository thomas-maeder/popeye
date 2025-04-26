#if !defined(PIECES_PIECES_H)
#define PIECES_PIECES_H

#include "utilities/boolean.h"

enum
{
  max_nr_hunter_walks = 10
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
  CamelRiderHopper,       /* 51 */
  ZebraRiderHopper,       /* 52 */
  GnuRiderHopper,         /* 53 */
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
  NonstopEquiStopper,    /* 121 */
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
  Sting,                 /* 153 */
  Senora,                /* 154 */
  Faro,                  /* 155 */
  Loco,                  /* 156 */
  Saltador,              /* 157 */
  MaoHopper,             /* 158 */
  MoaHopper,             /* 159 */
  ContraHamster,         /* 160 */
  EagleEquihopper,       /* 161 */
  EagleNonstopEquihopper,/* 162 */
  Hunter0,               /* 163 */

  nr_piece_walks = Hunter0+max_nr_hunter_walks
} piece_walk_type;

/* Enumeration type for various piece properties
 */
typedef enum
{
  piece_flag_white,
  piece_flag_black,
  Royal,
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
  Bul,
  Dob,
  Anda,

  nr_piece_flags
} piece_flag_type;

/* Set of flags representing characteristics of pieces (e.g. being
 * Neutral, or being Uncapturable)
 */
typedef unsigned long Flags;

extern Flags some_pieces_flags;
extern Flags all_pieces_flags;
extern Flags all_royals_flags;

extern boolean piece_walk_exists[nr_piece_walks];
extern boolean piece_walk_may_exist[nr_piece_walks];
extern boolean piece_walk_may_exist_fairy;

#endif
