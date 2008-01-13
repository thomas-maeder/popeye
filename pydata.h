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
** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
**
** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
**                 Transmuting/Reflecting Ks now take optional piece list
**                 turning them into vaulting types
**
** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
**
** 2007/12/21 SE   Command-line switch: -maxtime (same func as Option)
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
**************************** End of List ******************************/

#ifndef PYDATA_H
#define PYDATA_H

#include "pyproc.h"

#ifndef EXTERN
#       define EXTERN extern
#       define WE_ARE_EXTERN
#endif  /* EXTERN */

#ifdef  WE_ARE_EXTERN
extern  unsigned long   MaxMemory;
extern int MaxTime;
extern  char    *StartUp;
extern  long    MaxPositions;
extern  boolean LaTeXout;
extern  int     GlobalThreadCounter;
extern  boolean flag_regression;
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
int MaxTime = -1;
char            *StartUp = VERSIONSTRING;
boolean         LaTeXout= false;
int     	GlobalThreadCounter= 0;
boolean         flag_regression= false;
#endif  /* WE_ARE_EXTERN */

EXTERN  char            MMString[37];   /* MaxMemory available for diagnostic message */
					/* Safety first, think at the future (:-) */

EXTERN  long            PositionCnt;

EXTERN  boolean         FlagGenMatingMove, FlagMoveOrientatedStip;
EXTERN  boolean         flagpassive, flagcapture,
			flagactive,
			flagwhitemummer,
			flagblackmummer;

EXTERN  square          rb, rn;
EXTERN  square          bl_royal_sq, wh_royal_sq;

EXTERN  echiquier       e, e_ubi, e_ubi_mad;

EXTERN  boolean         mateallowed[maxply + 1];
EXTERN  boolean         senti[maxply + 1],
			norm_cham_prom[maxply + 1],
			cir_cham_prom[maxply + 1];

EXTERN struct
{
    square where;
    piece what;
} sb2[maxply+1];

EXTERN  square          ppridia[maxply + 1];

EXTERN  square
			ep[maxply + 1],
			ep2[maxply + 1],
			whduell[maxply + 1],
			blduell[maxply + 1],
			RN_[maxply + 1],
			RB_[maxply + 1];

EXTERN  square          pattfld, patti, pattj;
EXTERN move_generation_elmt move_generation_stack[toppile + 1];

#ifdef  WE_ARE_EXTERN
EXTERN smallint maxinbox[];
EXTERN unsigned int boxsize;
#endif

EXTERN struct
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

EXTERN  smallint        maxflights;


/* symbols for bits in castling_flag */
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

/* symbols for bit combinations in castling_flag */
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
EXTERN  unsigned char   no_castling;
EXTERN  short           castling_supported;
EXTERN  boolean         testcastling;

/* Symbols for squares - using these makes code much more human-readable */
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
EXTERN  boolean         FlagMaxSolsReached;
EXTERN  boolean         FlagShortSolsReached;

/* Stop solving when a given time (in seconds) for solving is reached */

EXTERN  int             maxsolvingtime;
EXTERN  boolean         FlagTimeOut;
EXTERN  boolean         FlagTimerInUse;

/* beep maxbeep times if a solution is found */

EXTERN  int             maxbeep;

/* Optimierung direkte Aufgaben */
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
			ren_parrain[maxply + 1],
			jouearr[maxply + 1];

EXTERN  boolean         flende,                 /* flagpatrsout,        */
			SatzFlag,DrohFlag,              
			rex_mad,rex_circe,rex_immun,rex_phan, rex_geneva,
                        rex_mess_ex, rex_wooz_ex;
EXTERN  boolean         pwcprom[maxply + 1];

EXTERN  pilecase        sqrenais,
			crenkam,        /* rebirth square for taking kamikaze pieces */
			super;          /* supercirce */
EXTERN  Flags           jouespec[maxply + 1];
EXTERN  numecoup        repere[maxply + 1];

EXTERN  boolean         exist[derbla + 1];
EXTERN  boolean         promonly[derbla + 1];

EXTERN  evalfunction_t *eval_white;
EXTERN  evalfunction_t *eval_2;
EXTERN  evalfunction_t *eval_black;
EXTERN boolean		(* ReciStipulation)(couleur),
		(* NonReciStipulation)(couleur),
		(* stipulation)(couleur);
EXTERN  short   (* white_length)(square departure, square arrival, square capture),
		(* black_length)(square departure, square arrival, square capture);

