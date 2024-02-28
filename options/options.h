#if !defined(OPTIONS_OPTIONS_H)
#define OPTIONS_OPTIONS_H

#include "utilities/boolean.h"

/* names for options */
/* Please keep the 3 options 14, 15 and 16. I need them for my database project. TLi */
typedef enum
{
  soltout,          /* 0 */
  solapparent,      /* 1 */
  whitetoplay,      /* 2 */
  solvariantes,     /* 3 */
  movenbr,          /* 4 */
  startmovenumber,  /* 5 */
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
  mutuallyexclusivecastling,     /* 32 */
  goal_is_end,     /* 33 */
  degeneratetree,  /* 34 */
  nullmoves,       /* 35 */
  uptomovenumber,  /* 36 */

  OptCount         /* 37 */
} Opt;

extern boolean OptFlag[OptCount];

#endif
