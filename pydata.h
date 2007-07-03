/********************** MODIFICATIONS to pydata.h **************************
** This is the list of modifications done to pydata.h
**
** Date       Who  What
**
** 2006/07/30 SE   New condition: Schwarzschacher  
**
** 2007/01/28 SE   New condition: NormalPawn  
**
** 2007/01/28 SE   New condition: Annan Chess 
**
** 2007/04/30 NG   sentinelles variants: naming changed for consistency ...
**
** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
**
** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
**
**************************** End of List ******************************/

#ifndef PYDATA_H
#define PYDATA_H

#ifndef EXTERN
#       define EXTERN extern
#       define WE_ARE_EXTERN
#endif  /* EXTERN */

#ifdef  WE_ARE_EXTERN
extern  unsigned long   MaxMemory;
extern  char    *StartUp;
extern  long    MaxPositions;
extern  boolean LaTeXout;
extern  int     GlobalThreadCounter;	/* V3.54  NG */
extern  boolean flag_regression;	/* V3.74  NG */
#else
#	ifdef __TURBOC__ /* TurboC and BorlandC  TLi */
   /* MaxPositions is either set in inithash() using the coreleft function
   ** or entered with the -maxpos option at the DOS prompt.    TLi
   */
   long         MaxPositions;
#	else
   long         MaxPositions    = 1000000000L;
#	endif
unsigned long           MaxMemory       = 0L;
char            *StartUp = VERSIONSTRING;
boolean         LaTeXout= false;
int     	GlobalThreadCounter= 0;	/* V3.54  NG */
boolean         flag_regression= false;	/* V3.74  NG */
#endif  /* WE_ARE_EXTERN */

EXTERN  char            MMString[37];   /* MaxMemory available for diagnostic message */
					/* Safety first, think at the future (:-), V3.37  NG */

EXTERN  long            PositionCnt;

EXTERN  boolean         FlagGenMatingMove, FlagMoveOrientatedStip;    /* V3.33 TLi */
EXTERN  boolean         flagpassive, flagcapture,       /* V3.46  SE/TLi */
			flagactive,                     /* V3.47  NG */
			flagwhitemummer,
			flagblackmummer;                /* V3.0  TLi */

EXTERN  square          rb, rn;         /* V3.12  TLi */
EXTERN  square          bl_royal_sq, wh_royal_sq;       /* V3.50  TLi */

EXTERN  echiquier       e, e_ubi, e_ubi_mad;    /* V3.12c  TLi, V3.29  NG */

EXTERN  boolean         mateallowed[maxply + 1];        /* V3.45  TLi */
EXTERN  boolean         senti[maxply + 1],
			norm_cham_prom[maxply + 1],     /* V3.1  TLi */
			cir_cham_prom[maxply + 1];      /* V3.1  TLi */

EXTERN struct /* V3.72 TM */
{
    square where;
    piece what;
} sb2[maxply+1];

EXTERN  square          ppridia[maxply + 1];            /* V2.90c  TLi */

EXTERN  square
			ep[maxply + 1],                 /* V3.0  TLi */
			ep2[maxply + 1],                /* V3.22 TLi */
			whduell[maxply + 1],            /* V3.0  TLi */
			blduell[maxply + 1],            /* V3.0  TLi */
			RN_[maxply + 1],                /* V3.02 TLi */    /* H.D. 10.02.93 */
			RB_[maxply + 1];                /* V3.03 TLi */

EXTERN  square          pattfld, patti, pattj;          /* V2.70c  TLi */
EXTERN move_generation_elmt move_generation_stack[toppile + 1];

#ifdef  WE_ARE_EXTERN
EXTERN smallint maxinbox[];  /* V3.71 TM */
EXTERN unsigned int boxsize; /* V3.71 TM */
#endif

EXTERN struct /* V3.71 TM */
{
    square where;
    piece what;
} sb3[toppile+1];

EXTERN  numecoup        nbcou;
EXTERN  ply             nbply;

EXTERN  killer_state current_killer_state;

#ifdef  WE_ARE_EXTERN
EXTERN  killer_state const null_killer_state;
#endif

EXTERN  pilecase        kpilcd;
EXTERN  pilecase        kpilca;

EXTERN  smallint        maxflights;             /* V3.12  TLi */


/* symbols for bits in castling_flag */   /* V3.35  NG */  /* V4.02  TM */
enum
{
  rh8_cancastle = 0x01,
  ra8_cancastle = 0x02,
  ke8_cancastle = 0x04,
  rh1_cancastle = 0x10,
  ra1_cancastle = 0x20,
  ke1_cancastle = 0x40
};
/* NOTE: ke[18]_cancastle must be larger than the respective
 * r[ah][18]_cancastle or evaluations of the form
 * TSTFLAGMASK(castling_flag[nbply],wh_castlings)<=ke1_cancastle
 * stop working. */

/* symbols for bit combinations in castling_flag */ /* V4.02  TM */
enum
{
  whk_castling = ke1_cancastle|rh1_cancastle,
  whq_castling = ke1_cancastle|ra1_cancastle,
  wh_castlings = ke1_cancastle|ra1_cancastle|rh1_cancastle,
  blk_castling = ke8_cancastle|rh8_cancastle,
  blq_castling = ke8_cancastle|ra8_cancastle,
  bl_castlings = ke8_cancastle|ra8_cancastle|rh8_cancastle  
};

EXTERN  unsigned char   castling_flag[maxply + 1];
EXTERN  unsigned char   no_castling;		/* V3.55  NG */
EXTERN  short           castling_supported;     /* V3.35  NG */
EXTERN  boolean         testcastling;		/* V3.62  NG */

/* Symbols for squares - using these makes code much more human-readable */
/* V4.02 TM */
enum
{
  square_a1 = 200,
  square_b1 = 201,
  square_c1 = 202,
  square_d1 = 203,
  square_e1 = 204,
  square_f1 = 205,
  square_g1 = 206,
  square_h1 = 207,

  square_a2 = 224,
  square_b2 = 225,
  square_c2 = 226,
  square_d2 = 227,
  square_e2 = 228,
  square_f2 = 229,
  square_g2 = 230,
  square_h2 = 231,

  square_a3 = 248,

  square_a4 = 272,
  square_d4 = 275,
  square_e4 = 276,
  square_h4 = 279,

  square_a5 = 296,
  square_d5 = 299,
  square_e5 = 300,
  square_h5 = 303,

  square_h6 = 327,

  square_a7 = 344,
  square_b7 = 345,
  square_c7 = 346,
  square_d7 = 347,
  square_e7 = 348,
  square_f7 = 349,
  square_g7 = 350,
  square_h7 = 351,

  square_a8 = 368,
  square_b8 = 369,
  square_c8 = 370,
  square_d8 = 371,
  square_e8 = 372,
  square_f8 = 373,
  square_g8 = 374,
  square_h8 = 375,

  /* the following values are used instead of capture square
   * to indicate special moves */
  messigny_exchange = maxsquare+1,
  kingside_castling = maxsquare+2,
  queenside_castling = maxsquare+3
};

/* Symbols for geometric calculations - please ALWAYS use these rather
 * than int literals */
enum {
  dir_left=   -1,
  dir_right=  +1,

  dir_up=     24,
  dir_down=  -24
};


/* Stop solving when a given number of solutions is reached */

EXTERN  int             solutions, maxsolutions,
                        sol_per_matingpos, maxsol_per_matingpos;
EXTERN  boolean         FlagMaxSolsReached;	/* V3.60  NG */
EXTERN  boolean         FlagShortSolsReached;	/* V3.60  NG */

/* Stop solving when a given time (in seconds) for solving is reached */

EXTERN  int             maxsolvingtime;		/* V3.53  NG */
EXTERN  boolean         FlagTimeOut;		/* V3.54  NG */
EXTERN  boolean         FlagTimerInUse;		/* V3.54  NG */

/* beep maxbeep times if a solution is found */

EXTERN  int             maxbeep;		/* V3.77  NG */

/* Optimierung direkte Aufgaben   V3.02  TLi */

EXTERN  empile_optimization_table_elmt       empile_optimization_table[100];
EXTERN  int             empile_optimization_table_count;
EXTERN  move_generation_mode_type  move_generation_mode;
EXTERN  move_generation_mode_type move_generation_mode_opti_per_couleur[2];
		
EXTERN  numvec          ortho_opt_queen[2*(haut-bas)+1],
			ortho_opt_rook[2*(haut-bas)+1],
			ortho_opt_bishop[2*(haut-bas)+1],
			ortho_opt_knight[2*(haut-bas)+1];
#define CheckDirQueen  (ortho_opt_queen+(haut-bas))
#define CheckDirRook   (ortho_opt_rook+(haut-bas))
#define CheckDirBishop (ortho_opt_bishop+(haut-bas))
#define CheckDirKnight (ortho_opt_knight+(haut-bas))
EXTERN  boolean         totalortho;

EXTERN  smallint        zzzaa[derbla - dernoi + 1];     /* nbpiece */

EXTERN  piece           pjoue[maxply + 1], pprise[maxply + 1],
			norm_prom[maxply + 1], cir_prom[maxply + 1],
			ren_parrain[maxply + 1],        /* V3.02  TLi */
			jouearr[maxply + 1];            /* V3.1  TLi */

EXTERN  boolean         flende,                 /* flagpatrsout,        */
			SatzFlag,DrohFlag,              
			rex_mad,rex_circe,rex_immun,rex_phan,    /* V2.70c,2.80c  TLi, V3.51  NG  */
                        rex_mess_ex, rex_wooz_ex;       /* V3.55  TLi */
EXTERN  boolean         pwcprom[maxply + 1];            /* V1.6c  NG */

EXTERN  pilecase        sqrenais,
			crenkam,        /* rebirth square for taking kamikaze pieces V2.4d  TM */
			super;          /* supercirce  V3.1  TLi */
EXTERN  Flags           jouespec[maxply + 1];    /* V2.4d  TM */
EXTERN  numecoup        repere[maxply + 1];

EXTERN  boolean         exist[derbla + 1];              /* V3.43  NG */
EXTERN  boolean         promonly[derbla + 1];           /* V3.43  NG */

EXTERN  evalfunction_t *eval_white;
EXTERN  evalfunction_t *eval_2;
EXTERN  evalfunction_t *eval_black;
EXTERN boolean		(* ReciStipulation)(couleur),   /* V3.31  TLi */
		(* NonReciStipulation)(couleur), /* V3.31  TLi */
		(* stipulation)(couleur);       /* V2.90c  TLi */
EXTERN  short   (* white_length)(square departure, square arrival, square capture), /* V3.0  TLi */
		(* black_length)(square departure, square arrival, square capture);       /* V3.0  TLi */

typedef square  (* renaifunc)(piece, Flags, square, square, square, couleur);

EXTERN  renaifunc immunrenai, circerenai, antirenai, marsrenai;/* V3.1  TLi */

EXTERN  boolean         anycirce, anycirprom, anyanticirce, anyimmun, anyclone;  /* V3.1  TLi */

/* new anticirce implementation -- V.38  TLi */
EXTERN  boolean         AntiCirCheylan;   /* V3.38  TLi */
EXTERN  boolean         SingleBoxType;    /* V3.73  NG */
EXTERN  boolean         sbtype1, sbtype2, sbtype3;    /* V3.73  NG */

EXTERN  pilecouleur     trait;
EXTERN  boolean         flagfee,                /* V2.51  NG */
			flagriders, flagleapers, flagsimplehoppers, 
            flagsimpledecomposedleapers, flagsymmetricfairy,  /* V3.81 SE */
			flagveryfairy,          /* V3.12  TM */
			flagleofamilyonly,      /* V3.77  NG */
			empilegenre,            /* V2.51  NG */
			flaglegalsquare, jouegenre,     /* V2.60  NG  V2.70c  TLi */
			repgenre, change_moving_piece;  /* V3.1  TLi */

EXTERN  square          cirrenroib, cirrenroin,         /* V3.1  TLi */
			immrenroib, immrenroin;

EXTERN  boolean         PrintReciSolution;              /* V3.31  TLi */

EXTERN  smallint        marge;
EXTERN  numecoup        debut;
EXTERN  numecoup        testdebut;                      /* V3.00  NG */
EXTERN  tab             tabsol;                         /* V2.70c  TLi */

EXTERN  Flags           pprispec[maxply + 1];

EXTERN  couleur         zzzag[haut - bas + 1];  /* sqcolor */           /* V2.60  NG */

EXTERN  Flags           spec[maxsquare+4];     /* V3.33  TLi, V3.55  TLi */
EXTERN  smallint        zzzaj[haut - bas + 1];  /* gridnumber */        /* V2.60  NG */

EXTERN  smallint        NonTrivialNumber, NonTrivialLength;           /* V3.32  TLi */