typedef square  (* renaifunc)(piece, Flags, square, square, square, couleur);

EXTERN  renaifunc immunrenai, circerenai, antirenai, marsrenai, genevarenai;

EXTERN  boolean         anycirce, anycirprom, anyanticirce, anyimmun, anyclone, anygeneva;

/* new anticirce implementation */
EXTERN  boolean         AntiCirCheylan;
EXTERN  enum
{
  singlebox_type1,
  singlebox_type2,
  singlebox_type3
} SingleBoxType;

EXTERN  pilecouleur     trait;
EXTERN  boolean         flagfee,
			flagriders, flagleapers, flagsimplehoppers, 
                        flagsimpledecomposedleapers, flagsymmetricfairy,
			flagveryfairy,
			flagleofamilyonly,
			empilegenre,
			flaglegalsquare, jouegenre,
			repgenre, change_moving_piece;

EXTERN  square          cirrenroib, cirrenroin,
			immrenroib, immrenroin;

EXTERN  boolean         PrintReciSolution;

EXTERN  smallint        marge;
EXTERN  numecoup        debut;
EXTERN  numecoup        testdebut;
EXTERN  tab             tabsol;

EXTERN  Flags           pprispec[maxply + 1];

EXTERN  couleur         zzzag[haut - bas + 1];  /* sqcolor */

EXTERN  Flags           spec[maxsquare+4];
EXTERN  smallint        zzzaj[haut - bas + 1];  /* gridnumber */

EXTERN  smallint        NonTrivialNumber, NonTrivialLength;

EXTERN  Flags           zzzan[haut - bas + 1];  /* sq_spec */
EXTERN  boolean         we_generate_exact, there_are_consmoves,
			wh_exact, bl_exact, wh_ultra, bl_ultra;

EXTERN  smallint        inum[maxply + 1];       /* aktuelle Anzahl Imitatoren */

EXTERN  imarr           isquare;                /* Imitatorstandfelder */

EXTERN  boolean         Iprom[maxply + 1];      /* Imitatorumwandlung? */
EXTERN  square          im0;                    /* Standort des ersten Imitators */

EXTERN  smallint        enonce,
			zugebene, maxdefen, droh;
EXTERN  smallint        introenonce;
EXTERN  smallint        MoveNbr, RestartNbr;
EXTERN  boolean         restartenabled;
EXTERN  couleur         neutcoul;
EXTERN  boolean         calctransmute;

EXTERN  boolean         flag_testlegality, k_cap,
			flag_writinglinesolution,
			anymars,
			flag_madrasi;
EXTERN  boolean         is_phantomchess;
EXTERN  square          marsid;
EXTERN  square          TargetSquare, ReciTargetSquare, NonReciTargetSquare;

EXTERN  boolean         DoubleMate, CounterMate, ReciDoubleMate, NonReciDoubleMate;

EXTERN  piece           getprompiece[derbla + 1];       /* it's a inittable ! */
EXTERN  piece           checkpieces[derbla - leob + 1]; /* only fairies ! */

EXTERN  piece           whitetransmpieces[derbla],
                        blacktransmpieces[derbla];
EXTERN  boolean         whitenormaltranspieces,
                        blacknormaltranspieces;
EXTERN  piece           orphanpieces[derbla + 1];

EXTERN  boolean         ProofFairy;

EXTERN  couleur         maincamp;

EXTERN  piece           NextChamCircePiece[PieceCount];
EXTERN  boolean         InitChamCirce;

EXTERN unsigned char    more_ren;
EXTERN boolean          IsardamB,PatienceB,SentPionAdverse,SentPionNeutral,
			flagAssassin,flag_nk,jouetestgenre;
EXTERN boolean          flagdiastip, flag_dontaddk;
EXTERN boolean          mate_or_stale_patt;
EXTERN smallint         max_pn, max_pb;
EXTERN boolean          flagmaxi, flagultraschachzwang, flagparasent;
EXTERN smallint         max_pt;
EXTERN boolean          isapril[derbla + 1];

EXTERN boolean          checkhopim;
EXTERN square           chop[toppile + 1];

