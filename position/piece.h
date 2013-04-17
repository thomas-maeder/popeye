#if !defined(POSITION_PIECE_H)
#define POSITION_PIECE_H


/* Declarations of types and functions related to chess pieces
 */

enum
{
  maxnrhuntertypes = 10
};

/* Type representing piece kinds
 */
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
  friendb,        /* 116 */
  dolphinb,       /* 117 */
  rabbitb,        /* 118 */
  bobb,           /* 119 */
  equiengb,       /* 120 */
  equifrab,       /* 121 */
  querqub,        /* 122 */
  bouncerb,       /* 123 */
  rookbouncerb,   /* 124 */
  bishopbouncerb, /* 125 */
  chinesepawnb,   /* 126 */
  radialknightb,  /* 127 */
  reversepb,      /* 128 */
  roselocustb,    /* 129 */
  zebub,          /* 130 */
  refnb,          /* 131 */
  cs20b,           /* 132 */
  cs40b,           /* 133 */
  cs11b,           /* 134 */
  cs33b,           /* 135 */
  cs31b,           /* 136 */
  doublerookhopperb, /* 137 */
  doublebishopperb,  /* 138 */
  norixb,          /* 139 */
  treehopperb,     /* 140 */
  leafhopperb,     /* 141 */
  greatertreehopperb,     /* 142 */
  greaterleafhopperb,     /* 143 */
  kanglionb,     /* 144 */
  kaob,          /* 145 */
  knighthopperb, /* 146 */
  shipb,         /* 147 */
  gryphonb,      /* 148 */
  marineknightb, /* 149 */
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
  doublerookhoppern = -doublerookhopperb,
  doublebishoppern = -doublebishopperb,
  norixn         = -norixb,
  treehoppern    = -treehopperb,
  leafhoppern    = -leafhopperb,
  greatertreehoppern    = -greatertreehopperb,
  greaterleafhoppern    = -greaterleafhopperb,
  kanglionn      = -kanglionb,
  kaon           = -kaob,
  knighthoppern  = -knighthopperb,
  shipn          = -shipb,
  gryphonn       = -gryphonb,
  marineknightn  = -marineknightb,
  hunter0n       = -hunter0b,
  dernoi         = -derbla

} piece;

#endif