/*****  V3.20  TLi  *****  begin  *****/
EXTERN  Flags           zzzan[haut - bas + 1];  /* sq_spec */
EXTERN  boolean         we_generate_exact, there_are_consmoves,
			wh_exact, bl_exact, wh_ultra, bl_ultra; /* V3.22  TLi */
/*****  V3.20  TLi  *****   end   *****/

							       /* V4.2d  TM */
EXTERN  smallint        inum[maxply + 1];       /* aktuelle Anzahl Imitatoren */

EXTERN  imarr           isquare;                /* Imitatorstandfelder */

EXTERN  boolean         Iprom[maxply + 1];      /* Imitatorumwandlung? */
EXTERN  square          im0;                    /* Standort des ersten Imitators */

EXTERN  smallint        enonce,
			zugebene, maxdefen, droh;         /* V2.1c  TLi */
EXTERN  smallint        introenonce;            /* V3.44  TLi */
/* EXTERN  smallint        numenroute;    V3.44  TLi */
EXTERN  smallint        MoveNbr, RestartNbr;            /* V3.44  TLi */
EXTERN  boolean         restartenabled;                 /* V3.44  TLi */
EXTERN  couleur         neutcoul;
EXTERN  boolean         calctransmute,                  /* V3.0  TLi */
			orph_refking;                   /* V3.0  TLi */

EXTERN  boolean         flag_testlegality, k_cap,       /* V3.44  SE/TLi */
			flag_writinglinesolution,       /* V3.44  TLi */
			anymars,                        /* V3.46  SE/TLi */
			flag_madrasi;                   /* V3.60  TLi */
EXTERN  boolean         is_phantomchess;                /* V3.47  NG */
EXTERN  square          marsid;                         /* V3.46  SE/TLi */
EXTERN  square          TargetField, ReciTargetField, NonReciTargetField;   /* V3.31  TLi */

EXTERN  boolean         DoubleMate, CounterMate, ReciDoubleMate, NonReciDoubleMate;        /* V3.32  TLi */

EXTERN  piece           getprompiece[derbla + 1];       /* it's a inittable !  V2.60  NG */
EXTERN  piece           checkpieces[derbla - leob + 1]; /* only fairies !  V2.60  NG */

EXTERN  piece           transmpieces[derbla];           /* V3.0  TLi */
EXTERN  piece           orphanpieces[derbla + 1];       /* V3.0  TLi */

EXTERN  boolean         ProofFairy;                     /* V3.36  TLi */

EXTERN  couleur         maincamp;                       /* V3.42  NG */

EXTERN  piece           NextChamCircePiece[PieceCount];  /* V3.45  TLi */
EXTERN  boolean         InitChamCirce;                   /* V3.45  TLi */

EXTERN unsigned char    more_ren;      					    /* V3.50 SE */
EXTERN boolean          IsardamB,PatienceB,SentPionAdverse,SentPionNeutral,
			flagAssassin,flag_nk,jouetestgenre;				 /* V3.50 SE */
EXTERN boolean          flagdiastip, flag_dontaddk;      /* V3.50  SE */
EXTERN boolean          mate_or_stale_patt;		/* V3.60  SE */
EXTERN smallint         max_pn, max_pb;            	/* V3.60  SE */
EXTERN boolean          flagmaxi, flagultraschachzwang, flagparasent; /* V3.62 SE */
EXTERN smallint         max_pt;               /* V3.63  SE */
EXTERN boolean          isapril[derbla + 1];

EXTERN boolean          checkhopim;		/* V3.64 SE */
EXTERN square           chop[toppile + 1];	/* V3.64 SE */

EXTERN piece            sentinelb, sentineln;	/* V3.70 SE */
EXTERN boolean          anytraitor;		/* V3.70 SE */
EXTERN boolean          att_1[maxply + 1];	/* V3.70 SE */
EXTERN boolean          flag_atob;		/* V3.70 SE */
EXTERN boolean         (*nocontactfunc) (square ia);
EXTERN boolean		OscillatingKingsTypeB[2], OscillatingKingsTypeC[2];
EXTERN boolean		anyantimars;
EXTERN square           cmren[toppile + 1];
EXTERN square           supertrans[maxply+1];
EXTERN piece            current_trans_gen;
EXTERN piece            ctrans[toppile+1];
EXTERN boolean          nonkilgenre;
EXTERN square           superbas;
EXTERN boolean          complex_castling_through_flag;
EXTERN boolean          oscillatedKs[toppile + 1];
EXTERN boolean          SATCheck, satXY, dont_generate_castling;
EXTERN smallint         WhiteSATFlights, BlackSATFlights;
EXTERN boolean          WhiteStrictSAT[maxply+1],BlackStrictSAT[maxply+1];
EXTERN square           takemake_departuresquare;
EXTERN square           takemake_capturesquare;
EXTERN boolean          repub_k[toppile + 1];
EXTERN boolean          flag_appseul;
EXTERN smallint         zzzao[haut - bas + 1];
EXTERN boolean          flag_synchron;
EXTERN long int         BGL_black, BGL_white; /* V4.06 SE */
EXTERN boolean          BGL_global, BGL_blackinfinity, BGL_whiteinfinity;
EXTERN boolean			  	blacknull, nullgenre, whitenull;		/* V3.70 SE */
EXTERN smallint         annanvar;
EXTERN numecoup         tempcoup;
EXTERN ply      	      tempply;
EXTERN boolean          jouetestgenre1, jouetestgenre_save;

#ifdef WE_ARE_EXTERN
	extern PieTable PieNamString[LangCount];
