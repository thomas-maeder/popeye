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
  rexincl,                  /* 0 */
  circe,                    /* 1 */
  circemirror,              /* 2 */
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
  immunmirror,             /* 18 */
  contactgrid,             /* 19 */
  imitators,               /* 20 */
  cavaliermajeur,          /* 21 */
  haanerchess,             /* 22 */
  circechameleon,               /* 23 */
  circecouscous,                /* 24 */
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
  circeparrain,                 /* 44 */
  noiprom,                 /* 45 */
  circesymmetry,           /* 46 */
  vogt,                    /* 47 */
  einstein,                /* 48 */
  bicolores,               /* 49 */
  newkoeko,                /* 50 */
  circeclone,              /* 51 */
  anticirce,                    /* 52 */
  circefilemirror,         /* 53 */
  circeantipoden,          /* 54 */
  circeclonemirror,        /* 55 */
  antimirror,              /* 56 */
  antidiagramm,            /* 57 */
  antifile,                /* 58 */
  antisymmetrie,           /* 59 */
  antimirrorfile,          /* 60 */
  antiantipoden,           /* 61 */
  antiequipollents,        /* 62 */
  immunfile,               /* 63 */
  immundiagramm,           /* 64 */
  immunmirrorfile,         /* 65 */
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
  circemirrorvertical,     /* 84 */
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
  circecouscousmirror,          /* 99 */
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
  circecontraparrain,          /* 178 */
  kobulkings,             /* 179 */
  platzwechselrochade,    /* 180 */
  whitealphabetic,        /* 181 */
  blackalphabetic,        /* 182 */
  circetakeandmake,       /* 183 */
  superguards,            /* 184 */
  wormholes,              /* 185 */
  marine,                 /* 186 */
  ultramarine,            /* 187 */
  backhome,               /* 188 */
  facetoface,             /* 189 */
  backtoback,             /* 190 */
  cheektocheek,           /* 191 */
  chameleonsequence,      /* 192 */
  anticlonecirce,         /* 193 */

  CondCount               /* 194 */
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

extern boolean anymars;
extern boolean anyantimars;
extern boolean calc_reflective_king[nr_sides];
extern boolean anyparrain;

#endif
