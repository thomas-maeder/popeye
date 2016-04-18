#if !defined(CONDITIONS_CONDITIONS_H)
#define CONDITIONS_CONDITIONS_H

#include "position/position.h"

/* Some remarks to the conditions:
**      if hypervolage is set, also volage will be set
**      if leofamily is set, also chinoises will be set
**      if patrouille is set, also patrcalcule and patrsoutien will
**              be set.
**      if immunmirror is set, also immun and mirror will be set
**      if pwc is set, also circe will be set
**      if iprom is set, also imitators will be set
**      if equipollents or coucou is set, also pwc will be set
*/


/* Names for conditions */
typedef enum
{
  circe,                    /* 0 */
  circemirror,              /* 1 */
  madras,                   /* 2 */
  volage,                   /* 3 */
  hypervolage,              /* 4 */
  bichro,                   /* 5 */
  monochro,                 /* 6 */
  gridchess,                /* 7 */
  koeko,                    /* 8 */
  blackedge,                /* 9 */
  whiteedge,               /* 10 */
  leofamily,               /* 11 */
  chinoises,               /* 12 */
  patrouille,              /* 13 */
  pwc,                     /* 14 */
  nocapture,               /* 15 */
  immun,                   /* 16 */
  immunmirror,             /* 17 */
  contactgrid,             /* 18 */
  imitators,               /* 19 */
  cavaliermajeur,          /* 20 */
  haanerchess,             /* 21 */
  circechameleon,          /* 22 */
  circecouscous,           /* 23 */
  circeequipollents,       /* 24 */
  circefile,               /* 25 */
  blmax,                   /* 26 */
  blmin,                   /* 27 */
  whmax,                   /* 28 */
  whmin,                   /* 29 */
  magicsquare,             /* 30 */
  sentinelles,             /* 31 */
  tibet,                   /* 32 */
  dbltibet,                /* 33 */
  circediagramm,           /* 34 */
  holes,                   /* 35 */
  blcapt,                  /* 36 */
  whcapt,                  /* 37 */
  refl_king,               /* 38 */
  trans_king,              /* 39 */
  blfollow,                /* 40 */
  whfollow,                /* 41 */
  duellist,                /* 42 */
  circeparrain,            /* 43 */
  noiprom,                 /* 44 */
  circesymmetry,           /* 45 */
  vogt,                    /* 46 */
  einstein,                /* 47 */
  bicolores,               /* 48 */
  newkoeko,                /* 49 */
  circeclone,              /* 50 */
  anticirce,               /* 51 */
  circefilemirror,         /* 52 */
  circeantipoden,          /* 53 */
  circeclonemirror,        /* 54 */
  antimirror,              /* 55 */
  antidiagramm,            /* 56 */
  antifile,                /* 57 */
  antisymmetrie,           /* 58 */
  antimirrorfile,          /* 59 */
  antiantipoden,           /* 60 */
  antiequipollents,        /* 61 */
  immunfile,               /* 62 */
  immundiagramm,           /* 63 */
  immunmirrorfile,         /* 64 */
  immunsymmetry,           /* 65 */
  immunantipoden,          /* 66 */
  immunequipollents,       /* 67 */
  reveinstein,             /* 68 */
  supercirce,              /* 69 */
  degradierung,            /* 70 */
  norsk,                   /* 71 */
  traitor,                 /* 72 */
  andernach,               /* 73 */
  whforsqu,                /* 74 */
  whconforsqu,             /* 75 */
  blforsqu,                /* 76 */
  blconforsqu,             /* 77 */
  chamchess,               /* 78 */
  beamten,                 /* 79 */
  glasgow,                 /* 80 */
  antiandernach,           /* 81 */
  frischauf,               /* 82 */
  circemirrorvertical,     /* 83 */
  isardam,                 /* 84 */
  ohneschach,              /* 85 */
  circediametral,          /* 86 */
  promotiononly,           /* 87 */
  circerank,               /* 88 */
  exclusive,               /* 89 */
  mars,                    /* 90 */
  marsmirror,              /* 91 */
  phantom,                 /* 92 */
  whrefl_king,             /* 93 */
  blrefl_king,             /* 94 */
  whtrans_king,            /* 95 */
  bltrans_king,            /* 96 */
  antieinstein,            /* 97 */
  circecouscousmirror,     /* 98 */
  blroyalsq,               /* 99 */
  whroyalsq,              /* 100 */
  brunner,                /* 101 */
  plus,                   /* 102 */
  circeassassin,          /* 103 */
  patience,               /* 104 */
  republican,             /* 105 */
  extinction,             /* 106 */
  central,                /* 107 */
  actrevolving,           /* 108 */
  messigny,               /* 109 */
  woozles,                /* 110 */
  biwoozles,              /* 111 */
  heffalumps,             /* 112 */
  biheffalumps,           /* 113 */
  whprom_sq,              /* 114 */
  blprom_sq,              /* 115 */
  nowhiteprom,            /* 116 */
  noblackprom,            /* 117 */
  eiffel,                 /* 118 */
  blackultraschachzwang,  /* 119 */
  whiteultraschachzwang,  /* 120 */
  arc,                    /* 121 */
  shieldedkings,          /* 122 */
  sting,                  /* 123 */
  linechamchess,          /* 124 */
  nowhcapture,            /* 125 */
  noblcapture,            /* 126 */
  april,                  /* 127 */
  alphabetic,             /* 128 */
  circeturncoats,         /* 129 */
  circedoubleagents,      /* 130 */
  amu,                    /* 131 */
  singlebox,              /* 132 */
  MAFF,                   /* 133 */
  OWU,                    /* 134 */
  white_oscillatingKs,    /* 135 */
  black_oscillatingKs,    /* 136 */
  antikings,              /* 137 */
  antimars,               /* 138 */
  antimarsmirror,         /* 139 */
  antimarsantipodean,     /* 140 */
  whsupertrans_king,      /* 141 */
  blsupertrans_king,      /* 142 */
  antisuper,              /* 143 */
  ultrapatrouille,        /* 144 */
  swappingkings,          /* 145 */
  dynasty,                /* 146 */
  SAT,                    /* 147 */
  strictSAT,              /* 148 */
  takemake,               /* 149 */
  blacksynchron,          /* 150 */
  whitesynchron,          /* 151 */
  blackantisynchron,      /* 152 */
  whiteantisynchron,      /* 153 */
  masand,                 /* 154 */
  BGL,                    /* 155 */
  schwarzschacher,        /* 156 */
  annan,                  /* 157 */
  normalp,                /* 158 */
  lortap,                 /* 159 */
  vault_king,             /* 160 */
  whvault_king,           /* 161 */
  blvault_king,           /* 162 */
  protean,                /* 163 */
  geneva,                 /* 164 */
  champursue,             /* 165 */
  antikoeko,              /* 166 */
  castlingchess,          /* 167 */
  losingchess,            /* 168 */
  disparate,              /* 169 */
  ghostchess,             /* 170 */
  hauntedchess,           /* 171 */
  provocateurs,           /* 172 */
  circecage,              /* 173 */
  dummy,                  /* 174 */
  football,               /* 175 */
  circecontraparrain,     /* 176 */
  kobulkings,             /* 177 */
  platzwechselrochade,    /* 178 */
  whitealphabetic,        /* 179 */
  blackalphabetic,        /* 180 */
  circetakeandmake,       /* 181 */
  superguards,            /* 182 */
  wormholes,              /* 183 */
  marine,                 /* 184 */
  ultramarine,            /* 185 */
  backhome,               /* 186 */
  facetoface,             /* 187 */
  backtoback,             /* 188 */
  cheektocheek,           /* 189 */
  chameleonsequence,      /* 190 */
  anticlonecirce,         /* 191 */
  snekchess,              /* 192 */
  snekcirclechess,        /* 193 */
  lastcapture,            /* 194 */
  argentinian,            /* 195 */

  CondCount               /* 196 */
} Cond;

/* for intelligent specification of one-sided conditions */
typedef enum
{
  maxi,             /* 0 */
  ultraschachzwang, /* 1 */

  ExtraCondCount    /* 2 */
} ExtraCond;

typedef enum
{
  ConditionTypeA,
  ConditionTypeB,
  ConditionTypeC,
  ConditionTypeD,

  ConditionLetteredVariantTypeCount
} ConditionLetteredVariantType;

typedef enum
{
  ConditionType1,
  ConditionType2,
  ConditionType3,

  ConditionNumberedVariantTypeCount
} ConditionNumberedVariantType;

extern boolean CondFlag[CondCount];
extern boolean ExtraCondFlag[ExtraCondCount];
extern square royal_square[nr_sides];
extern boolean calc_reflective_king[nr_sides];

#endif