#else
	PieTable PieNamString[LangCount] = {
	{ /* French PieNamString */
	/*  0*/ {'.',' '}, /* vide */
	/*  1*/ {' ',' '}, /* hors echiquier */
	/*  2*/ {'r',' '}, /* roi */
	/*  3*/ {'p',' '}, /* pion */
	/*  4*/ {'d',' '}, /* dame */
	/*  5*/ {'c',' '}, /* cavalier */
	/*  6*/ {'t',' '}, /* tour */
	/*  7*/ {'f',' '}, /* fou */
	/*  8*/ {'l','e'}, /* leo */
	/*  9*/ {'m','a'}, /* mao */
	/* 10*/ {'p','a'}, /* pao */
	/* 11*/ {'v','a'}, /* vao */
	/* 12*/ {'r','o'}, /* rose */
	/* 13*/ {'s',' '}, /* sauterelle */
	/* 14*/ {'n',' '}, /* noctambule */
	/* 15*/ {'z',' '}, /* zebre */
	/* 16*/ {'c','h'}, /* chameau */
	/* 17*/ {'g','i'}, /* girafe */
	/* 18*/ {'c','c'}, /* cavalier racine carree cinquante */
	/* 19*/ {'b','u'}, /* bucephale  (cheval d'Alexandre le Grand) */
	/* 20*/ {'v','i'}, /* vizir */
	/* 21*/ {'a','l'}, /* alfil */
	/* 22*/ {'f','e'}, /* fers */
	/* 23*/ {'d','a'}, /* dabbabba */
	/* 24*/ {'l','i'}, /* lion */
	/* 25*/ {'e','q'}, /* equisauteur (non-stop) */
	/* 26*/ {'l',' '}, /* locuste */
	/* 27*/ {'p','b'}, /* pion berolina */
	/* 28*/ {'a','m'}, /* amazone */
	/* 29*/ {'i','m'}, /* imperatrice */
	/* 30*/ {'p','r'}, /* princesse */
	/* 31*/ {'g',' '}, /* gnou */
	/* 32*/ {'a','n'}, /* antilope */
	/* 33*/ {'e','c'}, /* ecureuil */
	/* 34*/ {'v',' '}, /* varan */
	/* 35*/ {'d','r'}, /* dragon */
	/* 36*/ {'k','a'}, /* kangourou */
	/* 37*/ {'c','s'}, /* cavalier spirale */
	/* 38*/ {'u','u'}, /* UbiUbi */
	/* 39*/ {'h','a'}, /* hamster: sauterelle a 180 degre */
	/* 40*/ {'e',' '}, /* elan: sauterelle a 45 degre */
	/* 41*/ {'a','i'}, /* aigle: sauterelle a 90 degre */
	/* 42*/ {'m',' '}, /* moineaux: sauterelle a 135 degre */
	/* 43*/ {'a','r'}, /* archeveque */
	/* 44*/ {'f','r'}, /* fou rebondissant */
	/* 45*/ {'c','a'}, /* cardinal */
	/* 46*/ {'s','n'}, /* noctambule sauteur */
	/* 47*/ {'p','i'}, /* piece impuissant */
	/* 48*/ {'c','n'}, /* chameau noctambule */
	/* 49*/ {'z','n'}, /* zebre noctambule */
	/* 50*/ {'g','n'}, /* gnou noctambule */
	/* 51*/ {'s','c'}, /* chameau sauteur */
	/* 52*/ {'s','z'}, /* zebre sauteur */
	/* 53*/ {'s','g'}, /* gnou sauteur */
	/* 54*/ {'c','d'}, /* cavalier spirale diagonale */
	/* 55*/ {'c','r'}, /* cavalier rebondissant */
	/* 56*/ {'e','a'}, /* equisauteur anglais */
	/* 57*/ {'c','t'}, /* CAT (= cavalier de troie) */
	/* 58*/ {'b','s'}, /* berolina superpion */
	/* 59*/ {'s','p'}, /* superpion */
	/* 60*/ {'t','l'}, /* tour-lion */
	/* 61*/ {'f','l'}, /* fou-lion */
	/* 62*/ {'s','i'}, /* sirene */
	/* 63*/ {'t','r'}, /* triton */
	/* 64*/ {'n','e'}, /* nereide */
	/* 65*/ {'o',' '}, /* orphan */
	/* 66*/ {'e','h'}, /* "edgehog", "randschwein" */
	/* 67*/ {'m','o'}, /* moa */
	/* 68*/ {'t','c'}, /* tour/fou chasseur */
	/* 69*/ {'f','c'}, /* fou/tour chasseur */
	/* 70*/ {'a','o'}, /* noctambule mao */
	/* 71*/ {'o','a'}, /* noctambule moa */
	/* 72*/ {'s','t'}, /* sauterelle tour */
	/* 73*/ {'s','f'}, /* sauterelle fou */
	/* 74*/ {'r','e'}, /* roi des elfes */
	/* 75*/ {'b','t'}, /* boy-scout */
	/* 76*/ {'g','t'}, /* girl-scout */
	/* 77*/ {'s','k'}, /* skylla */
	/* 78*/ {'c','y'}, /* charybdis */
	/* 79*/ {'s','a'}, /* sauterelle contA */
	/* 80*/ {'r','l'}, /* rose lion */
	/* 81*/ {'r','s'}, /* rose sauteur */
	/* 82*/ {'o','k'}, /* okapi */
	/* 83*/ {'3','7'}, /* 3:7-cavalier */
	/* 84*/ {'s','2'}, /* sauterelle-2 */
	/* 85*/ {'s','3'}, /* sauterelle-3 */
	/* 86*/ {'1','6'}, /* 1:6-cavalier */
	/* 87*/ {'2','4'}, /* 2:4-cavalier */
	/* 88*/ {'3','5'}, /* 3:5-cavalier */
	/* 89*/ {'d','s'}, /* double sauterelle */
	/* 90*/ {'s','r'}, /* roi sauteur */
	/* 91*/ {'o','r'}, /* orix */
	/* 92*/ {'1','5'}, /* 1:5-cavalier */
	/* 93*/ {'2','5'}, /* 2:5-cavalier */
	/* 94*/ {'g','l'}, /* gral */
        /* 95*/ {'l','t'}, /* tour locuste */
        /* 96*/ {'l','f'}, /* fou locuste */
        /* 97*/ {'l','n'}, /* noctambule locuste */
        /* 98*/ {'v','s'}, /* vizir sauteur */
        /* 99*/ {'f','s'}, /* fers sauteur */
	/*100*/ {'b','i'}, /* bison */
	/*101*/ {'e','l'}, /* elephant */
	/*102*/ {'n','a'}, /* Nao */
	/*103*/ {'e','t'}, /* elan tour: sauterelle tour a 45 degre */
	/*104*/ {'a','t'}, /* aigle tour: sauterelle tour a 90 degre */
	/*105*/ {'m','t'}, /* moineaux tour: sauterelle tour a 135 degre */
	/*106*/ {'e','f'}, /* elan fou: sauterelle fou a 45 degre */
	/*107*/ {'a','f'}, /* aigle fou: sauterelle fou a 90 degre */
	/*108*/ {'m','f'}, /* moineaux fou: sauterelle fou a 135 degre */
	/*109*/ {'r','a'}, /* rao: rose chinois */
	/*110*/ {'s','o'}, /* scorpion: roi des elfes et sauterelle */
	/*111*/ {'m','g'}, /* marguerite */
	/*112*/ {'3','6'}, /* 3:6-cavalier */
        /*113*/ {'n','l'}, /* noctambule lion */
        /*114*/ {'m','l'}, /* noctambule mao lion */
        /*115*/ {'m','m'}, /* noctambule moa lion */
        /*116*/ {'a','s'}, /* sauterelle d'Andernach */
        /*117*/ {'a',' '}, /* ami */
        /*118*/ {'d','n'}, /* dauphin */
        /*119*/ {'l','a'}, /* lapin */
        /*120*/ {'b','o'}, /* bob */
        /*121*/ {'q','a'}, /* equi anglais */
        /*122*/ {'q','f'}, /* equi francais */
        /*123*/ {'q','q'}, /* querquisite */
        /*124*/ {'b','1'}, /* bouncer */
        /*125*/ {'b','2'}, /* tour-bouncer */
        /*126*/ {'b','3'}, /* fou-bouncer */
        /*127*/ {'p','c'}  /* pion chinois */
	},{ /* German PieNamString */
	/*  0*/ {'.',' '}, /* leer */
	/*  1*/ {' ',' '}, /* ausserhalb des Brettes */
	/*  2*/ {'k',' '}, /* Koenig     0,1 + 1,1 S */
	/*  3*/ {'b',' '}, /* Bauer */
	/*  4*/ {'d',' '}, /* Dame       0,1 + 1,1 R */
	/*  5*/ {'s',' '}, /* Springer   1,2 S */
	/*  6*/ {'t',' '}, /* Turm       0,1 R */
	/*  7*/ {'l',' '}, /* Laeufer    1,1 R */
	/*  8*/ {'l','e'}, /* Leo */
	/*  9*/ {'m','a'}, /* Mao */
	/* 10*/ {'p','a'}, /* Pao */
	/* 11*/ {'v','a'}, /* Vao */
	/* 12*/ {'r','o'}, /* Rose */
	/* 13*/ {'g',' '}, /* Grashuepfer 0,1 + 1,1 H */
	/* 14*/ {'n',' '}, /* Nachtreiter 1,2 R */
	/* 15*/ {'z',' '}, /* Zebra       2,3 S */
	/* 16*/ {'c','a'}, /* Kamel       1,3 S */
	/* 17*/ {'g','i'}, /* Giraffe     1,4 S */
	/* 18*/ {'w','f'}, /* Wurzel-50-Springer 5,5 + 1,7 S */
	/* 19*/ {'b','u'}, /* Wurzel-25-Springer 3,4 + 5,0 S  (Bukephalos: Pferd Alexanders des Grossen) */
	/* 20*/ {'w','e'}, /* Wesir */
	/* 21*/ {'a','l'}, /* Alfil */
	/* 22*/ {'f','e'}, /* Fers */
	/* 23*/ {'d','a'}, /* Dabbaba */
	/* 24*/ {'l','i'}, /* Lion */
	/* 25*/ {'n','e'}, /* Non-stop Equihopper */
	/* 26*/ {'h',' '}, /* Heuschrecke */
	/* 27*/ {'b','b'}, /* Berolina Bauer */
	/* 28*/ {'a','m'}, /* Amazone    (Dame + Springer) */
	/* 29*/ {'k','n'}, /* Kaiserin   (Turm + Springer) */
	/* 30*/ {'p','r'}, /* Prinzessin (Laeufer + Springer) */
	/* 31*/ {'g','n'}, /* Gnu        (Springer + Kamel) */
	/* 32*/ {'a','n'}, /* Antilope    3,4 S */
	/* 33*/ {'e','i'}, /* Eichhoernchen      (Springer + Alfil + Dabbabba) */
	/* 34*/ {'w','a'}, /* Waran      (Turm + Nachtreiter) */
	/* 35*/ {'d','r'}, /* Drache     (Springer + Bauer) */
	/* 36*/ {'k','a'}, /* Kaenguruh */
	/* 37*/ {'s','s'}, /* Spiralspringer */
	/* 38*/ {'u','u'}, /* UbiUbi */
	/* 39*/ {'h','a'}, /* Hamster: 180 Grad Grashuepfer */
	/* 40*/ {'e','l'}, /* Elch: 45 Grad Grashuepfer */
	/* 41*/ {'a','d'}, /* Adler: 90 Grad Grashuepfer */
	/* 42*/ {'s','p'}, /* Spatz: 135 Grad Grashuepfer */
	/* 43*/ {'e','r'}, /* Erzbischof */
	/* 44*/ {'r','l'}, /* Reflektierender Laeufer */
	/* 45*/ {'k','l'}, /* Kardinal */
	/* 46*/ {'n','h'}, /* Nachtreiterhuepfer */
	/* 47*/ {'d','u'}, /* Dummy */
	/* 48*/ {'c','r'}, /* Kamelreiter */
	/* 49*/ {'z','r'}, /* Zebrareiter */
	/* 50*/ {'g','r'}, /* Gnureiter */
	/* 51*/ {'c','h'}, /* Kamelreiterhuepfer */
	/* 52*/ {'z','h'}, /* Zebrareiterhuepfer */
	/* 53*/ {'g','h'}, /* Gnureiterhuepfer */
	/* 54*/ {'d','s'}, /* DiagonalSpiralSpringer */
	/* 55*/ {'r','s'}, /* Reflektierender Springer */
	/* 56*/ {'e','q'}, /* Equihopper */
	/* 57*/ {'c','t'}, /* CAT (= cavalier de troie) */
	/* 58*/ {'b','s'}, /* Berolina Superbauer */
	/* 59*/ {'s','b'}, /* Superbauer */
	/* 60*/ {'t','l'}, /* Turm-Lion */
	/* 61*/ {'l','l'}, /* Laeufer-Lion */
	/* 62*/ {'s','i'}, /* Sirene */
	/* 63*/ {'t','r'}, /* Triton */
	/* 64*/ {'n','d'}, /* Nereide */
	/* 65*/ {'o',' '}, /* Orphan */
	/* 66*/ {'r','d'}, /* Randschwein */
	/* 67*/ {'m','o'}, /* Moa */
	/* 68*/ {'t','j'}, /* Turm/Laeufer-Jaeger */
	/* 69*/ {'l','j'}, /* Laeufer/Turm-Jaeger */
	/* 70*/ {'a','o'}, /* Maoreiter */
	/* 71*/ {'o','a'}, /* Moareiter */
	/* 72*/ {'t','h'}, /* Turmhuepfer */
	/* 73*/ {'l','h'}, /* Laeuferhuepfer */
	/* 74*/ {'e','k'}, /* Erlkoenig */
	/* 75*/ {'p','f'}, /* Pfadfinder */
	/* 76*/ {'p','n'}, /* Pfadfinderin */
	/* 77*/ {'s','k'}, /* Skylla */
	/* 78*/ {'c','y'}, /* Charybdis */
	/* 79*/ {'k','g'}, /* KontraGrashuepfer */
	/* 80*/ {'r','n'}, /* RosenLion */
	/* 81*/ {'r','h'}, /* RosenHuepfer */
	/* 82*/ {'o','k'}, /* Okapi */
	/* 83*/ {'3','7'}, /* 3:7-Springer */
	/* 84*/ {'g','2'}, /* Grashuepfer-2 */
	/* 85*/ {'g','3'}, /* Grashuepfer-3 */
	/* 86*/ {'1','6'}, /* 1:6-Springer */
	/* 87*/ {'2','4'}, /* 2:4-Springer */
	/* 88*/ {'3','5'}, /* 3:5-Springer */
	/* 89*/ {'d','g'}, /* DoppelGrashuepfer */
	/* 90*/ {'k','h'}, /* Koenighuepfer */
	/* 91*/ {'o','r'}, /* Orix */
	/* 92*/ {'1','5'}, /* 1:5-Springer */
	/* 93*/ {'2','5'}, /* 2:5-Springer */
	/* 94*/ {'g','l'}, /* Gral */
        /* 95*/ {'h','t'}, /* Turmheuschrecke */
        /* 96*/ {'h','l'}, /* Laeuferheuschrecke */
        /* 97*/ {'h','n'}, /* Nachtreiterheuschrecke */
        /* 98*/ {'w','r'}, /* Wesirreiter */
        /* 99*/ {'f','r'}, /* Fersreiter */
	/*100*/ {'b','i'}, /* Bison */
	/*101*/ {'e','t'}, /* Elefant */
	/*102*/ {'n','a'}, /* Nao */
	/*103*/ {'t','c'}, /* Turmelch: 45 Grad Grashuepfer */
	/*104*/ {'t','a'}, /* Turmadler: 90 Grad Grashuepfer */
	/*105*/ {'t','s'}, /* Turmspatz: 135 Grad Grashuepfer */
	/*106*/ {'l','c'}, /* Laeuferelch: 45 Grad Grashuepfer */
	/*107*/ {'l','a'}, /* Laeuferadler: 90 Grad Grashuepfer */
	/*108*/ {'l','s'}, /* Laeuferspatz: 135 Grad Grashuepfer */
	/*109*/ {'r','a'}, /* Rao: Chinesische Rose */
	/*110*/ {'s','o'}, /* Skorpion: Erlkoenig + Grashuepfer */
	/*111*/ {'m','g'}, /* Marguerite */
	/*112*/ {'3','6'}, /* 3:6 Springer */
        /*113*/ {'n','l'}, /* Nachtreiterlion */
        /*114*/ {'m','l'}, /* Maoreiterlion */
        /*115*/ {'m','m'}, /* Moareiterlion */
        /*116*/ {'a','g'}, /* AndernachGrashuepfer */
        /*117*/ {'f',' '}, /* Freund */
        /*118*/ {'d','e'}, /* Delphin */
        /*119*/ {'h','e'}, /* Hase: Lion-Huepfer ueber 2 Boecke */
        /*120*/ {'b','o'}, /* Bob: Lion-Huepfer ueber 4 Boecke */
        /*121*/ {'q','e'}, /* EquiEnglisch */
        /*122*/ {'q','f'}, /* EquiFranzoesisch */
        /*123*/ {'o','d'}, /* Odysseus */
        /*124*/ {'b','1'}, /* Bouncer */
        /*125*/ {'b','2'}, /* Turm-bouncer */
        /*126*/ {'b','3'}, /* Laeufer-bouncer */
        /*127*/ {'c','b'}  /* Chinesischer Bauer */
	},{/* English PieNamString */
	/*  0*/ {'.',' '}, /* empty */
	/*  1*/ {' ',' '}, /* outside board */
	/*  2*/ {'k',' '}, /* king */
	/*  3*/ {'p',' '}, /* pawn */
	/*  4*/ {'q',' '}, /* queen */
	/*  5*/ {'s',' '}, /* knight */
	/*  6*/ {'r',' '}, /* rook */
	/*  7*/ {'b',' '}, /* bishop */
	/*  8*/ {'l','e'}, /* leo */
	/*  9*/ {'m','a'}, /* mao */
	/* 10*/ {'p','a'}, /* pao */
	/* 11*/ {'v','a'}, /* vao */
	/* 12*/ {'r','o'}, /* rose */
	/* 13*/ {'g',' '}, /* grashopper */
	/* 14*/ {'n',' '}, /* nightrider */
	/* 15*/ {'z',' '}, /* zebra */
	/* 16*/ {'c','a'}, /* camel */
	/* 17*/ {'g','i'}, /* giraffe */
	/* 18*/ {'r','f'}, /* root-50-leaper */
	/* 19*/ {'b','u'}, /* root-25-leaper  (bucephale: horse of Alexander the Great) */
	/* 20*/ {'w','e'}, /* wazir */
	/* 21*/ {'a','l'}, /* alfil */
	/* 22*/ {'f','e'}, /* fers */
	/* 23*/ {'d','a'}, /* dabbabba */
	/* 24*/ {'l','i'}, /* lion */
	/* 25*/ {'n','e'}, /* nonstop equihopper */
	/* 26*/ {'l',' '}, /* locust */
	/* 27*/ {'b','p'}, /* berolina pawn */
	/* 28*/ {'a','m'}, /* amazon */
	/* 29*/ {'e','m'}, /* empress */
	/* 30*/ {'p','r'}, /* princess */
	/* 31*/ {'g','n'}, /* gnu */
	/* 32*/ {'a','n'}, /* antelope */
	/* 33*/ {'s','q'}, /* squirrel */
	/* 34*/ {'w','a'}, /* waran */
	/* 35*/ {'d','r'}, /* dragon */
	/* 36*/ {'k','a'}, /* kangaroo */
	/* 37*/ {'s','s'}, /* Spiralspringer */
	/* 38*/ {'u','u'}, /* UbiUbi */
	/* 39*/ {'h','a'}, /* hamster: 180 degree grashopper */
	/* 40*/ {'m',' '}, /* moose: 45 degree grashopper */
	/* 41*/ {'e','a'}, /* eagle: 90 degree grashopper */
	/* 42*/ {'s','w'}, /* sparrow: 135 degree grashopper */
	/* 43*/ {'a','r'}, /* archbishop */
	/* 44*/ {'r','b'}, /* reflecting bishop */
	/* 45*/ {'c',' '}, /* cardinal */
	/* 46*/ {'n','h'}, /* nightrider-hopper */
	/* 47*/ {'d','u'}, /* dummy */
	/* 48*/ {'c','r'}, /* camelrider */
	/* 49*/ {'z','r'}, /* zebrarider */
	/* 50*/ {'g','r'}, /* gnurider */
	/* 51*/ {'c','h'}, /* camelrider hopper */
	/* 52*/ {'z','h'}, /* zebrarider hopper */
	/* 53*/ {'g','h'}, /* gnurider hopper */
	/* 54*/ {'d','s'}, /* DiagonalSpiralSpringer */
	/* 55*/ {'b','k'}, /* bouncy knight */
	/* 56*/ {'e','q'}, /* equihoppper */
	/* 57*/ {'c','t'}, /* CAT  ( cavalier de troie) */
	/* 58*/ {'b','s'}, /* berolina superpawn */
	/* 59*/ {'s','p'}, /* superpawn */
	/* 60*/ {'r','l'}, /* rook-lion */
	/* 61*/ {'b','l'}, /* bishop-lion */
	/* 62*/ {'s','i'}, /* sirene */
	/* 63*/ {'t','r'}, /* triton */
	/* 64*/ {'n','d'}, /* nereide */
	/* 65*/ {'o',' '}, /* orphan */
	/* 66*/ {'e','h'}, /* edgehog */
	/* 67*/ {'m','o'}, /* moa */
	/* 68*/ {'r','r'}, /* rook/bishop-hunter */
	/* 69*/ {'b','r'}, /* bishop/rook-hunter */
	/* 70*/ {'a','o'}, /* maorider */
	/* 71*/ {'o','a'}, /* moarider */
	/* 72*/ {'r','h'}, /* rookhopper */
	/* 73*/ {'b','h'}, /* bishophopper */
	/* 74*/ {'e','k'}, /* erlking */
	/* 75*/ {'b','t'}, /* boyscout */
	/* 76*/ {'g','t'}, /* girlscout */
	/* 77*/ {'s','k'}, /* Skylla */
	/* 78*/ {'c','y'}, /* Charybdis */
	/* 79*/ {'c','g'}, /* contra grashopper */
	/* 80*/ {'r','n'}, /* RoseLion */
	/* 81*/ {'r','p'}, /* RoseHopper */
	/* 82*/ {'o','k'}, /* Okapi */
	/* 83*/ {'3','7'}, /* 3:7-leaper */
	/* 84*/ {'g','2'}, /* grasshopper-2 */
	/* 85*/ {'g','3'}, /* grasshopper-3 */
	/* 86*/ {'1','6'}, /* 1:6-leaper */
	/* 87*/ {'2','4'}, /* 2:4-leaper */
	/* 88*/ {'3','5'}, /* 3:5-leaper */
	/* 89*/ {'d','g'}, /* Double Grasshopper */
	/* 90*/ {'k','h'}, /* Kinghopper */
	/* 91*/ {'o','r'}, /* Orix */
	/* 92*/ {'1','5'}, /* 1:5-leaper */
	/* 93*/ {'2','5'}, /* 2:5-leaper */
	/* 94*/ {'g','l'}, /* Gral */
        /* 95*/ {'l','r'}, /* rook locust */
        /* 96*/ {'l','b'}, /* bishop locust */
        /* 97*/ {'l','n'}, /* nightrider locust */
        /* 98*/ {'w','r'}, /* wazirrider */
        /* 99*/ {'f','r'}, /* fersrider */
	/*100*/ {'b','i'}, /* bison */
	/*101*/ {'e','t'}, /* elephant */
	/*102*/ {'n','a'}, /* Nao */
	/*103*/ {'r','m'}, /* rook moose: 45 degree grashopper */
	/*104*/ {'r','e'}, /* rook eagle: 90 degree grashopper */
	/*105*/ {'r','w'}, /* rook sparrow: 135 degree grashopper */
	/*106*/ {'b','m'}, /* bishop moose: 45 degree grashopper */
	/*107*/ {'b','e'}, /* bishop eagle: 90 degree grashopper */
	/*108*/ {'b','w'}, /* bishop sparrow: 135 degree grashopper */
	/*109*/ {'r','a'}, /* rao: chinese rose */
	/*110*/ {'s','o'}, /* scorpion: erlking + grashopper */
	/*111*/ {'m','g'}, /* marguerite */
	/*112*/ {'3','6'}, /* 3:6 leaper */
        /*113*/ {'n','l'}, /* nightriderlion */
        /*114*/ {'m','l'}, /* maoriderlion */
        /*115*/ {'m','m'}, /* moariderlion */
        /*116*/ {'a','g'}, /* AndernachGrasshopper */
        /*117*/ {'f',' '}, /* friend */
        /*118*/ {'d','o'}, /* dolphin */
        /*119*/ {'r','t'}, /* rabbit */
        /*120*/ {'b','o'}, /* bob */
        /*121*/ {'q','e'}, /* equi english */
        /*122*/ {'q','f'}, /* equi french */
        /*123*/ {'q','q'}, /* querquisite */
        /*124*/ {'b','1'}, /* bouncer */
        /*125*/ {'b','2'}, /* tour-bouncer */
        /*126*/ {'b','3'},  /* fou-bouncer */
        /*127*/ {'c','p'}  /* chinese pawn */	
  }
	};