EXTERN piece            sentinelb, sentineln;
EXTERN boolean          anytraitor;
EXTERN boolean          att_1[maxply + 1];
EXTERN boolean          flag_atob;
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
EXTERN long int         BGL_black, BGL_white;
EXTERN boolean          BGL_global, BGL_blackinfinity, BGL_whiteinfinity;
EXTERN boolean			  	blacknull, nullgenre, whitenull;
EXTERN smallint         annanvar;
EXTERN numecoup         tempcoup;
EXTERN ply      	      tempply;
EXTERN boolean          jouetestgenre1, jouetestgenre_save;
EXTERN boolean          rex_protean_ex;
EXTERN int              gridvar, currentgridnum;
EXTERN boolean          calc_whtrans_king, calc_bltrans_king, 
                        calc_whrefl_king, calc_blrefl_king;

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
    /*127*/ {'p','c'},  /* pion chinois */
    /*128*/ {'r','k'},  /* Cavalier Radiale */
    /*129*/ {'p','p'}  /* Pion Protean */
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
    /*127*/ {'c','b'},  /* Chinesischer Bauer */
    /*128*/ {'r','k'},  /* Radial Springer */
    /*129*/ {'p','p'} /* Protean Bauer */
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
    /*127*/ {'c','p'},  /* chinese pawn */	
    /*128*/ {'r','k'},  /* radial knight */	
    /*129*/ {'p','p'},  /* protean pawn */	
  }
	};
#endif

#ifdef WE_ARE_EXTERN
	extern Lang     ActLang;
	extern char     ActAuthor[];
	extern char     ActOrigin[];
	extern char     ActTitle[];
	extern char     ActTwin[];
	extern char     ActAward[], ActStip[];