#endif

#ifdef WE_ARE_EXTERN
	extern Lang     ActLang;
	extern char     ActAuthor[];
	extern char     ActOrigin[];
	extern char     ActTitle[];
	extern char     ActTwin[];
	extern char     ActAward[], ActStip[];          /* V3.46  TLi */
#else
	Lang            ActLang;
	char            ActAuthor[256];
	char            ActOrigin[256];
	char            ActTitle[256];
	char            ActTwin[1532];
	char            ActAward[256], ActStip[37];     /* V3.46  TLi */
#endif

#ifdef WE_ARE_EXTERN
	extern PieceChar *PieceTab;
#else
	PieceChar       *PieceTab=PieNamString[German];
#endif
				/* later set according to language */

#ifdef WE_ARE_EXTERN
	extern boolean  OptFlag[OptCount];
	extern char     *OptString[LangCount][OptCount];
#else
	boolean OptFlag[OptCount];
	char    *OptString[LangCount][OptCount] = {
	{
	/* Francais French Franzoesisch */
	/* 0*/  "Defense",
	/* 1*/  "Apparent",
	/* 2*/  "ApparentSeul",
	/* 3*/  "Variantes",
	/* 4*/  "Trace",
	/* 5*/  "Enroute",
	/* 6*/  "SansRB",
	/* 7*/  "Duplex",
	/* 8*/  "SansRN",
	/* 9*/  "Menace",
	/*10*/  "SansMenace",
	/*11*/  "Essais",
	/*12*/  "MaxSolutions",
	/*13*/  "MaxCasesFuites",
	/*14*/  "Search",
	/*15*/  "MultiSolutions",
	/*16*/  "NonSymetrique",
	/*17*/  "NonTrivial",
	/*18*/  "KeepMatingPiece",
	/*19*/  "EnPassant",
	/*20*/  "SansEchiquier",
	/*21*/  "SansVariantesCourtes",
	/*22*/  "DemiDuplex",
	/*23*/  "ApresCle",
	/*24*/  "Intelligent",
	/*25*/  "MaxTemps",
	/*26*/  "SansRoquer",
	/*27*/  "Quodlibet",
	/*28*/  "FinApresSolutionsCourtes",
	/*29*/  "Bip"
	},{
	/* Deutsch German Allemand */
	/* 0*/  "Widerlegung",
	/* 1*/  "Satzspiel",
	/* 2*/  "WeissBeginnt",
	/* 3*/  "Varianten",
	/* 4*/  "Zugnummern",
	/* 5*/  "StartZugnummer",
	/* 6*/  "OhneWk",
	/* 7*/  "Duplex",
	/* 8*/  "OhneSk",
	/* 9*/  "Drohung",
	/*10*/  "OhneDrohung",
	/*11*/  "Verfuehrung",
	/*12*/  "MaxLoesungen",
	/*13*/  "MaxFluchtFelder",
	/*14*/  "Suchen",
	/*15*/  "MehrereLoesungen",
	/*16*/  "KeineSymmetrie",
	/*17*/  "NichtTrivial",
	/*18*/  "HalteMattsetzendenStein",
	/*19*/  "EnPassant",
	/*20*/  "OhneBrett",
	/*21*/  "OhneKurzVarianten",
	/*22*/  "HalbDuplex",
	/*23*/  "NachSchluessel",
	/*24*/  "Intelligent",
	/*25*/  "MaxZeit",
	/*26*/  "KeineRochade",
	/*27*/  "Quodlibet",
	/*28*/  "StopNachKurzloesungen",
	/*29*/  "Pieps"
	},{
	/* English Anglais Englisch */
	/* 0*/  "Defence",
	/* 1*/  "SetPlay",
	/* 2*/  "WhiteToPlay",
	/* 3*/  "Variations",
	/* 4*/  "MoveNumbers",
	/* 5*/  "StartMoveNumber",
	/* 6*/  "NoWk",
	/* 7*/  "Duplex",
	/* 8*/  "NoBk",
	/* 9*/  "Threat",
	/*10*/  "NoThreat",
	/*11*/  "Try",
	/*12*/  "MaxSolutions",
	/*13*/  "MaxFlightsquares",
	/*14*/  "Search",
	/*15*/  "MultiSolutions",
	/*16*/  "NoSymmetry",
	/*17*/  "NonTrivial",
	/*18*/  "KeepMatingPiece",
	/*19*/  "EnPassant",
	/*20*/  "NoBoard",
	/*21*/  "NoShortVariations",
	/*22*/  "HalfDuplex",
	/*23*/  "PostKeyPlay",
	/*24*/  "Intelligent",
	/*25*/  "MaxTime",
	/*26*/  "NoCastling",
	/*27*/  "Quodlibet",
	/*28*/  "StopOnShortSolutions",
	/*29*/  "Beep"
	}
};
#endif

#ifdef WE_ARE_EXTERN
	extern boolean  CondFlag[CondCount];
	extern char     *CondString[LangCount][CondCount];
#else
	boolean CondFlag[CondCount];
	char    *CondString[LangCount][CondCount] = {
	{
	/* French Condition Names */
	/* 0*/  "RexInclusif",
	/* 1*/  "Circe",
	/* 2*/  "CirceMalefique",
	/* 3*/  "Madrasi",
	/* 4*/  "Volage",
	/* 5*/  "Hypervolage",
	/* 6*/  "Bichromatique",
	/* 7*/  "Monochromatique",
	/* 8*/  "Grille",
	/* 9*/  "Koeko",
	/*10*/  "NoirBordCoup",
	/*11*/  "BlancBordCoup",
	/*12*/  "Leofamily",
	/*13*/  "Chinoises",
	/*14*/  "Patrouille",
	/*15*/  "CirceEchange",
	/*16*/  "SansPrises",
	/*17*/  "Immun",
	/*18*/  "ImmunMalefique",
	/*19*/  "ContactGrille",
	/*20*/  "Imitator",             /* V2.4d  TM */
	/*21*/  "CavalierMajeur",       /* V2.60  NG */
	/*22*/  "Haan",                 /* V2.60  NG */
	/*23*/  "CirceCameleon",        /* V2.80c  TLi */
	/*24*/  "CirceCouscous",        /* V3.00  NG */
	/*25*/  "CirceEquipollents",    /* V2.80c  TLi */
	/*26*/  "FileCirce",            /* V2.80c  TLi */
	/*27*/  "NoirMaximum",          /* V2.90c  TLi */
	/*28*/  "NoirMinimum",          /* V2.90c  TLi */
	/*29*/  "BlancMaximum",         /* V2.90c  TLi */
	/*30*/  "BlancMinimum",         /* V2.90c  TLi */
	/*31*/  "CaseMagique",          /* V2.90c  TLi */
	/*32*/  "Sentinelles",          /* V2.90c  TLi */
	/*33*/  "Tibet",                /* V2.90c  TLi */
	/*34*/  "DoubleTibet",          /* V2.90c  TLi */
	/*35*/  "CirceDiagramme",       /* V2.90c  TLi */
	/*36*/  "Trou",                 /* V2.90  NG */
	/*37*/  "NoirPriseForce",       /* V3.0  TLi */
	/*38*/  "BlancPriseForce",      /* V3.0  TLi */
	/*39*/  "RoisReflecteurs",      /* V3.02  TLi */
	/*40*/  "RoisTransmutes",       /* V3.0  TLi */
	/*41*/  "NoirPoursuite",        /* V3.0  TLi */
	/*42*/  "BlancPoursuite",       /* V3.0  TLi */
	/*43*/  "Duelliste",            /* V3.0  TLi */
	/*44*/  "CirceParrain",         /* V3.02  TLi */
	/*45*/  "SansIProm",            /* V3.02  TLi */
	/*46*/  "CirceSymetrique",      /* V3.02  TLi */
	/*47*/  "EchecsVogtlaender",    /* V3.03  TLi, V3.54  NG */
	/*48*/  "EchecsEinstein",       /* V3.1  TLi */
	/*49*/  "Bicolores",            /* V3.1  TLi */
	/*50*/  "KoekoNouveaux",        /* V3.1  TLi */
	/*51*/  "CirceClone",           /* V3.1  TLi */
	/*52*/  "AntiCirce",
	/*53*/  "FileCirceMalefique",
	/*54*/  "CirceAntipoden",
	/*55*/  "CirceCloneMalefique",
	/*56*/  "AntiCirceMalefique",
	/*57*/  "AntiCirceDiagramme",
	/*58*/  "AntiFileCirce",
	/*59*/  "AntiCirceSymetrique",
	/*60*/  "AntiFileCirceMalefique",
	/*61*/  "AntiCirceAntipoden",
	/*62*/  "AntiCirceEquipollents",
	/*63*/  "ImmunFile",
	/*64*/  "ImmunDiagramme",
	/*65*/  "ImmunFileMalefique",
	/*66*/  "ImmunSymmetrique",
	/*67*/  "ImmunAntipoden",
	/*68*/  "ImmunEquipollents",            /* V3.1  TLi */
	/*69*/  "ReversEchecsEinstein",
	/*70*/  "SuperCirce",
	/*71*/  "Degradierung",
	/*72*/  "NorskSjakk",
	/*73*/  "exact",
	/*74*/  "EchecsTraitor",
	/*75*/  "EchecsAndernach",
	/*76*/  "BlancCaseForce",
	/*77*/  "BlancCaseForceConsequent",
	/*78*/  "NoirCaseForce",
	/*79*/  "NoirCaseForceConsequent",
	/*80*/  "ultra",                        /* V3.22  TLi */
	/*81*/  "EchecsChameleon",              /* V3.32  TLi */
	/*82*/  "EchecsFonctionnaire",          /* V3.32  TLi, V3.57  NG */
	/*83*/  "EchecsGlasgow",                /* V3.38  TLi */
	/*84*/  "EchecsAntiAndernach",          /* V3.41a TLi */
	/*85*/  "FrischAufCirce",               /* V3.41b TLi */
	/*86*/  "CirceMalefiqueVerticale",      /* V3.42  NG */
	/*87*/  "Isardam",                      /* V3.44  SE/TLi */
	/*88*/  "SansEchecs",                   /* V3.44  TLi */
	/*89*/  "CirceDiametrale",              /* V3.44  TLi */
	/*90*/  "PromSeul",                     /* V3.44  NG */
	/*91*/  "RankCirce",                    /* V3.45  TLi */
	/*92*/  "EchecsExclusif",               /* V3.45  TLi */
	/*93*/  "MarsCirce",                    /* V3.46  SE/TLi */
	/*94*/  "MarsCirceMalefique",           /* V3.46  TLi */
	/*95*/  "EchecsPhantom",                /* V3.47  NG */
	/*96*/  "BlancRoiReflecteur",           /* V3.47  NG */
	/*97*/  "NoirRoiReflecteur",            /* V3.47  NG */
	/*98*/  "BlancRoiTransmute",            /* V3.47  NG */
	/*99*/  "NoirRoiTransmute",             /* V3.47  NG */
	/*100*/ "EchecsAntiEinstein",           /* V3.50  TLi */
	/*101*/ "CirceCouscousMalefique",       /* V3.50  TLi */
	/*102*/ "NoirCaseRoyal",                /* V3.50  TLi */
	/*103*/ "BlancCaseRoyal",               /* V3.50  TLi */
	/*104*/ "EchecsBrunner",                /* V3.50  SE */
	/*105*/ "EchecsPlus",                   /* V3.50  SE */
	/*106*/ "CirceAssassin",                /* V3.50  SE */
	/*107*/ "EchecsPatience",		/* V3.50  SE */
	/*108*/ "EchecsRepublicains",		/* V3.50  SE */
	/*109*/ "EchecsExtinction",		/* V3.50  SE */
	/*110*/ "EchecsCentral",                /* V3.50  SE */
	/*111*/ "ActuatedRevolvingBoard",       /* V3.53  TLi */
	/*112*/ "EchecsMessigny",               /* V3.55  TLi */
	/*113*/ "Woozles",                      /* V3.55  TLi */
	/*114*/ "BiWoozles",                    /* V3.55  TLi */
	/*115*/ "Heffalumps",                   /* V3.55  TLi */
	/*116*/ "BiHeffalumps",                 /* V3.55  TLi */
	/*117*/ "RexExclusif",                  /* V3.55  TLi */
	/*118*/ "BlancCasePromotion",           /* V3.57  NG */
	/*119*/ "NoirCasePromotion",            /* V3.57  NG */
	/*120*/ "SansBlancPromotion",           /* V3.57  NG */
	/*121*/ "SansNoirPromotion",            /* V3.57  NG */
	/*122*/ "EchecsEiffel",                 /* V3.60  TLi */
	/*123*/ "NoirUltraSchachZwang",		/* V3.62  SE */
	/*124*/ "BlancUltraSchachZwang",	/* V3.62  SE */
	/*125*/ "ActuatedRevolvingCentre",      /* V3.62  SE */
	/*126*/ "ShieldedKings",       		/* V3.62  SE */
	/*127*/ "EchecsSting",      		/* V3.63  NG */
	/*128*/ "EchecsChameleonLigne",         /* V3.64  TLi */
	/*129*/ "BlancSansPrises",              /* V3.64  NG */
	/*130*/ "NoirSansPrises",               /* V3.64  NG */
	/*131*/ "EchecsAvril",                  /* V3.64  ThM */
	/*132*/ "EchecsAlphabetiques",          /* V3.64  SE */
	/*133*/ "CirceTurncoats",               /* V3.70  SE */
	/*134*/ "CirceDoubleAgents",            /* V3.70  SE */
	/*135*/ "AMU",                          /* V3.70  SE */
 	/*136*/ "SingleBox",                    /* V3.71  ThM, NG */
	/*137*/ "MAFF",				/* V3.78  SE */
	/*138*/ "OWU",				/* V3.78  SE */
	/*139*/ "BlancRoisOscillant",		/* V3.78  SE */
	/*140*/ "NoirRoisOscillant",	        /* V3.78  SE */
	/*141*/ "AntiRois",	                /* V3.78  SE */
	/*142*/ "AntiMarsCirce",	        /* V3.78  SE */
	/*143*/ "AntiMarsMalefiqueCirce",	/* V3.78  SE */
	/*144*/ "AntiMarsAntipodeanCirce",	/* V3.78  SE */
	/*145*/ "BlancSuperRoiTransmute",       /* V3.78  SE */
	/*146*/ "NoirSuperRoiTransmute",        /* V3.78  SE */
	/*147*/ "AntiSuperCirce",               /* V3.78  SE */
	/*148*/ "UltraPatrouille",              /* V3.78  SE */
	/*149*/ "RoisEchanges",                 /* V3.81a NG */
	/*150*/ "DynastieRoyale",                /* V4.02 TM */
	/*151*/ "SAT",                       /* V4.03  SE */
	/*152*/ "StrictSAT",                 /* V4.03  SE */
	/*153*/ "Take&MakeEchecs",                 /* V4.03  SE */
	/*154*/ "NoirSynchronCoup",                 /* V4.06  SE */
	/*155*/ "BlancSynchronCoup",                 /* V4.06  SE */
	/*156*/ "NoirAntiSynchronCoup",                 /* V4.06  SE */
	/*157*/ "BlancAntiSynchronCoup",                 /* V4.06  SE */
	/*158*/ "Masand",                 /* V4.06  SE */
	/*159*/ "BGL" ,                     /* V4.06 SE */
	/*160*/ "NoirEchecs",                     /* V4.06 SE */
  	/*161*/ "AnnanEchecs" ,          /* V4.07 SE */
  	/*162*/ "PionNormale"           /* V4.07 SE */
	},{
	/* German Condition Names */
	/* 0*/  "RexInklusive",
	/* 1*/  "Circe",
	/* 2*/  "SpiegelCirce",
	/* 3*/  "Madrasi",
	/* 4*/  "Volage",
	/* 5*/  "Hypervolage",
	/* 6*/  "BichromesSchach",
	/* 7*/  "MonochromesSchach",
	/* 8*/  "Gitterschach",
	/* 9*/  "KoeKo",
	/*10*/  "SchwarzerRandzueger",
	/*11*/  "WeisserRandzueger",
	/*12*/  "Leofamily",
	/*13*/  "ChinesischesSchach",
	/*14*/  "PatrouilleSchach",
	/*15*/  "PlatzwechselCirce",
	/*16*/  "Ohneschlag",
	/*17*/  "Immunschach",
	/*18*/  "SpiegelImmunschach",
	/*19*/  "KontaktGitter",
	/*20*/  "Imitator",                     /* V2.4d  TM */
	/*21*/  "CavalierMajeur",               /* V2.60  NG */
	/*22*/  "HaanerSchach",                 /* V2.60  NG */
	/*23*/  "ChamaeleonCirce",              /* V2.80c  TLi */
	/*24*/  "CouscousCirce",                /* V3.00  NG */
	/*25*/  "EquipollentsCirce",            /* V2.80c  TLi */
	/*26*/  "FileCirce",                    /* V2.80c  TLi */
	/*27*/  "SchwarzerLaengstzueger",       /* V2.90c  TLi */
	/*28*/  "SchwarzerKuerzestzueger",      /* V2.90c  TLi */
	/*29*/  "WeisserLaengstzueger",         /* V2.90c  TLi */
	/*30*/  "WeisserKuerzestzueger",        /* V2.90c  TLi */
	/*31*/  "MagischeFelder",               /* V2.90c  TLi */
	/*32*/  "Sentinelles",                  /* V2.90c  TLi */
	/*33*/  "TibetSchach",                  /* V2.90c  TLi */
	/*34*/  "DoppeltibetSchach",            /* V2.90c  TLi */
	/*35*/  "DiagrammCirce",                /* V2.90c  TLi */
	/*36*/  "Loch",                         /* V2.90  NG */
	/*37*/  "SchwarzerSchlagzwang",         /* V3.0  TLi */
	/*38*/  "WeisserSchlagzwang",           /* V3.0  TLi */
	/*39*/  "ReflektierendeKoenige",        /* V3.0  TLi */
	/*40*/  "TransmutierendeKoenige",       /* V3.02 TLi */
	/*41*/  "SchwarzerVerfolgungszueger",   /* V3.0  TLi */
	/*42*/  "WeisserVerfolgungszueger",     /* V3.0  TLi */
	/*43*/  "Duellantenschach",             /* V3.0  TLi */
	/*44*/  "CirceParrain",                 /* V3.02  TLi */
	/*45*/  "OhneIUW",                      /* V3.02  TLi */
	/*46*/  "SymmetrieCirce",               /* V3.02  TLi */
	/*47*/  "VogtlaenderSchach",            /* V3.03  TLi */
	/*48*/  "EinsteinSchach",               /* V3.1  TLi */
	/*49*/  "Bicolores",                    /* V3.1  TLi */
	/*50*/  "NeuKoeko",             /* V3.1  TLi */
	/*51*/  "CirceClone",           /* V3.1  TLi */
	/*52*/  "AntiCirce",            /* V3.1  TLi */
	/*53*/  "SpiegelFileCirce",
	/*54*/  "AntipodenCirce",
	/*55*/  "SpiegelCirceclone",
	/*56*/  "AntiSpiegelCirce",
	/*57*/  "AntiDiagrammCirce",
	/*58*/  "AntiFileCirce",
	/*59*/  "AntiSymmetrieCirce",
	/*60*/  "AntiSpiegelFileCirce",
	/*61*/  "AntiAntipodenCirce",
	/*62*/  "AntiEquipollentsCirce",
	/*63*/  "FileImmunSchach",
	/*64*/  "DiagrammImmunSchach",
	/*65*/  "SpiegelFileImmunSchach",
	/*66*/  "SymmetrieImmunSchach",
	/*67*/  "AntipodenImmunSchach",
	/*68*/  "EquipollentsImmunSchach",
	/*69*/  "ReversesEinsteinSchach",
	/*70*/  "SuperCirce",
	/*71*/  "Degradierung",
	/*72*/  "NorskSjakk",
	/*73*/  "exakt",
	/*74*/  "TraitorChess",
	/*75*/  "AndernachSchach",
	/*76*/  "WeissesZwangsfeld",
	/*77*/  "WeissesKonsequentesZwangsfeld",
	/*78*/  "SchwarzesZwangsfeld",
	/*79*/  "SchwarzesKonsequentesZwangsfeld",
	/*80*/  "Ultra",
	/*81*/  "ChamaeleonSchach",        /* V3.32  TLi */
	/*82*/  "BeamtenSchach",           /* V3.32  TLi */
	/*83*/  "GlasgowSchach",           /* V3.38  TLi */
	/*84*/  "AntiAndernachSchach",          /* V3.41a TLi */
	/*85*/  "FrischAufCirce",          /* V3.41b TLi */
	/*86*/  "VertikalesSpiegelCirce",       /* V3.42 NG */
	/*87*/  "Isardam",                      /* V3.44  SE/TLi */
	/*88*/  "OhneSchach",                   /* V3.44  TLi */
	/*89*/  "DiametralCirce",               /* V3.44  TLi */
	/*90*/  "UWnur",                        /* V3.44  NG */
	/*91*/  "RankCirce",                    /* V3.45  TLi */
	/*92*/  "ExklusivSchach",               /* V3.45  TLi */
	/*93*/  "MarsCirce",                    /* V3.46  SE/TLi */
	/*94*/  "MarsSpiegelCirce",             /* V3.46  TLi */
	/*95*/  "PhantomSchach",                /* V3.47  NG */
	/*96*/  "WeisserReflektierenderKoenig",    /* V3.47  NG */
	/*97*/  "SchwarzerReflektierenderKoenig",  /* V3.47  NG */
	/*98*/  "WeisserTransmutierenderKoenig",   /* V3.47  NG */
	/*99*/  "SchwarzerTransmutierenderKoenig", /* V3.47  NG */
	/*100*/ "AntiEinsteinSchach",           /* V3.50  TLi */
	/*101*/ "SpiegelCouscousCirce",         /* V3.50  TLi */
	/*102*/ "SchwarzesKoeniglichesFeld",    /* V3.50  TLi */
	/*103*/ "WeissesKoeniglichesFeld",      /* V3.50  TLi */
	/*104*/ "BrunnerSchach",                /* V3.50  SE */
	/*105*/ "PlusSchach",                   /* V3.50  SE */
	/*106*/ "AssassinCirce",                /* V3.50  SE */
	/*107*/ "PatienceSchach",		/* V3.50  SE */
	/*108*/ "RepublikanerSchach",           /* V3.50  SE */
	/*109*/ "AusrottungsSchach",            /* V3.50  SE */
	/*110*/ "ZentralSchach",                /* V3.50  SE */
	/*111*/ "ActuatedRevolvingBoard",       /* V3.53  TLi */
	/*112*/ "MessignySchach",               /* V3.55  TLi */
	/*113*/ "Woozles",                      /* V3.55  TLi */
	/*114*/ "BiWoozles",                    /* V3.55  TLi */
	/*115*/ "Heffalumps",                   /* V3.55  TLi */
	/*116*/ "BiHeffalumps",                 /* V3.55  TLi */
	/*117*/ "RexExklusive",                 /* V3.55  TLi */
	/*118*/ "UWFeldWeiss",                  /* V3.57  NG */
	/*119*/ "UWFeldSchwarz",                /* V3.57  NG */
	/*120*/ "OhneWeissUW",                  /* V3.57  NG */
	/*121*/ "OhneSchwarzUW",                /* V3.57  NG */
	/*122*/ "EiffelSchach",                 /* V3.60  TLi */
	/*123*/ "SchwarzerUltraSchachZwang",	/* V3.62  SE */
	/*124*/ "WeisserUltraSchachZwang",	/* V3.62  SE */
	/*125*/ "ActuatedRevolvingCentre",      /* V3.62  SE */
	/*126*/ "SchutzKoenige",       		/* V3.62  SE */
	/*127*/ "StingSchach",                  /* V3.63  NG */
	/*128*/ "LinienChamaeleonSchach",      	/* V3.64  TLi */
	/*129*/ "WeisserOhneSchlag",      	/* V3.64  NG */
	/*130*/ "SchwarzerOhneSchlag",      	/* V3.64  NG */
	/*131*/ "Aprilschach",                  /* V3.64  ThM */
	/*132*/ "AlphabetischesSchach",         /* V3.64  SE */
	/*133*/ "TurncoatCirce",                /* V3.70  SE */
	/*134*/ "DoppelAgentenCirce",           /* V3.70  SE */
	/*135*/ "AMU",                          /* V3.70  SE */
        /*136*/ "NurPartiesatzSteine",          /* V3.71  ThM, NG */
        /*137*/ "MAFF",                     	/* V3.78  SE */
        /*138*/ "OWU",                      	/* V3.78  SE */
        /*139*/ "WeisseOszillierendeKoenige",   /* V3.78  SE */
        /*140*/ "SchwarzeOszillierendeKoenige", /* V3.78  SE */
        /*141*/ "AntiKoenige",                  /* V3.78  SE */
	/*142*/ "AntiMarsCirce",	        /* V3.78  SE */
	/*143*/ "AntiMarsMalefiqueCirce",	/* V3.78  SE */
	/*144*/ "AntiMarsAntipodeanCirce",	/* V3.78  SE */
	/*145*/ "WeisserSuperTransmutierenderKoenig",	/* V3.78  SE */
	/*146*/ "SchwarzerSuperTransmutierenderKoenig",	/* V3.78  SE */
	/*147*/ "AntiSuperCirce",               /* V3.78  SE */
	/*148*/ "UltraPatrouille",              /* V3.78  SE */
	/*149*/ "TauschKoenige",		/* V3.81a NG */
	/*150*/ "KoenigsDynastie",               /* V4.02 TM */
	/*151*/ "SAT",                       /* V4.03  SE */
	/*152*/ "StrictSAT",                 /* V4.03  SE */
	/*153*/ "Take&MakeSchach",                 /* V4.03  SE */
	/*154*/ "SchwarzerSynchronZueger",                 /* V4.06  SE */
	/*155*/ "WeisserSynchronZueger",                 /* V4.06  SE */
	/*156*/ "SchwarzerAntiSynchronZueger",                 /* V4.06  SE */
	/*157*/ "WeisserAntiSynchronZueger",                 /* V4.06  SE */
	/*158*/ "Masand",                 /* V4.06  SE */
	/*159*/ "BGL",                     /* V4.06 SE */
	/*160*/ "SchwarzSchaecher",              /* V4.06 SE */
	/*161*/ "Annanschach",            /* V4.07 SE */
	/*162*/ "NormalBauern"            /* V4.07 SE */
  },{
	/* English Condition Names */
	/* 0*/  "RexInclusiv",
	/* 1*/  "Circe",
	/* 2*/  "MirrorCirce",
	/* 3*/  "Madrasi",
	/* 4*/  "Volage",
	/* 5*/  "Hypervolage",
	/* 6*/  "BichromChess",
	/* 7*/  "MonochromChess",
	/* 8*/  "GridChess",
	/* 9*/  "KoeKo",
	/*10*/  "BlackEdgeMover",
	/*11*/  "WhiteEdgeMover",
	/*12*/  "Leofamily",
	/*13*/  "ChineseChess",
	/*14*/  "Patrouille",
	/*15*/  "PWC",
	/*16*/  "NoCapture",
	/*17*/  "ImmunChess",
	/*18*/  "MirrorImmunChess",
	/*19*/  "ContactGridChess",
	/*20*/  "Imitator",             /* V2.4d  TM */
	/*21*/  "CavalierMajeur",       /* V2.60  NG */
	/*22*/  "HaanerChess",          /* V2.60  NG */
	/*23*/  "ChameleonCirce",       /* V2.80c  TLi */
	/*24*/  "CouscousCirce",        /* V3.00  NG */
	/*25*/  "EquipollentsCirce",    /* V2.80c  TLi */
	/*26*/  "FileCirce",            /* V2.80c  TLi */
	/*27*/  "BlackMaximummer",      /* V2.90c  TLi */
	/*28*/  "BlackMinimummer",      /* V2.90c  TLi */
	/*29*/  "WhiteMaximummer",      /* V2.90c  TLi */
	/*30*/  "WhiteMinimummer",      /* V2.90c  TLi */
	/*31*/  "MagicSquares",         /* V2.90c  TLi */
	/*32*/  "Sentinelles",          /* V2.90c  TLi */
	/*33*/  "Tibet",                /* V2.90c  TLi */
	/*34*/  "DoubleTibet",          /* V2.90c  TLi */
	/*35*/  "DiagramCirce",         /* V2.90c  TLi */
	/*36*/  "Hole",                 /* V2.90  NG */
	/*37*/  "BlackMustCapture",     /* V3.0  TLi */
	/*38*/  "WhiteMustCapture",     /* V3.0  TLi */
	/*39*/  "ReflectiveKings",      /* V3.0  TLi */
	/*40*/  "TransmutedKings",      /* V3.02 TLi */
	/*41*/  "BlackFollowMyLeader",  /* V3.0  TLi */
	/*42*/  "WhiteFollowMyLeader",  /* V3.0  TLi */
	/*43*/  "DuellistChess",        /* V3.0  TLi */
	/*44*/  "ParrainCirce",         /* V3.02  TLi */
	/*45*/  "NoIProm",              /* V3.02  TLi */
	/*46*/  "SymmetryCirce",        /* V3.02  TLi */
	/*47*/  "VogtlaenderChess",     /* V3.03  TLi */
	/*48*/  "EinsteinChess",        /* V3.1  TLi */
	/*49*/  "Bicolores",            /* V3.1  TLi */
	/*50*/  "NewKoeko",             /* V3.1  TLi */
	/*51*/  "CirceClone",           /* V3.1  TLi */
	/*52*/  "AntiCirce",            /* V3.1  TLi */
	/*53*/  "MirrorFileCirce",
	/*54*/  "AntipodeanCirce",
	/*55*/  "MirrorCirceClone",
	/*56*/  "MirrorAntiCirce",
	/*57*/  "DiagramAntiCirce",
	/*58*/  "FileAntiCirce",
	/*59*/  "SymmetryAntiCirce",
	/*60*/  "MirrorAntiCirce",
	/*61*/  "AntipodeanAntiCirce",
	/*62*/  "EquipollentsAntiCirce",
	/*63*/  "FileImmunChess",
	/*64*/  "DiagramImmunChess",
	/*65*/  "MirrorFileImmunChess",
	/*66*/  "SymmetryImmunChess",
	/*67*/  "AntipodeanImmunChess",
	/*68*/  "EquipollentsImmunChess",
	/*69*/  "ReversalEinsteinChess",
	/*70*/  "SuperCirce",
	/*71*/  "RelegationChess",
	/*72*/  "NorskSjakk",
	/*73*/  "exact",
	/*74*/  "TraitorChess",
	/*75*/  "AndernachChess",
	/*76*/  "WhiteForcedSquare",
	/*77*/  "WhiteConsequentForcedSquare",
	/*78*/  "BlackForcedSquare",
	/*79*/  "BlackConsequentForcedSquare",
	/*80*/  "ultra",
	/*81*/  "ChameleonChess",    /* V3.32  TLi */
	/*82*/  "FunctionaryChess",  /* V3.32  TLi, V3.57  NG */
	/*83*/  "GlasgowChess",      /* V3.38  TLi */
	/*84*/  "AntiAndernachChess",  /* V3.41a TLi */
	/*85*/  "FrischAufCirce",      /* V3.41b TLi */
	/*86*/  "VerticalMirrorCirce",  /* V3.42 NG */
	/*87*/  "Isardam",             /* V3.44  SE/TLi */
	/*88*/  "ChecklessChess",      /* V3.44  TLi */
	/*89*/  "DiametralCirce",       /* V3.44  TLi */
	/*90*/  "PromOnly",             /* V3.44  NG */
	/*91*/  "RankCirce",            /* V3.45  TLi */
	/*92*/  "ExclusivChess",        /* V3.45  TLi */
	/*93*/  "MarsCirce",            /* V3.46  SE/TLi */
	/*94*/  "MarsMirrorCirce",      /* V3.46  TLi */
	/*95*/  "PhantomChess",         /* V3.47  NG */
	/*96*/  "WhiteReflectiveKing",  /* V3.47  NG */
	/*97*/  "BlackReflectiveKing",  /* V3.47  NG */
	/*98*/  "WhiteTransmutedKing",  /* V3.47  NG */
	/*99*/  "BlackTransmutedKing",  /* V3.47  NG */
	/*100*/ "AntiEinsteinChess",    /* V3.50  TLi */
	/*101*/ "MirrorCouscousCirce",  /* V3.50  TLi */
	/*102*/ "BlackRoyalSquare",     /* V3.50  TLi */
	/*103*/ "WhiteRoyalSquare",     /* V3.50  TLi */
	/*104*/ "BrunnerChess",         /* V3.50  SE */
	/*105*/ "PlusChess",            /* V3.50  SE */
	/*106*/ "CirceAssassin",        /* V3.50  SE */
	/*107*/ "PatienceChess",	/* V3.50  SE */
	/*108*/ "RepublicanChess",      /* V3.50  SE */
	/*109*/ "ExtinctionChess",      /* V3.50  SE */
	/*110*/ "CentralChess",         /* V3.50  SE */
	/*111*/ "ActuatedRevolvingBoard",/* V3.53  TLi */
	/*112*/ "MessignyChess",        /* V3.55  TLi */
	/*113*/ "Woozles",              /* V3.55  TLi */
	/*114*/ "BiWoozles",            /* V3.55  TLi */
	/*115*/ "Heffalumps",           /* V3.55  TLi */
	/*116*/ "BiHeffalumps",         /* V3.55  TLi */
	/*117*/ "RexExclusiv",          /* V3.55  TLi */
	/*118*/ "WhitePromSquares",     /* V3.57  NG */
	/*119*/ "BlackPromSquares",     /* V3.57  NG */
	/*120*/ "NoWhitePromotion",     /* V3.57  NG */
	/*121*/ "NoBlackPromotion",     /* V3.57  NG */
	/*122*/ "EiffelChess",           /* V3.60  TLi */
	/*123*/ "BlackUltraSchachZwang",	/* V3.62  SE */
	/*124*/ "WhiteUltraSchachZwang",	/* V3.62  SE */
	/*125*/ "ActuatedRevolvingCentre",      /* V3.62  SE */
	/*126*/ "ShieldedKings",      		/* V3.62  SE */
	/*127*/ "StingChess",      		/* V3.63  NG */
	/*128*/ "LineChameleonChess",       	/* V3.64  TLi */
	/*129*/ "NoWhiteCapture",       	/* V3.64  TLi */
	/*130*/ "NoBlackCapture",      		/* V3.64  TLi */
	/*131*/ "AprilChess",                   /* V3.64  ThM */
	/*132*/ "AlphabeticChess",              /* V3.64  SE */
	/*133*/ "CirceTurncoats",               /* V3.70  SE */
	/*134*/ "CirceDoubleAgents",            /* V3.70  SE */
	/*135*/ "AMU",                          /* V3.70  SE */
	/*136*/ "SingleBox",                    /* V3.70  ThM, NG */
	/*137*/ "MAFF",                         /* V3.78  SE */
	/*138*/ "OWU",                          /* V3.78  SE */
	/*139*/ "WhiteOscillatingKings",        /* V3.78  SE */
	/*140*/ "BlackOscillatingKings",        /* V3.78  SE */
	/*141*/ "AntiKings",                    /* V3.78  SE */
	/*142*/ "AntiMarsCirce",	        /* V3.78  SE */
	/*143*/ "AntiMarsMirrorCirce",	        /* V3.78  SE */
	/*144*/ "AntiMarsAntipodeanCirce",	/* V3.78  SE */
	/*145*/ "WhiteSuperTransmutingKing",    /* V3.78  SE */
	/*146*/ "BlackSuperTransmutingKing",    /* V3.78  SE */
	/*147*/ "AntiSuperCirce",               /* V3.78  SE */
	/*148*/ "UltraPatrol",                  /* V3.78  SE */
	/*149*/ "SwappingKings",		/* V3.81a NG */
	/*150*/ "RoyalDynasty",                  /* V4.02 TM */
	/*151*/ "SAT",                       /* V4.03  SE */
	/*152*/ "StrictSAT",                 /* V4.03  SE */
	/*153*/ "Take&MakeChess",                 /* V4.03  SE */
	/*154*/ "BlackSynchronMover",                 /* V4.06  SE */
	/*155*/ "WhiteSynchronMover",                 /* V4.06  SE */
	/*156*/ "BlackAntiSynchronMover",                 /* V4.06  SE */
	/*157*/ "WhiteAntiSynchronMover",
	/*158*/ "Masand",                 /* V4.06  SE */
	/*159*/ "BGL",                     /* V4.06 SE */
	/*160*/ "BlackChecks",                     /* V4.06 SE */
	/*161*/ "AnnanChess",           /* V4.07 SE */
	/*162*/ "NormalPawn"           /* V4.07 SE */
  }
	};