#else
	Lang            ActLang;
	char            ActAuthor[256];
	char            ActOrigin[256];
	char            ActTitle[256];
	char            ActTwin[1532];
	char            ActAward[256], ActStip[37];
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
	/*29*/  "Bip",
	/*30*/  "SupprimezGrille",
	/*31*/  "EcriVezGrille"
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
	/*29*/  "Pieps",
	/*30*/  "UnterdruckenGitter",
	/*31*/  "SchreibenGitter"
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
	/*29*/  "Beep",
	/*30*/  "SuppressGrid",
	/*31*/  "WriteGrid"
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
	/*20*/  "Imitator",
	/*21*/  "CavalierMajeur",
	/*22*/  "Haan",
	/*23*/  "CirceCameleon",
	/*24*/  "CirceCouscous",
	/*25*/  "CirceEquipollents",
	/*26*/  "FileCirce",
	/*27*/  "NoirMaximum",
	/*28*/  "NoirMinimum",
	/*29*/  "BlancMaximum",
	/*30*/  "BlancMinimum",
	/*31*/  "CaseMagique",
	/*32*/  "Sentinelles",
	/*33*/  "Tibet",
	/*34*/  "DoubleTibet",
	/*35*/  "CirceDiagramme",
	/*36*/  "Trou",
	/*37*/  "NoirPriseForce",
	/*38*/  "BlancPriseForce",
	/*39*/  "RoisReflecteurs",
	/*40*/  "RoisTransmutes",
	/*41*/  "NoirPoursuite",
	/*42*/  "BlancPoursuite",
	/*43*/  "Duelliste",
	/*44*/  "CirceParrain",
	/*45*/  "SansIProm",
	/*46*/  "CirceSymetrique",
	/*47*/  "EchecsVogtlaender",
	/*48*/  "EchecsEinstein",
	/*49*/  "Bicolores",
	/*50*/  "KoekoNouveaux",
	/*51*/  "CirceClone",
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
	/*68*/  "ImmunEquipollents",
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
	/*80*/  "ultra",
	/*81*/  "EchecsChameleon",
	/*82*/  "EchecsFonctionnaire",
	/*83*/  "EchecsGlasgow",
	/*84*/  "EchecsAntiAndernach",
	/*85*/  "FrischAufCirce",
	/*86*/  "CirceMalefiqueVerticale",
	/*87*/  "Isardam",
	/*88*/  "SansEchecs",
	/*89*/  "CirceDiametrale",
	/*90*/  "PromSeul",
	/*91*/  "RankCirce",
	/*92*/  "EchecsExclusif",
	/*93*/  "MarsCirce",
	/*94*/  "MarsCirceMalefique",
	/*95*/  "EchecsPhantom",
	/*96*/  "BlancRoiReflecteur",
	/*97*/  "NoirRoiReflecteur",
	/*98*/  "BlancRoiTransmute",
	/*99*/  "NoirRoiTransmute",
	/*100*/ "EchecsAntiEinstein",
	/*101*/ "CirceCouscousMalefique",
	/*102*/ "NoirCaseRoyal",
	/*103*/ "BlancCaseRoyal",
	/*104*/ "EchecsBrunner",
	/*105*/ "EchecsPlus",
	/*106*/ "CirceAssassin",
	/*107*/ "EchecsPatience",
	/*108*/ "EchecsRepublicains",
	/*109*/ "EchecsExtinction",
	/*110*/ "EchecsCentral",
	/*111*/ "ActuatedRevolvingBoard",
	/*112*/ "EchecsMessigny",
	/*113*/ "Woozles",
	/*114*/ "BiWoozles",
	/*115*/ "Heffalumps",
	/*116*/ "BiHeffalumps",
	/*117*/ "RexExclusif",
	/*118*/ "BlancCasePromotion",
	/*119*/ "NoirCasePromotion",
	/*120*/ "SansBlancPromotion",
	/*121*/ "SansNoirPromotion",
	/*122*/ "EchecsEiffel",
	/*123*/ "NoirUltraSchachZwang",
	/*124*/ "BlancUltraSchachZwang",
	/*125*/ "ActuatedRevolvingCentre",
	/*126*/ "ShieldedKings",
	/*127*/ "EchecsSting",
	/*128*/ "EchecsChameleonLigne",
	/*129*/ "BlancSansPrises",
	/*130*/ "NoirSansPrises",
	/*131*/ "EchecsAvril",
	/*132*/ "EchecsAlphabetiques",
	/*133*/ "CirceTurncoats",
	/*134*/ "CirceDoubleAgents",
	/*135*/ "AMU",
 	/*136*/ "SingleBox",
	/*137*/ "MAFF",
	/*138*/ "OWU",
	/*139*/ "BlancRoisOscillant",
	/*140*/ "NoirRoisOscillant",
	/*141*/ "AntiRois",
	/*142*/ "AntiMarsCirce",
	/*143*/ "AntiMarsMalefiqueCirce",
	/*144*/ "AntiMarsAntipodeanCirce",
	/*145*/ "BlancSuperRoiTransmute",
	/*146*/ "NoirSuperRoiTransmute",
	/*147*/ "AntiSuperCirce",
	/*148*/ "UltraPatrouille",
	/*149*/ "RoisEchanges",
	/*150*/ "DynastieRoyale",
	/*151*/ "SAT",
	/*152*/ "StrictSAT",
	/*153*/ "Take&MakeEchecs",
	/*154*/ "NoirSynchronCoup",
	/*155*/ "BlancSynchronCoup",
	/*156*/ "NoirAntiSynchronCoup",
	/*157*/ "BlancAntiSynchronCoup",
	/*158*/ "Masand",
	/*159*/ "BGL" ,
	/*160*/ "NoirEchecs",
  	/*161*/ "AnnanEchecs" ,
  	/*162*/ "PionNormale",
    /*163*/ "Elliuortap",
    /*164*/ "RoisSautants",
    /*165*/ "ProteanEchecs",
  	/*166*/ "EchecsGeneve",
  	/*167*/ "BlancRoisSautant",
  	/*168*/ "NoirRoisSautant"
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
	/*20*/  "Imitator",
	/*21*/  "CavalierMajeur",
	/*22*/  "HaanerSchach",
	/*23*/  "ChamaeleonCirce",
	/*24*/  "CouscousCirce",
	/*25*/  "EquipollentsCirce",
	/*26*/  "FileCirce",
	/*27*/  "SchwarzerLaengstzueger",
	/*28*/  "SchwarzerKuerzestzueger",
	/*29*/  "WeisserLaengstzueger",
	/*30*/  "WeisserKuerzestzueger",
	/*31*/  "MagischeFelder",
	/*32*/  "Sentinelles",
	/*33*/  "TibetSchach",
	/*34*/  "DoppeltibetSchach",
	/*35*/  "DiagrammCirce",
	/*36*/  "Loch",
	/*37*/  "SchwarzerSchlagzwang",
	/*38*/  "WeisserSchlagzwang",
	/*39*/  "ReflektierendeKoenige",
	/*40*/  "TransmutierendeKoenige",
	/*41*/  "SchwarzerVerfolgungszueger",
	/*42*/  "WeisserVerfolgungszueger",
	/*43*/  "Duellantenschach",
	/*44*/  "CirceParrain",
	/*45*/  "OhneIUW",
	/*46*/  "SymmetrieCirce",
	/*47*/  "VogtlaenderSchach",
	/*48*/  "EinsteinSchach",
	/*49*/  "Bicolores",
	/*50*/  "NeuKoeko",
	/*51*/  "CirceClone",
	/*52*/  "AntiCirce",
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
	/*81*/  "ChamaeleonSchach",
	/*82*/  "BeamtenSchach",
	/*83*/  "GlasgowSchach",
	/*84*/  "AntiAndernachSchach",
	/*85*/  "FrischAufCirce",
	/*86*/  "VertikalesSpiegelCirce",
	/*87*/  "Isardam",
	/*88*/  "OhneSchach",
	/*89*/  "DiametralCirce",
	/*90*/  "UWnur",
	/*91*/  "RankCirce",
	/*92*/  "ExklusivSchach",
	/*93*/  "MarsCirce",
	/*94*/  "MarsSpiegelCirce",
	/*95*/  "PhantomSchach",
	/*96*/  "WeisserReflektierenderKoenig",
	/*97*/  "SchwarzerReflektierenderKoenig",
	/*98*/  "WeisserTransmutierenderKoenig",
	/*99*/  "SchwarzerTransmutierenderKoenig",
	/*100*/ "AntiEinsteinSchach",
	/*101*/ "SpiegelCouscousCirce",
	/*102*/ "SchwarzesKoeniglichesFeld",
	/*103*/ "WeissesKoeniglichesFeld",
	/*104*/ "BrunnerSchach",
	/*105*/ "PlusSchach",
	/*106*/ "AssassinCirce",
	/*107*/ "PatienceSchach",
	/*108*/ "RepublikanerSchach",
	/*109*/ "AusrottungsSchach",
	/*110*/ "ZentralSchach",
	/*111*/ "ActuatedRevolvingBoard",
	/*112*/ "MessignySchach",
	/*113*/ "Woozles",
	/*114*/ "BiWoozles",
	/*115*/ "Heffalumps",
	/*116*/ "BiHeffalumps",
	/*117*/ "RexExklusive",
	/*118*/ "UWFeldWeiss",
	/*119*/ "UWFeldSchwarz",
	/*120*/ "OhneWeissUW",
	/*121*/ "OhneSchwarzUW",
	/*122*/ "EiffelSchach",
	/*123*/ "SchwarzerUltraSchachZwang",
	/*124*/ "WeisserUltraSchachZwang",
	/*125*/ "ActuatedRevolvingCentre",
	/*126*/ "SchutzKoenige",
	/*127*/ "StingSchach",
	/*128*/ "LinienChamaeleonSchach",
	/*129*/ "WeisserOhneSchlag",
	/*130*/ "SchwarzerOhneSchlag",
	/*131*/ "Aprilschach",
	/*132*/ "AlphabetischesSchach",
	/*133*/ "TurncoatCirce",
	/*134*/ "DoppelAgentenCirce",
	/*135*/ "AMU",
    /*136*/ "NurPartiesatzSteine",
    /*137*/ "MAFF",
    /*138*/ "OWU",
    /*139*/ "WeisseOszillierendeKoenige",
    /*140*/ "SchwarzeOszillierendeKoenige",
    /*141*/ "AntiKoenige",
	/*142*/ "AntiMarsCirce",
	/*143*/ "AntiMarsMalefiqueCirce",
	/*144*/ "AntiMarsAntipodeanCirce",
	/*145*/ "WeisserSuperTransmutierenderKoenig",
	/*146*/ "SchwarzerSuperTransmutierenderKoenig",
	/*147*/ "AntiSuperCirce",
	/*148*/ "UltraPatrouille",
	/*149*/ "TauschKoenige",
	/*150*/ "KoenigsDynastie",
	/*151*/ "SAT",
	/*152*/ "StrictSAT",
	/*153*/ "Take&MakeSchach",
	/*154*/ "SchwarzerSynchronZueger",
	/*155*/ "WeisserSynchronZueger",
	/*156*/ "SchwarzerAntiSynchronZueger",
	/*157*/ "WeisserAntiSynchronZueger",
	/*158*/ "Masand",
	/*159*/ "BGL",
	/*160*/ "SchwarzSchaecher",
	/*161*/ "Annanschach",
	/*162*/ "NormalBauern",
    /*163*/ "Lortap",
    /*164*/ "SpringenKonigen",
    /*165*/ "ProteanSchach",
  	/*166*/ "GenferSchach",
  	/*167*/ "WeissSpringenKonig",
  	/*168*/ "SchwarzSpringenKonig"
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
	/*20*/  "Imitator",
	/*21*/  "CavalierMajeur",
	/*22*/  "HaanerChess",
	/*23*/  "ChameleonCirce",
	/*24*/  "CouscousCirce",
	/*25*/  "EquipollentsCirce",
	/*26*/  "FileCirce",
	/*27*/  "BlackMaximummer",
	/*28*/  "BlackMinimummer",
	/*29*/  "WhiteMaximummer",
	/*30*/  "WhiteMinimummer",
	/*31*/  "MagicSquares",
	/*32*/  "Sentinelles",
	/*33*/  "Tibet",
	/*34*/  "DoubleTibet",
	/*35*/  "DiagramCirce",
	/*36*/  "Hole",
	/*37*/  "BlackMustCapture",
	/*38*/  "WhiteMustCapture",
	/*39*/  "ReflectiveKings",
	/*40*/  "TransmutedKings",
	/*41*/  "BlackFollowMyLeader",
	/*42*/  "WhiteFollowMyLeader",
	/*43*/  "DuellistChess",
	/*44*/  "ParrainCirce",
	/*45*/  "NoIProm",
	/*46*/  "SymmetryCirce",
	/*47*/  "VogtlaenderChess",
	/*48*/  "EinsteinChess",
	/*49*/  "Bicolores",
	/*50*/  "NewKoeko",
	/*51*/  "CirceClone",
	/*52*/  "AntiCirce",
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
	/*81*/  "ChameleonChess",
	/*82*/  "FunctionaryChess",
	/*83*/  "GlasgowChess",
	/*84*/  "AntiAndernachChess",
	/*85*/  "FrischAufCirce",
	/*86*/  "VerticalMirrorCirce",
	/*87*/  "Isardam",
	/*88*/  "ChecklessChess",
	/*89*/  "DiametralCirce",
	/*90*/  "PromOnly",
	/*91*/  "RankCirce",
	/*92*/  "ExclusivChess",
	/*93*/  "MarsCirce",
	/*94*/  "MarsMirrorCirce",
	/*95*/  "PhantomChess",
	/*96*/  "WhiteReflectiveKing",
	/*97*/  "BlackReflectiveKing",
	/*98*/  "WhiteTransmutedKing",
	/*99*/  "BlackTransmutedKing",
	/*100*/ "AntiEinsteinChess",
	/*101*/ "MirrorCouscousCirce",
	/*102*/ "BlackRoyalSquare",
	/*103*/ "WhiteRoyalSquare",
	/*104*/ "BrunnerChess",
	/*105*/ "PlusChess",
	/*106*/ "CirceAssassin",
	/*107*/ "PatienceChess",
	/*108*/ "RepublicanChess",
	/*109*/ "ExtinctionChess",
	/*110*/ "CentralChess",
	/*111*/ "ActuatedRevolvingBoard",
	/*112*/ "MessignyChess",
	/*113*/ "Woozles",
	/*114*/ "BiWoozles",
	/*115*/ "Heffalumps",
	/*116*/ "BiHeffalumps",
	/*117*/ "RexExclusiv",
	/*118*/ "WhitePromSquares",
	/*119*/ "BlackPromSquares",
	/*120*/ "NoWhitePromotion",
	/*121*/ "NoBlackPromotion",
	/*122*/ "EiffelChess",
	/*123*/ "BlackUltraSchachZwang",
	/*124*/ "WhiteUltraSchachZwang",
	/*125*/ "ActuatedRevolvingCentre",
	/*126*/ "ShieldedKings",
	/*127*/ "StingChess",
	/*128*/ "LineChameleonChess",
	/*129*/ "NoWhiteCapture",
	/*130*/ "NoBlackCapture",
	/*131*/ "AprilChess",
	/*132*/ "AlphabeticChess",
	/*133*/ "CirceTurncoats",
	/*134*/ "CirceDoubleAgents",
	/*135*/ "AMU",
	/*136*/ "SingleBox",
	/*137*/ "MAFF",
	/*138*/ "OWU",
	/*139*/ "WhiteOscillatingKings",
	/*140*/ "BlackOscillatingKings",
	/*141*/ "AntiKings",
	/*142*/ "AntiMarsCirce",
	/*143*/ "AntiMarsMirrorCirce",
	/*144*/ "AntiMarsAntipodeanCirce",
	/*145*/ "WhiteSuperTransmutingKing",
	/*146*/ "BlackSuperTransmutingKing",
	/*147*/ "AntiSuperCirce",
	/*148*/ "UltraPatrol",
	/*149*/ "SwappingKings",
	/*150*/ "RoyalDynasty",
	/*151*/ "SAT",
	/*152*/ "StrictSAT",
	/*153*/ "Take&MakeChess",
	/*154*/ "BlackSynchronMover",
	/*155*/ "WhiteSynchronMover",
	/*156*/ "BlackAntiSynchronMover",
	/*157*/ "WhiteAntiSynchronMover",
	/*158*/ "Masand",
	/*159*/ "BGL",
	/*160*/ "BlackChecks",
	/*161*/ "AnnanChess",
	/*162*/ "NormalPawn",
    /*163*/ "Lortap",
    /*164*/ "VaultingKings",
    /*165*/ "ProteanChess",
    /*166*/ "GenevaChess",
  	/*167*/ "WhiteVaultingKing",
  	/*168*/ "BlackVaultingKing"
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
      "CouleurEchangeantSautoir",
      "Protean"
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
      "SprungbockFarbeWechselnd",
      "Protean"
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
      "HurdleColourChanging",
      "Protean"
	}
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  char GlobalStr[];
	extern  char *AlphaEnd;
	extern  char ReciAlphaEnd[];
	extern  char NonReciAlphaEnd[];