#endif

/* The notation of stipulations is international, therefore we need
** not provide one for every language
*/
EXTERN unsigned int StipFlags;

#ifdef WE_ARE_EXTERN
	extern  char    *PieSpString[LangCount][PieSpCount];
	extern  char    **PieSpTab;
	extern  Flags   PieSpExFlags;
#else
	Flags   PieSpExFlags;   /* used for problem-wide piecespecification */
	char    **PieSpTab;
	char    *PieSpString[LangCount][PieSpCount] = {
	{
/* French */
	"Blanc",
	"Noir",
	"Neutre",
	"Kamikaze",
	"Royale",
	"Paralysante",
	"Chameleon",
	"Jigger",
	"Volage",
        "Fonctionnaire",
	"DemiNeutre",
	"CouleurEchangeantSautoir"
	},{
/* German */
	"Weiss",
	"Schwarz",
	"Neutral",
	"Kamikaze",
	"Koeniglich",
	"Paralysierend",
	"Chamaeleon",
	"Jigger",
	"Volage",
        "Beamtet",
	"HalbNeutral",
	"SprungbockFarbeWechselnd"
	},{
/* English */
	"White",
	"Black",
	"Neutral",
	"Kamikaze",
	"Royal",
	"Paralysing",
	"Chameleon",
	"Jigger",
	"Volage",
        "Functionary",
	"HalfNeutral",
	"HurdleColourChanging"
	}
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  char GlobalStr[];
	/* extern  char AlphaEnd[];  V3.31  TLi */
	extern  char *AlphaEnd;    /* V3.31  TLi */
	extern  char ReciAlphaEnd[];  /* V3.31  TLi */
	extern  char NonReciAlphaEnd[];
#else
	/* char    GlobalStr[256]; */
	char    GlobalStr[4*maxply];		/* V3.70  TLi */
	/* char    AlphaEnd[4];   V3.31  TLi */
	char    *AlphaEnd;   /* V3.31  TLi */
	char    ReciAlphaEnd[5];  /* V3.31  TLi */ /* V3.37  NG */
	char    NonReciAlphaEnd[5]; /* V3.31  TLi */ /* V3.37  NG */
#endif

/* V3.50 SE */
#ifdef WE_ARE_EXTERN
	extern char
*VariantTypeString[LangCount][VariantTypeCount];
#else
	char    *VariantTypeString[LangCount][VariantTypeCount] = {
	{
	/* French */
	/* 0*/  "TypeB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "MaximumNoir",		/* V3.78  SE bugfix - resolved conflict with Cond of same name */
	/* 6*/  "MaximumBlanc",		/* V3.78  SE bugfix - resolved conflict with Cond of same name */
        /* 7*/  "ParaSentinelles",
        /* 8*/  "MaximumTotal",		/* V4.30  NG renamed according to bugfix above */
	/* 9*/  "Berolina",
	/*10*/  "Type1",
	/*11*/  "Type2",
	/*12*/  "Type3",
	/*13*/  "Voisin",
	/*14*/  "TypeC",
	/*15*/  "TypeD"
	},{
	/* German */
	/* 0*/  "TypB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "MaximumSchwarz",	/* V4.30  NG renamed according to bugfix above */
	/* 6*/  "MaximumWeiss",		/* V4.30  NG renamed according to bugfix above */
        /* 7*/  "ParaSentinelles",
        /* 8*/  "MaximumTotal",		/* V4.30  NG renamed according to bugfix above */
	/* 9*/  "Berolina",
	/*10*/  "Typ1",
	/*11*/  "Typ2",
	/*12*/  "Typ3",
	/*13*/  "Nachbar",
	/*14*/  "TypC",
	/*15*/  "TypD"
	},{
	/* English */
	/* 0*/  "TypeB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "MaximumBlack",		/* V3.78  SE bugfix - resolved conflict with Cond of same name */
	/* 6*/  "MaximumWhite",		/* V3.78  SE bugfix - resolved conflict with Cond of same name */
        /* 7*/  "ParaSentinelles",
        /* 8*/  "MaximumTotal",		/* V4.30  NG renamed according to bugfix above */
	/* 9*/  "Berolina",
	/*10*/  "Type1",
	/*11*/  "Type2",
	/*12*/  "Type3",
	/*13*/  "Neighbour",
	/*14*/  "TypeC",
	/*15*/  "TypeD"
	}
	};
#endif

/* V3.62 SE */
#ifdef WE_ARE_EXTERN
	extern char *ExtraCondString[LangCount][ExtraCondCount];
#else
	char    *ExtraCondString[LangCount][ExtraCondCount] = {
	{
	/* French */
	/* 0*/  "Maximum",
	/* 1*/  "UltraSchachZwang"
	},{
	/* German */
	/* 0*/  "Laengstzueger",
	/* 1*/  "UltraSchachZwang"
	},{
	/* English */
	/* 0*/  "Maximummer",
	/* 1*/  "UltraSchachZwang"
	}
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  numvec mixhopdata[4][17];
#else
/* This are the vectors for hoppers which change directions after the hurdle */  /* V2.60  NG */
/* and for the CAT (a special concatenation of knight and dabbabba-rider     */  /* V2.60  NG */
	numvec mixhopdata[4][17] = {
	/* moose     45 */ { 0, -23,  25,  23,  25,  23, -25, -23, -25,
				 -1,  24,   1,  24,   1, -24,  -1, -24 },
	/* eagle     90 */ { 0,  24, -24,   1,  -1,  24, -24,   1,  -1,
				 25, -25,  23, -23,  25, -25,  23, -23 },
	/* sparrow  135 */ { 0,  23, -25, -23, -25, -23,  25,  23,  25,
				  1, -24,  -1, -24,  -1,  24,   1,  24 },
	/* CAT          */ { 0, -22,  47,  22, -47,  26,  49, -26, -49,
				 -2,  48,  48,   2,   2, -48, -48,  -2 }
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  numvec vec[maxvec + 1];
#else
/* don't try to delete something like "duplicates" or change
  the order of the vectors.
  they are all necessary and need this order !!
*/   							/* V2.60  NG */
numvec vec[maxvec + 1] = { 0,
/*   1 -   4 | 0,1 */    1,   24,   -1,  -24,
/*   5 -   8 | 1,1 */   23,   25,  -23,  -25,
/*   9 -  16 | 1,2 */   22,   47,   49,   26,  -22,  -47,  -49,  -26,
/*  17 -  24 | 1,2 */   22,   47,   49,   26,  -22,  -47,  -49,  -26,
/*  25 -  32 | 2,3 */   45,   70,   74,   51,  -45,  -70,  -74,  -51,
/*  33 -  40 | 1,3 */   21,   71,   73,   27,  -21,  -71,  -73,  -27,
/*  41 -  48 | 1,4 */   20,   95,   97,   28,  -20,  -95,  -97,  -28,
/*  49 -  56 | 3,4 */   68,   93,   99,   76,  -68,  -93,  -99,  -76,
/*  57 -  60 | 0,5 */    5,  120,   -5, -120,
/*  61 -  64 | 0,2 */    2,   48,   -2,  -48,
/*  65 -  68 | 2,2 */   46,   50,  -46,  -50,
/*  69 -  76 | 1,7 */   17,  167,  169,   31,  -17, -167, -169,  -31,
/*  77 -  80 | 5,5 */  115,  125, -115, -125,
/*  81 -  88 | 3,7 */   65,  165,  171,   79,  -65, -165, -171,  -79,
/*  89 -  96 | 1,6 */   18,  143,  145,   30,  -18, -143, -145,  -30,
/*  97 - 104 | 2,4 */   44,   94,   98,   52,  -44,  -94,  -98,  -52,
/* 105 - 112 | 3,5 */   67,  117,  123,   77,  -67, -117, -123,  -77,
/* 113 - 120 | 1,5 */   19,  119,  121,   29,  -19, -119, -121,  -29,
/* 121 - 128 | 2,5 */   43,  118,  122,   53,  -43, -118, -122,  -53,
/* 129 - 136 | 3,6 */   66,  141,  147,   78,  -66, -141, -147,  -78,
};
#endif

/* Symbols indicating start and end of a range inside vec - please
 * ALWAYS use these rather than int literals. */
enum {
  vec_rook_start= 1,              vec_rook_end=   4,
  vec_bishop_start=5,             vec_bishop_end=  8,
  vec_queen_start=1,              vec_queen_end=  8,
  vec_knight_start=9,             vec_knight_end=  16,
  vec_elephant_start=1,           vec_elephant_end=  16,
  vec_zebre_start=25,             vec_zebre_end=  32,
  vec_okapi_start=17,             vec_okapi_end=32,
  vec_chameau_start=33,           vec_chameau_end=  40,
  vec_bison_start=25,             vec_bison_end=  40,
  vec_equi_nonintercept_start=17, vec_equi_nonintercept_end=40,
  vec_girafe_start=41,            vec_girafe_end=  48,
  vec_antilope_start=49,          vec_antilope_end=  56,
  vec_bucephale_start=49,         vec_bucephale_end=  60,
  vec_dabbaba_start=61,           vec_dabbaba_end=  64,
  vec_alfil_start= 65,            vec_alfil_end=   68,
  vec_ecureuil_start= 61,         vec_ecureuil_end= 68, /* +knight vecs */
  vec_rccinq_start= 69,           vec_rccinq_end=   80,
  vec_leap37_start=81,            vec_leap37_end=  88,
  vec_leap16_start=89,            vec_leap16_end=  96,
  vec_leap24_start=97,            vec_leap24_end=  104,
  vec_leap35_start=105,           vec_leap35_end=  112,
  vec_leap15_start=113,           vec_leap15_end=  120,
  vec_leap25_start=121,           vec_leap25_end=  128,
  vec_leap36_start=129,           vec_leap36_end=  136
};


#ifdef WE_ARE_EXTERN
	extern  square boardnum[65];
#else
/* This are the really used boardnumbers */     /* V2.60  NG */
	square boardnum[65] = {
	/* first   rank */      bas, 201, 202, 203, 204, 205, 206, 207,
	/* second  rank */      224, 225, 226, 227, 228, 229, 230, 231,
	/* third   rank */      248, 249, 250, 251, 252, 253, 254, 255,
	/* fourth  rank */      272, 273, 274, 275, 276, 277, 278, 279,
	/* fifth   rank */      296, 297, 298, 299, 300, 301, 302, 303,
	/* sixth   rank */      320, 321, 322, 323, 324, 325, 326, 327,
	/* seventh rank */      344, 345, 346, 347, 348, 349, 350, 351,
	/* eighth  rank */      368, 369, 370, 371, 372, 373, 374, haut,
	/* end marker   */    0};
#endif


#ifdef WE_ARE_EXTERN
	extern  short move_diff_code[haut - bas + 1];
#else
/* This are the codes for the length-difference */      /* V2.90  NG */
/* between two squares */
/* ATTENTION: use abs(square from - square to) for indexing this table. */
/*        all move_down_codes are mapped this way to move_up_codes !    */

	short move_diff_code[haut - bas + 1]= {
	/* left/right   */        0,   1,   4,   9,  16,  25,  36,  49,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 1 left  up   */            50,  37,  26,  17,  10,   5,   2,
	/* 1 right up   */        1,   2,   5,  10,  17,  26,  37,  50,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 2 left  up   */            53,  40,  29,  20,  13,   8,   5,
	/* 2 right up   */        4,   5,   8,  13,  20,  29,  40,  53,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 3 left  up   */            58,  45,  34,  25,  18,  13,  10,
	/* 3 right up   */        9,  10,  13,  18,  25,  34,  45,  58,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 4 left  up   */            65,  52,  41,  32,  25,  20,  17,
	/* 4 right up   */       16,  17,  20,  25,  32,  41,  52,  65,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 5 left  up   */            74,  61,  50,  41,  34,  29,  26,
	/* 5 right up   */       25,  26,  29,  34,  41,  50,  61,  74,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 6 left  up   */            85,  72,  61,  52,  45,  40,  37,
	/* 6 right up   */       36,  37,  40,  45,  52,  61,  72,  85,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 7 left  up   */            98,  85,  74,  65,  58,  53,  50,
	/* 7 right up   */       49,  50,  53,  58,  65,  74,  85,  98
			       };
#endif

#ifdef WE_ARE_EXTERN   /* V4.06 SE */ 
 extern  long int BGL_move_diff_code[haut - bas + 1];
#else
 long int BGL_move_diff_code[haut - bas + 1]= {
 /* left/right   */        0,   100,   200,   300,  400,  500,  600,  700,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 1 left  up   */            707,  608,  510,  412,  316,   224,   141,
 /* 1 right up   */        100,   141,   224,  316,  412,  510,  608,  707,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 2 left  up   */            728,  632,  539,  447,  361,   283,   224,
 /* 2 right up   */        200,   224,   283,  361,  447,  539,  632,  728,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 3 left  up   */            762,  671,  583,  500,  424,  361,  316,
 /* 3 right up   */        300,  316,  361,  424,  500,  583,  671,  762,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 4 left  up   */            806,  721,  640,  566,  500,  447,  412,
 /* 4 right up   */       400,  412,  447,  500,  566,  640,  721,  806,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 5 left  up   */            860,  781,  707,  640,  583,  539,  510,
 /* 5 right up   */       500,  510,  539,  583,  640,  707,  781,  860,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 6 left  up   */            922,  849,  781,  721,  671,  632,  608,
 /* 6 right up   */       600,  608,  632,  671,  721,  781,  849,  922,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 7 left  up   */            990,  922,  860,  806,  762,  728,  707,
 /* 7 right up   */       700,  707,  728,  762,  806,  860,  922,  990
          };
#endif

#ifdef WE_ARE_EXTERN
	extern  checkfunction_t *checkfunctions[derbla + 1];
#else
/* This are the used checkingfunctions  */     /* V2.60  NG */
    checkfunction_t *checkfunctions[derbla + 1] = {
/*  0 */        0, /* not used */
/*  1 */        0, /* not used */
/*  2 */        roicheck,
/*  3 */        pioncheck,
/*  4 */        damecheck,
/*  5 */        cavcheck,
/*  6 */        tourcheck,
/*  7 */        foucheck,
/*  8 */        leocheck,
/*  9 */        maocheck,
/* 10 */        paocheck,
/* 11 */        vaocheck,
/* 12 */        rosecheck,
/* 13 */        scheck,
/* 14 */        ncheck,
/* 15 */        zcheck,
/* 16 */        chcheck,
/* 17 */        gicheck,
/* 18 */        rccinqcheck,
/* 19 */        bucheck,
/* 20 */        vizircheck,
/* 21 */        alfilcheck,
/* 22 */        ferscheck,
/* 23 */        dabcheck,
/* 24 */        lioncheck,
/* 25 */        nequicheck,
/* 26 */        loccheck,
/* 27 */        pbcheck,
/* 28 */        amazcheck,
/* 29 */        impcheck,
/* 30 */        princcheck,
/* 31 */        gnoucheck,
/* 32 */        antilcheck,
/* 33 */        ecurcheck,
/* 34 */        warancheck,
/* 35 */        dragoncheck,
/* 36 */        kangoucheck,
/* 37 */        cscheck,
/* 38 */        ubicheck,
/* 39 */        nevercheck,	/* hamster cannot check */
/* 40 */        moosecheck,
/* 41 */        eaglecheck,
/* 42 */        sparrcheck,
/* 43 */        archcheck,
/* 44 */        reffoucheck,
/* 45 */        cardcheck,
/* 46 */        nsautcheck,
/* 47 */        nevercheck,	/* dummy cannot check */
/* 48 */        camridcheck,
/* 49 */        zebridcheck,
/* 50 */        gnuridcheck,
/* 51 */        camhopcheck,
/* 52 */        zebhopcheck,
/* 53 */        gnuhopcheck,
/* 54 */        dcscheck,
/* 55 */        refccheck,
/* 56 */        equicheck,
/* 57 */        catcheck,
/* 58 */        bspawncheck,
/* 59 */        spawncheck,
/* 60 */        t_lioncheck,
/* 61 */        f_lioncheck,
/* 62 */        loccheck,               /* sirene checks like locust */
/* 63 */        tritoncheck,
/* 64 */        nereidecheck,
/* 65 */        orphancheck,
/* 66 */        edgehcheck,
/* 67 */        moacheck,
/* 68 */        rhuntcheck,
/* 69 */        bhuntcheck,
/* 70 */        maoridercheck,
/* 71 */        moaridercheck,
/* 72 */        r_hopcheck,
/* 73 */        b_hopcheck,
/* 74 */        roicheck,               /* erlking checks like king */
/* 75 */        bscoutcheck,
/* 76 */        gscoutcheck,
/* 77 */        skyllacheck,
/* 78 */        charybdischeck,
/* 79 */        contragrascheck,
/* 80 */        roselioncheck,
/* 81 */        rosehoppercheck,
/* 82 */        okapicheck,             /* V3.32  TLi */
/* 83 */        leap37check,            /* V3.34  TLi */
/* 84 */        grasshop2check,         /* V3.34  TLi */
/* 85 */        grasshop3check,         /* V3.34  TLi */
/* 86 */        leap16check,            /* V3.38  NG */
/* 87 */        leap24check,            /* V3.42  TLi */
/* 88 */        leap35check,            /* V3.42  TLi */
/* 89 */        doublegrascheck,        /* V3.44  SE */
/* 90 */        kinghopcheck,           /* V3.44  SE */
/* 91 */        orixcheck,              /* V3.44  NG */
/* 92 */        leap15check,            /* V3.46  NG */
/* 93 */        leap25check,            /* V3.46  NG */
/* 94 */        gralcheck,              /* V3.46  NG */
/* 95 */        tritoncheck,            /* V3.53  TLi */
/* 96 */        nereidecheck,           /* V3.53  TLi */
/* 97 */        nightlocustcheck,       /* V3.53  TLi */
/* 98 */        tourcheck,              /* wazirrider checks like rook */
/* 99 */        foucheck,               /* fersrider checks like bishop */
/*100 */        bisoncheck,             /* V3.60  TLi */
/*101 */        elephantcheck,          /* V3.62  TLi */
/*102 */        naocheck,               /* V3.62  TLi */
/*103 */        rookmoosecheck,		/* V3.62  NG */
/*104 */        rookeaglecheck,		/* V3.62  NG */
/*105 */        rooksparrcheck,		/* V3.62  NG */
/*106 */        bishopmoosecheck,	/* V3.62  NG */
/*107 */        bishopeaglecheck,	/* V3.62  NG */
/*108 */        bishopsparrcheck,       /* V3.62  NG */
/*109 */        roselioncheck,  	/* rao checks like roselion */ /* V3.63  NG */
/*110 */        scorpioncheck,  	/* V3.63  NG */
/*111 */        margueritecheck,  	/* V3.64  TLi */
/*112 */        leap36check,    	/* V3.64  TLi */
/*113 */        nightriderlioncheck,    /* V3.64  TLi */
/*114 */        maoriderlioncheck,      /* V3.64  TLi */
/*115 */        moariderlioncheck,      /* V3.65  TLi */
/*116 */        scheck,                 /* V3.65  TLi */
/*117 */        friendcheck,            /* V3.65  TLi */
/*118 */        dolphincheck,           /* V3.70  TLi */
/*119 */        rabbitcheck,            /* V3.76  NG */
/*120 */        bobcheck,               /* V3.76  NG */
/*121 */	equiengcheck,           /* V3.78  SE */
/*122 */	equifracheck,           /* V3.78  SE */
/*123 */	querquisitecheck,	/* V3.78  SE */
/*124 */	bouncercheck,		/* V4.03  SE */
/*125 */	rookbouncercheck,	/* V4.03  SE */
/*126 */	bishopbouncercheck,	/* V4.03  SE */
/*127 */	pchincheck,		/* V4.06  SE */
/*128 */        huntercheck,
/*129 */        huntercheck,
/*130 */        huntercheck,
/*131 */        huntercheck,
/*132 */        huntercheck,
/*133 */        huntercheck,
/*134 */        huntercheck,
/*135 */        huntercheck,
/*136 */        huntercheck,
/*137 */        huntercheck
		};
#endif

#ifdef WE_ARE_EXTERN
	extern  piece PAS[64];
#else
/* This is the InitialGameArray */     /* V3.50  NG */
piece       PAS[64] = {   tb,   cb,   fb,   db, roib,   fb,   cb,   tb,
			  pb,   pb,   pb,   pb,   pb,   pb,   pb,   pb,
			vide, vide, vide, vide, vide, vide, vide, vide,
			vide, vide, vide, vide, vide, vide, vide, vide,
			vide, vide, vide, vide, vide, vide, vide, vide,
			vide, vide, vide, vide, vide, vide, vide, vide,
			  pn,   pn,   pn,   pn,   pn,   pn,   pn,   pn,
			  tn,   cn,   fn,   dn, roin,   fn,   cn,   tn};
#endif

typedef struct {
    PieNam home;
    PieNam away;
} HunterType;

extern HunterType huntertypes[maxnrhuntertypes];
extern unsigned int nrhuntertypes;

#define hunterseparator '/'

#endif  /* not PYDATA_H */