#else
	char    GlobalStr[4*maxply];
	char    *AlphaEnd;
	char    ReciAlphaEnd[5];
	char    NonReciAlphaEnd[5];
#endif

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
	/* 5*/  "MaximumNoir",
	/* 6*/  "MaximumBlanc",
        /* 7*/  "ParaSentinelles",
        /* 8*/  "MaximumTotal",
	/* 9*/  "Berolina",
	/*10*/  "Type1",
	/*11*/  "Type2",
	/*12*/  "Type3",
	/*13*/  "Voisin",
	/*14*/  "TypeC",
	/*15*/  "TypeD",
	/*16*/  "RangeeTranslate",
	/*17*/  "ColonneTranslate",
	/*18*/  "RangeeColonneTranselate",
	/*19*/  "Orthogonale",
	/*20*/  "Irregulier",
  /*21*/  "Transmutant"
	},{
	/* German */
	/* 0*/  "TypB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "MaximumSchwarz",
	/* 6*/  "MaximumWeiss",
        /* 7*/  "ParaSentinelles",
        /* 8*/  "MaximumTotal",
	/* 9*/  "Berolina",
	/*10*/  "Typ1",
	/*11*/  "Typ2",
	/*12*/  "Typ3",
	/*13*/  "Nachbar",
	/*14*/  "TypC",
	/*15*/  "TypD",
	/*16*/  "ReiheVerschieben",
	/*17*/  "LinieVerschieben",
	/*18*/  "ReiheLinieVerschieben",
	/*19*/  "Orthogonal",
	/*20*/  "Unregelmassig",
  /*21*/  "Transmutierende"
	},{
	/* English */
	/* 0*/  "TypeB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "MaximumBlack",
	/* 6*/  "MaximumWhite",
        /* 7*/  "ParaSentinelles",
        /* 8*/  "MaximumTotal",
	/* 9*/  "Berolina",
	/*10*/  "Type1",
	/*11*/  "Type2",
	/*12*/  "Type3",
	/*13*/  "Neighbour",
	/*14*/  "TypeC",
	/*15*/  "TypeD",
	/*16*/  "RankShift",
	/*17*/  "FileShift",
	/*18*/  "RankFileShift",
	/*19*/  "Orthogonal",
	/*20*/  "Irregular",
  /*21*/  "Transmuting"
	}
	};
#endif

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
/* This are the vectors for hoppers which change directions after the hurdle */
/* and for the CAT (a special concatenation of knight and dabbabba-rider     */
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
*/
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
/* 137 - 140 | 0,3 */    3,   72,   -3,  -72,
/* 141 - 144 | 0,4 */    4,   96,   -4,  -96,
/* 145 - 148 | 0,6 */    6,  144,   -6, -144,
/* 149 - 152 | 0,7 */    7,  168,   -7, -168,
/* 153 - 156 | 3,3 */   69,   75,  -69,  -75,
/* 157 - 160 | 4,4 */   92,  100,  -92, -100,
/* 161 - 164 | 6,6 */  138,  150, -138, -150,
/* 165 - 168 | 7,7 */  161,  175, -161, -175,
/* 169 - 176 | 2,6 */   42,  142,  146,   54,  -42, -142, -146,  -54,
/* 177 - 184 | 4,5 */   91,  116,  124,  101,  -91, -116, -124, -101,
/* 185 - 192 | 4,6 */   90,  140,  148,  102,  -90, -140, -148, -102,
/* 193 - 200 | 4,7 */   89,  164,  172,  103,  -89, -164, -172, -103,
/* 201 - 208 | 5,6 */  114,  139,  149,  126, -114, -139, -149, -126,
/* 209 - 216 | 5,7 */  113,  163,  173,  127, -113, -163, -173, -127,
/* 217 - 224 | 6,7 */  137,  162,  174,  151, -137, -162, -174, -151,
/* 225 - 232 | 2,7 */   41,  166,  170,   55,  -41, -166, -170,  -55,
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
  vec_leap36_start=129,           vec_leap36_end=  136,
  vec_leap03_start=137,           vec_leap03_end=  140,
  vec_leap04_start=141,           vec_leap04_end=  144,
  vec_leap06_start=145,           vec_leap06_end=  148,
  vec_leap07_start=149,           vec_leap07_end=  152,
  vec_leap33_start=153,           vec_leap33_end=  156,
  vec_leap44_start=157,           vec_leap44_end=  160,
  vec_leap66_start=161,           vec_leap66_end=  164,
  vec_leap77_start=165,           vec_leap77_end=  168,
  vec_leap26_start=169,           vec_leap26_end=  176,
  vec_leap45_start=177,           vec_leap45_end=  184,
  vec_leap46_start=185,           vec_leap46_end=  192,
  vec_leap47_start=193,           vec_leap47_end=  200,
  vec_leap56_start=201,           vec_leap56_end=  208,
  vec_leap57_start=209,           vec_leap57_end=  216,
  vec_leap67_start=217,           vec_leap67_end=  224,
  vec_leap27_start=225,           vec_leap27_end=  232
};


#ifdef WE_ARE_EXTERN
	extern  square boardnum[65];
#else
/* This are the really used boardnumbers */
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
/* This are the codes for the length-difference */
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

#ifdef WE_ARE_EXTERN
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
/* This are the used checkingfunctions  */
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
/* 82 */        okapicheck,
/* 83 */        leap37check,
/* 84 */        grasshop2check,
/* 85 */        grasshop3check,
/* 86 */        leap16check,
/* 87 */        leap24check,
/* 88 */        leap35check,
/* 89 */        doublegrascheck,
/* 90 */        kinghopcheck,
/* 91 */        orixcheck,
/* 92 */        leap15check,
/* 93 */        leap25check,
/* 94 */        gralcheck,
/* 95 */        tritoncheck,
/* 96 */        nereidecheck,
/* 97 */        nightlocustcheck,
/* 98 */        tourcheck,              /* wazirrider checks like rook */
/* 99 */        foucheck,               /* fersrider checks like bishop */
/*100 */        bisoncheck,
/*101 */        elephantcheck,
/*102 */        naocheck,
/*103 */        rookmoosecheck,
/*104 */        rookeaglecheck,
/*105 */        rooksparrcheck,
/*106 */        bishopmoosecheck,
/*107 */        bishopeaglecheck,
/*108 */        bishopsparrcheck,
/*109 */        roselioncheck,  	/* rao checks like roselion */
/*110 */        scorpioncheck,
/*111 */        margueritecheck,
/*112 */        leap36check,
/*113 */        nightriderlioncheck,
/*114 */        maoriderlioncheck,
/*115 */        moariderlioncheck,
/*116 */        scheck,
/*117 */        friendcheck,
/*118 */        dolphincheck,
/*119 */        rabbitcheck,
/*120 */        bobcheck,
/*121 */	equiengcheck,
/*122 */	equifracheck,
/*123 */	querquisitecheck,
/*124 */	bouncercheck,
/*125 */	rookbouncercheck,
/*126 */	bishopbouncercheck,
/*127 */	pchincheck,
/*128 */  radialknightcheck,
/*129 */  reversepcheck,
/*130 */  huntercheck,
/*131 */  huntercheck,
/*132 */  huntercheck,
/*133 */  huntercheck,
/*134 */  huntercheck,
/*135 */  huntercheck,
/*136 */  huntercheck,
/*137 */  huntercheck,
/*138 */  huntercheck,
/*139 */  huntercheck
    };
#endif

#ifdef WE_ARE_EXTERN
	extern  piece PAS[64];
#else
/* This is the InitialGameArray */
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
